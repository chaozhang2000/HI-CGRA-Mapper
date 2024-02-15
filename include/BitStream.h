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

struct BitStreamInfo{
	CGRAInstruction insts[CONFIG_CGRA_INSTMEM_SIZE];
};
class BitStream{
	private:
		MRRG* m_mrrg;

		CGRA* m_cgra;

		int m_II;

		BitStreamInfo* m_bitStreamInfo;

		map<string,int> *m_Fukeymap;

		void generateInstofNode(CGRANode* node,BitStreamInfo* bitstream);
	public:
		BitStream(MRRG* t_mrrg,CGRA* t_cgra,int t_II);
		~BitStream();
		void generateBitStream();
};

#endif
