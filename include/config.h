#ifndef config_H
#define config_H

#include <string>
class CONFIG_INFO {
	public:
	std::string kernel;
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
	bool getconfig();
};
extern CONFIG_INFO config_info;
#endif
