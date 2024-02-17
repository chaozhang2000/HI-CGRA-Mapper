#include "common.h"
#include "BitStream.h"
#include <iostream>
#include <fstream>
#include "llvm/IR/Constants.h"

using namespace std;

#define ALLOPTS(f)\
				f(mul) f(add) f(getelementptr) f(load) f(store)
#define FUKEYMAPINIT(k) m_Fukeymap->insert(make_pair(#k,++i));
BitStream::BitStream(MRRG* t_mrrg,CGRA* t_cgra,int t_II){
	m_cgra = t_cgra;
	m_mrrg = t_mrrg;
	m_II = t_II;
	m_bitStreamInfo = new BitStreamInfo;
	m_Fukeymap = new map<string,int>;
	int i = 0;
	ALLOPTS(FUKEYMAPINIT);
}
void BitStream::generateBitStream(){
	memset(m_bitStreamInfo,0,sizeof(BitStreamInfo));
	IFDEF(CONFIG_BITSTREAM_DEBUG,OUTS("\nBITSTREAM DEBUG",ANSI_FG_BLUE)); 
	IFDEF(CONFIG_BITSTREAM_DEBUG,OUTS("==================================",ANSI_FG_CYAN));
	IFDEF(CONFIG_BITSTREAM_DEBUG,OUTS("start bitstream generate",ANSI_FG_CYAN)); 
	int rows = m_cgra->getrows();
	int cols = m_cgra->getcolumns();
  for (int i=0; i<rows; i++) {
    for (int j=0; j<cols; j++) {
			generateInstofNode(m_cgra->nodes[i][j],&(m_bitStreamInfo->BitstreaminfoOfPE[i*rows+j]));
			generateConst(m_cgra->nodes[i][j],&(m_bitStreamInfo->BitstreaminfoOfPE[i*rows+j]));
			generateShiftConst(m_cgra->nodes[i][j],&(m_bitStreamInfo->BitstreaminfoOfPE[i*rows+j]));
		}
	}
	string filename = "./bitstream.bin";
	ofstream file(filename,ios::binary|ios::trunc);
	if(!file){
		OUTS("Can't open or create bitstream.bin",ANSI_FG_RED); 
		return;
	}
	file.write((char*)m_bitStreamInfo,sizeof(BitStreamInfo));
	file.close();
	IFDEF(CONFIG_BITSTREAM_DEBUG,outs()<<"Map II="<<m_II<<"\n"); 
	IFDEF(CONFIG_BITSTREAM_DEBUG,outs()<<"successfully finish bitstream generation\n"); 
}

void BitStream::generateInstofNode(CGRANode* node,BitStreamInfoPE* bitstream){
	if(m_II > CONFIG_CGRA_INSTMEM_SIZE)llvm_unreachable("Inst Mem size < m_II, need bigger InstMem");
	bitstream->ctrlregs.Instnum = m_II;
	int first_cycle = m_mrrg->getFirstcycleofNode(node);
	int last_cycle = m_mrrg->getLastcycleofNode(node);
	NodeInfo* nodeinfo = m_mrrg->getNodeInfoOf(node);

	/*Fuinst*/
	for(int c = first_cycle; c<= last_cycle;c++){
		int inst_cnt = (c-first_cycle)% m_II;
		if(bitstream->insts[inst_cnt].FuInst.Fukey != 0 || nodeinfo->m_OccupiedByNode[c]==NULL)continue;
		string opname = nodeinfo->m_OccupiedByNode[c]->getOpcodeName();
		bitstream->insts[inst_cnt].FuInst.Fukey = (*m_Fukeymap)[opname];
		bitstream->insts[inst_cnt].FuInst.Src1key = nodeinfo->m_Src1OccupyState[c];
		bitstream->insts[inst_cnt].FuInst.Src2key = nodeinfo->m_Src2OccupyState[c];
		bitstream->insts[inst_cnt].FuInst.FudelayII = (c-first_cycle)/m_II;
	}


	/*Linkinst*/
	for(CGRANode* neighbor: *(node->getNeighbors())){
		CGRALink* link = node->getOutLinkto(neighbor);
		LinkInfo* linkinfo = m_mrrg->getLinkInfoOf(link);
		int direction = link->getdirection();
		last_cycle = m_mrrg->getLastcycleofLink(link);
		for(int c = first_cycle; c<= last_cycle;c++){
			int inst_cnt = (c-first_cycle)% m_II;
			if(bitstream->insts[inst_cnt].LinkInsts[direction].Dkey != LINK_NOT_OCCUPY || (linkinfo->m_occupied_state[c]==LINK_NOT_OCCUPY || linkinfo->m_occupied_state[c]==LINK_OCCUPY_EMPTY)) continue;
			bitstream->insts[inst_cnt].LinkInsts[direction].Dkey = linkinfo->m_occupied_state[c];
			bitstream->insts[inst_cnt].LinkInsts[direction].DelayII = (c-first_cycle)/m_II;
			/*
			outs()<<"d:"<<direction<<"\n";
			outs()<<"c:"<<c<<"\n";
			outs()<<"dkey:"<<bitstream->insts[inst_cnt].LinkInsts[direction].Dkey<<"\n";
			outs()<<"delayII:"<<bitstream->insts[inst_cnt].LinkInsts[direction].DelayII<<"\n";
			*/
		}
	}
}

void BitStream::generateConst(CGRANode* node,BitStreamInfoPE* bitstream){
	int first_cycle = m_mrrg->getFirstcycleofNode(node);
	int last_cycle = m_mrrg->getLastcycleofNode(node);
	NodeInfo* nodeinfo = m_mrrg->getNodeInfoOf(node);
	int cnt1 = 0;
	int cnt2 = 0;
	for(int i = 0;i<m_II;i++){
		for(int c = first_cycle+i; c<=last_cycle;c=c+m_II){
			bool src1isconst = nodeinfo->m_Src1OccupyState[c]==SRC_OCCUPY_FROM_CONST_MEM;
			bool src2isconst = nodeinfo->m_Src2OccupyState[c]==SRC_OCCUPY_FROM_CONST_MEM;
			if(!src1isconst&&!src2isconst)continue;
			if(src1isconst){
				DFGNodeConst* constnode1 = nodeinfo->m_OccupiedByNode[c]->getPredConstNode(0);
				DFGNodeParam* paramnode1 = nodeinfo->m_OccupiedByNode[c]->getPredParamNode(0);
				if(constnode1 == NULL && paramnode1 == NULL)llvm_unreachable("constnode1 should not be NULL, mapper have bug");
				if(cnt1 > CONFIG_CGRA_CONSTMEM_SIZE-1)llvm_unreachable("Need bigger ConstMem");
				bitstream->const1[cnt1] = constnode1 != NULL ? getconstvalue(constnode1):0;
				/*
				outs()<<"src1isconst"<<c<<"\n";
				outs()<<"cycle:"<<c<<"\n";
				outs()<<"cnt1:"<<cnt1<<"\n";
				outs()<<"NodeID"<<nodeinfo->m_OccupiedByNode[c]->getID()<<"\n";
				outs()<<"value"<<bitstream->const1[cnt1]<<"\n";
				*/
				cnt1 ++;
			}
			if(src2isconst){
				DFGNodeConst* constnode2 = nodeinfo->m_OccupiedByNode[c]->getPredConstNode(1);
				DFGNodeParam* paramnode2 = nodeinfo->m_OccupiedByNode[c]->getPredParamNode(1);
				if(constnode2 == NULL && paramnode2 == NULL)llvm_unreachable("constnode2 should not be NULL, mapper have bug");
				if(cnt2 > CONFIG_CGRA_CONSTMEM_SIZE-1)llvm_unreachable("Need bigger ConstMem");
				bitstream->const2[cnt2] = constnode2 !=NULL ? getconstvalue(constnode2):0;
				/*
				outs()<<"src2isconst"<<c<<"\n";
				outs()<<"cycle:"<<c<<"\n";
				outs()<<"cnt2:"<<cnt2<<"\n";
				outs()<<"NodeID"<<nodeinfo->m_OccupiedByNode[c]->getID()<<"\n";
				outs()<<"value"<<bitstream->const2[cnt2]<<"\n";
				*/
				cnt2 ++;
			}
			if(src1isconst || src2isconst) break;
		}
	}
	bitstream->ctrlregs.Constnum1=cnt1;
	bitstream->ctrlregs.Constnum2=cnt2;
}

int BitStream::getconstvalue(DFGNodeConst* constnode){
	ConstantInt * const_int = dyn_cast<ConstantInt>(constnode->getConstant());
	if(const_int == NULL)llvm_unreachable("const is not int, not support now,and should not reach here, this should be handle when generate the DFG");
	return const_int->getSExtValue();
}

void BitStream::generateShiftConst(CGRANode* node,BitStreamInfoPE* bitstream){
	int first_cycle = m_mrrg->getFirstcycleofNode(node);
	int last_cycle = m_mrrg->getLastcycleofNode(node);
	NodeInfo* nodeinfo = m_mrrg->getNodeInfoOf(node);
	int cnt1 = 0;
	int cnt2 = 0;
	for(int i = 0;i<m_II;i++){
		for(int c = first_cycle+i; c<=last_cycle;c=c+m_II){
			bool src1isloopnum = (nodeinfo->m_Src1OccupyState[c]==SRC_OCCUPY_FROM_LOOP0)||(nodeinfo->m_Src1OccupyState[c]==SRC_OCCUPY_FROM_LOOP1)||(nodeinfo->m_Src1OccupyState[c]==SRC_OCCUPY_FROM_LOOP2);
			bool src2isloopnum = (nodeinfo->m_Src2OccupyState[c]==SRC_OCCUPY_FROM_LOOP0)||(nodeinfo->m_Src2OccupyState[c]==SRC_OCCUPY_FROM_LOOP1)||(nodeinfo->m_Src2OccupyState[c]==SRC_OCCUPY_FROM_LOOP2);
			if(!src1isloopnum&&!src2isloopnum)continue;
			if(src1isloopnum){
				DFGNodeParam* paramnode1 = nodeinfo->m_OccupiedByNode[c]->getPredParamNode(0);
				if(paramnode1 == NULL)llvm_unreachable("paramnode1 should not be NULL, mapper have bug");
				int delayII = (c-first_cycle)/m_II;
				int shiftconst1 = calculateShiftconst(paramnode1,delayII);
				if(shiftconst1 != 0){
					if(cnt1 > CONFIG_CGRA_SHIFTCONSTMEM_SIZE-1)llvm_unreachable("Need bigger ShiftconstMem");
					bitstream->shiftconst1[cnt1] = shiftconst1;
					bitstream->insts[i].FuInst.Shiftconst1 = true;
					/*
				outs()<<"src1isloopnum"<<"\n";
				outs()<<"cycle:"<<c<<"\n";
				outs()<<"delayII:"<<delayII<<"\n";
				outs()<<"cnt1:"<<cnt1<<"\n";
				outs()<<"NodeID"<<nodeinfo->m_OccupiedByNode[c]->getID()<<"\n";
				outs()<<"value"<<bitstream->shiftconst1[cnt1]<<"\n";
				*/
					cnt1 ++;
				}
			}
			if(src2isloopnum){
				DFGNodeParam* paramnode2 = nodeinfo->m_OccupiedByNode[c]->getPredParamNode(1);
				if(paramnode2 == NULL)llvm_unreachable("parmanode2 should not be NULL, mapper have bug");
				int delayII = (c-first_cycle)/m_II;
				int shiftconst2 = calculateShiftconst(paramnode2,delayII);
				if(shiftconst2 != 0){
					if(cnt2 > CONFIG_CGRA_SHIFTCONSTMEM_SIZE-1)llvm_unreachable("Need bigger ShiftconstMem");
					bitstream->shiftconst2[cnt2] = shiftconst2;
					bitstream->insts[i].FuInst.Shiftconst2 = true;
					/*
				outs()<<"src2isloopnum"<<"\n";
				outs()<<"cycle:"<<c<<"\n";
				outs()<<"delayII:"<<delayII<<"\n";
				outs()<<"cnt2:"<<cnt2<<"\n";
				outs()<<"NodeID"<<nodeinfo->m_OccupiedByNode[c]->getID()<<"\n";
				outs()<<"value"<<bitstream->shiftconst2[cnt2]<<"\n";
				*/
					cnt2 ++;
				}
			}
			if(src1isloopnum || src2isloopnum)break;
		}
	}
	bitstream->ctrlregs.Shiftconstnum1=cnt1;
	bitstream->ctrlregs.Shiftconstnum2=cnt2;
	bitstream->ctrlregs.I_init=CONFIG_LOOP_I_START;
	bitstream->ctrlregs.J_init=CONFIG_LOOP_J_START;
	bitstream->ctrlregs.K_init=CONFIG_LOOP_K_START;
	bitstream->ctrlregs.I_inc=CONFIG_LOOP_I_INC;
	bitstream->ctrlregs.J_inc=CONFIG_LOOP_J_INC;
	bitstream->ctrlregs.K_inc=CONFIG_LOOP_K_INC;
	bitstream->ctrlregs.I_thread=CONFIG_LOOP_I_END;
	bitstream->ctrlregs.J_thread=CONFIG_LOOP_J_END;
	bitstream->ctrlregs.K_thread=CONFIG_LOOP_K_END;
}
/*TODO: not finish yet!! return the value of shiftconst, in mm example shiftconst is always 0*/
int BitStream::calculateShiftconst(DFGNodeParam* paramnode,int delayII){
	return 0;
}

BitStream::~BitStream(){
	delete m_bitStreamInfo;
}
