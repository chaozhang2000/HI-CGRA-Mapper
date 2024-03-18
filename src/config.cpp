#include "config.h"
#include "json.h"
#include "common.h"
#include <fstream>
#include <iostream>
using namespace std;
using json = nlohmann::json;

CONFIG_INFO config_info;
bool CONFIG_INFO::getconfig()
{
			bool hasJson= false;
      ifstream param("./param.json");
			if(param.good())hasJson= true;

      if (!hasJson) {
        OUTS("=============================================================\n",ANSI_FG_RED);
				OUTS("Please provide a valid <param.json> in the current directory.",ANSI_FG_RED);
				OUTS("A set of default parameters is leveraged.",ANSI_FG_RED);
        OUTS("=============================================================\n",ANSI_FG_RED);
				return false;
      }else{
        json configs;
				param >> configs;
	kernel= configs["kernel"];
	loopargnum = configs["loopargnum"];
	rows = configs["rows"];
	cols = configs["cols"];
	instmemsize = configs["instmemsize"];
	constmemsize = configs["constmemsize"];
	shiftconstmemsize = configs["shiftconstmemsize"];
	datamemsize = configs["datamemsize"];
	mrrgsize = configs["mrrgsize"];
	loop0start = configs["loop0start"];
	loop0inc = configs["loop0inc"];
	loop0end = configs["loop0end"];
	loop1start = configs["loop1start"];
	loop1inc = configs["loop1inc"];
	loop1end = configs["loop1end"];
	loop2start = configs["loop2start"];
	loop2inc = configs["loop2inc"];
	loop2end = configs["loop2end"];
	maxsimcycle = configs["maxsimcycle"];
	for(auto& opt : configs["optlatency"].items()){
		execLatency[opt.key()] = opt.value();
	}
	for(auto& opt : configs["optpipeline"].items()){
		pipeline[opt.key()] = opt.value();
	}
				return true;
			}
}
