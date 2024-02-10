#ifndef BitStream_H
#define BitStream_H

#include "MRRG.h"

struct BitStreamInfo{
	char test1[5];
	char test2[5];
};
class BitStream{
	private:
		MRRG* m_mrrg;

		int m_II;

		BitStreamInfo* m_bitStreamInfo;
	public:
		BitStream(MRRG* t_mrrg,int t_II);
		~BitStream();
		void generateBitStream();
};

#endif
