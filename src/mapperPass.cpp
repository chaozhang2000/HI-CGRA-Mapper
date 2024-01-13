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
      ifstream i("./param.json");
      if (!i.good()) {

        errs()<< "=============================================================\n";
        errs()<<"\033[0;31mPlease provide a valid <param.json> in the current directory.\n";
        errs()<<"A set of default parameters is leveraged.\033[0m\n";
        errs()<<"=============================================================\n";
      } else {
        json param;
        i >> param; 
				target_function = param["kernel"];
      }


      // Check existance. if the function is our target function
      if (target_function!=t_F.getName().str()) {
        return false;
      }

      DFG* dfg = new DFG(t_F);
			if (dfg->DFG_error){
				return false;
			}
#ifdef CONFIG_DFG_DEBUG 
      // Show the count of different opcodes (IRs).
      dfg->showOpcodeDistribution();
#endif

      // Generate the DFG dot file.
			bool isTrimmedDemo = true;
      dfg->generateDot(t_F, isTrimmedDemo);

			CGRA* cgra = new CGRA(4,4);

			MRRG* mrrg = new MRRG(cgra,200);

			Mapper* mapper = new Mapper(dfg,cgra,mrrg);

			mapper->heuristicMap();

			mrrg->MRRGclear();

			delete mapper;
			delete mrrg;
			delete dfg;
			delete cgra;
			return true;
    }

  };
}

char mapperPass::ID = 0;
static RegisterPass<mapperPass> X("mapperPass", "DFG Pass Analyse", false, false);
