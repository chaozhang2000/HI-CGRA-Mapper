#ifndef config_H
#define config_H

#include <string>
#include <map>
using namespace std;
class CONFIG_INFO {
	public:
	string kernel;
	int loopargnum;
	int rows;
	int cols;
	int instmemsize;
	int constmemsize;
	int shiftconstmemsize;
	int datamemsize;
	int mrrgsize;
	int loop0start;
	int loop0inc;
	int loop0end;
	int loop1start;
	int loop1inc;
	int loop1end;
	int loop2start;
	int loop2inc;
	int loop2end;
	int maxsimcycle;
	map<string,int> execLatency;	
	map<string,int> pipeline;
	bool getconfig();
};
extern CONFIG_INFO config_info;
#endif
