#include "common.h"
#include "BitStream.h"
#include <iostream>
#include <fstream>

using namespace std;
BitStream::BitStream(MRRG* t_mrrg,int t_II){
	m_mrrg = t_mrrg;
	m_II = t_II;
	m_bitStreamInfo = new BitStreamInfo;
	m_bitStreamInfo->test1[0]=0x68; 
	m_bitStreamInfo->test1[1]=0x68; 
	m_bitStreamInfo->test1[2]=0x68; 
	m_bitStreamInfo->test1[3]=0x68; 
	m_bitStreamInfo->test1[4]=0x68; 
	m_bitStreamInfo->test2[0]=0x69; 
	m_bitStreamInfo->test2[1]=0x69; 
	m_bitStreamInfo->test2[2]=0x69; 
	m_bitStreamInfo->test2[3]=0x69; 
	m_bitStreamInfo->test2[4]=0x69; 
}
void BitStream::generateBitStream(){
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
	IFDEF(CONFIG_BITSTREAM_DEBUG,outs()<<"successfully finish bitstream generation\n"); 
}

BitStream::~BitStream(){
	delete m_bitStreamInfo;
}
