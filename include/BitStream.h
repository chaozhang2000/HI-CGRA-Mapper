#ifndef BitStream_H
#define BitStream_H

#include "MRRG.h"

struct CGRALinkInst{
	int Dkey;
	int DelayII;
};
struct CGRAFuInst{
	int Fukey;
	int Src1key;
	int Src2key;
	int FudelayII;
	bool Shiftconst1;//
	bool Shiftconst2;//
};
struct CGRAInstruction {
	CGRAFuInst FuInst;
	CGRALinkInst LinkInsts[4];
};
struct CtrlRegs {
	int Instnum;
	int IInum;
	int Constnum1;
	int Constnum2;
	int Shiftconstnum1;
	int Shiftconstnum2;
	int I_init;
	int J_init;
	int K_init;
	int I_inc;
	int J_inc;
	int K_inc;
	int I_thread;
	int J_thread;
	int K_thread;

	int Instcnt;
	int IIcnt;
	int Constcnt1;
	int Constcnt2;
	int Shiftconstcnt1;
	int Shiftconstcnt2;
};

struct BitStreamInfoPE{
	CGRAInstruction insts[CONFIG_CGRA_INSTMEM_SIZE];
	int const1[CONFIG_CGRA_CONSTMEM_SIZE];
	int const2[CONFIG_CGRA_CONSTMEM_SIZE];
	int shiftconst1[CONFIG_CGRA_SHIFTCONSTMEM_SIZE];
	int shiftconst2[CONFIG_CGRA_SHIFTCONSTMEM_SIZE];
	CtrlRegs ctrlregs;
};
struct BitStreamCheck{
	int InstMemSize;
	int ConstMemSize;
	int ShiftConstMemSize;
};
struct BitStreamInfo{
	BitStreamCheck CheckInfo;
	BitStreamInfoPE BitstreaminfoOfPE[16];
};
class BitStream{
	private:
		MRRG* m_mrrg;

		CGRA* m_cgra;

		int m_II;

		BitStreamInfo* m_bitStreamInfo;

		map<string,int> *m_Fukeymap;

		void generateInstofNode(CGRANode* node,BitStreamInfoPE* bitstream);
		void generateConst(CGRANode* node,BitStreamInfoPE* bitstream);
		int getconstvalue(DFGNodeConst* constnode);
		void generateShiftConst(CGRANode* node,BitStreamInfoPE* bitstream);
		int calculateShiftconst(DFGNodeParam* paramnode,int delayII);
		int getIInum();
		void printBitStream();
	public:
		BitStream(MRRG* t_mrrg,CGRA* t_cgra,int t_II);
		~BitStream();
		void generateBitStream();
};

#endif
