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
	generateInstofNode(m_cgra->nodes[0][0],m_bitStreamInfo);
	generateConst(m_cgra->nodes[0][0],m_bitStreamInfo);


	IFDEF(CONFIG_BITSTREAM_DEBUG,OUTS("\nBITSTREAM DEBUG",ANSI_FG_BLUE)); 
	IFDEF(CONFIG_BITSTREAM_DEBUG,OUTS("==================================",ANSI_FG_CYAN));
	IFDEF(CONFIG_BITSTREAM_DEBUG,OUTS("start bitstream generate",ANSI_FG_CYAN)); 
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

void BitStream::generateInstofNode(CGRANode* node,BitStreamInfo* bitstream){
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

void BitStream::generateConst(CGRANode* node,BitStreamInfo* bitstream){
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
				if(constnode1 == NULL)llvm_unreachable("constnode1 should not be NULL, mapper have bug");
				bitstream->const1[cnt1] = getconstvalue(constnode1);
				/*
				outs()<<"src1isconst"<<c<<"\n";
				outs()<<"cycle:"<<c<<"\n";
				outs()<<"cnt1:"<<cnt1<<"\n";
				outs()<<"NodeID"<<nodeinfo->m_OccupiedByNode[c]->getID()<<"\n";
				outs()<<"value"<<bitstream->const1[cnt1]<<"\n";
				*/
				cnt1 ++;
				break;
			}
			if(src2isconst){
				DFGNodeConst* constnode2 = nodeinfo->m_OccupiedByNode[c]->getPredConstNode(1);
				if(constnode2 == NULL)llvm_unreachable("constnode2 should not be NULL, mapper have bug");
				bitstream->const2[cnt2] = getconstvalue(constnode2);
				/*
				outs()<<"src2isconst"<<c<<"\n";
				outs()<<"cycle:"<<c<<"\n";
				outs()<<"cnt2:"<<cnt2<<"\n";
				outs()<<"NodeID"<<nodeinfo->m_OccupiedByNode[c]->getID()<<"\n";
				outs()<<"value"<<bitstream->const2[cnt2]<<"\n";
				*/
				cnt2 ++;
				break;
			}
		}
	}
}
int BitStream::getconstvalue(DFGNodeConst* constnode){
	ConstantInt * const_int = dyn_cast<ConstantInt>(constnode->getConstant());
	if(const_int == NULL)llvm_unreachable("const is not int, not support now,and should not reach here, this should be handle when generate the DFG");
	return const_int->getSExtValue();
}

BitStream::~BitStream(){
	delete m_bitStreamInfo;
}
