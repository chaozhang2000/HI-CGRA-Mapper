#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopInfo.h>
#include <fstream>
#include <iostream>
#include "json.h"
#include "DFG.h"
#include "common.h"
#include "CGRA.h"
#include "MRRG.h"
#include "Mapper.h"

using namespace llvm;
using namespace std;
using json = nlohmann::json;

void addDefaultKernels(map<string, list<int>*>*);

namespace {

  struct mapperPass : public FunctionPass {

  public:
    static char ID;
    mapperPass() : FunctionPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addPreserved<LoopInfoWrapperPass>();
      AU.setPreservesAll();
    }

		/**
		 * Mapper enter at this function
		 */
    bool runOnFunction(Function &t_F) override {

      // Read the target function from JSON file.
			string target_function;
			int loopargnum = 0;
      ifstream i("./param.json");
      if (!i.good()) {
        outs()<< "=============================================================\n";
				OUTS("Please provide a valid <param.json> in the current directory.",ANSI_FG_RED);
				OUTS("A set of default parameters is leveraged.",ANSI_FG_RED);
        outs()<<"=============================================================\n";
      } else {
        json param;
        i >> param; 
				target_function = param["kernel"];
				loopargnum = param["loopargnum"];
      }

      // Check existance. if the function is our target function
      if (target_function!=t_F.getName().str()) {
        return false;
      }

			bool hasmapconstrantJson= false;
      ifstream mapconstraint("./mapconstraint.json");
			if(mapconstraint.good())hasmapconstrantJson= true;

      DFG* dfg = new DFG(t_F,loopargnum);
			if (dfg->DFG_error){
				return false;
			}

      // Generate the DFG dot file.
      dfg->generateDot(t_F);

			/*the mapping need a mapconstraint.json to map all load store DFGNode ID to CGRANode ID, to tell the mapper,which mem the data save, now we assume every PE can access a data mem,if the mapconstraint.json not exsisted break,else read map info from the file*/
			map<int,int>* constraintmap = new map<int,int>;//TODO: free this space
      if (!hasmapconstrantJson) {
        outs()<< "=============================================================\n";
				OUTS("Please provide a <mapconstraint.json> in the current directory.",ANSI_FG_RED);
        outs()<<"=============================================================\n";
				return false;
      }else{
        json constraint;
				mapconstraint >> constraint;
				json maps = constraint["DFGNodeIDCGRANodeID"];
				json map;
				for(unsigned long i =0;i<maps.size();i++){
					map = maps[i];
					(*constraintmap)[map[0]] = map[1];
				}
			}

			CGRA* cgra = new CGRA(4,4);
#ifdef CONFIG_MAP_EN
			MRRG* mrrg = new MRRG(cgra,200);

			Mapper* mapper = new Mapper(dfg,cgra,mrrg);

			mapper->heuristicMap();

			delete mapper;
			delete mrrg;
#endif
			delete cgra;
			delete dfg;
			return true;
    }

  };
}

char mapperPass::ID = 0;
static RegisterPass<mapperPass> X("mapperPass", "DFG Pass Analyse", false, false);
