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
#include "BitStream.h"
#include "config.h"

using namespace llvm;
using namespace std;
using json = nlohmann::json;

bool getConstraint(map<int,int>* constraintmap);

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
			
			if(!getconfig(&config_info)){
				return false;
			}

      // Check existance. if the function is our target function
      if (config_info.kernel!=t_F.getName().str()) {
        return false;
      }


      DFG* dfg = new DFG(t_F,config_info.loopargnum);
			if (dfg->DFG_error){
				return false;
			}

			CGRA* cgra = new CGRA(config_info.rows,config_info.cols);

#ifdef CONFIG_MAP_CONSTRAINT
			map<int,int>* constraintmap = new map<int,int>;//TODO: free this space

			bool hasConstraintFile = false; 

      /*get constraint form the file*/
			hasConstraintFile = getConstraint(constraintmap);

      /*set constraint in dfg*/
			dfg->setConstraints(constraintmap,cgra->getNodeCount()-1);
			delete constraintmap;
#endif

      /*Generate the DFG dot file.*/
      dfg->generateDot(t_F);

#ifdef CONFIG_MAP_CONSTRAINT
      /*don't have ConstrantFile,don't do map.*/
			if(!hasConstraintFile) {
        outs()<< "=============================================================\n";
				OUTS("Please provide a <mapconstraint.json> in the current directory.",ANSI_FG_RED);
        outs()<<"=============================================================\n";
				return false;
			}

			/*setConstraint may cause err,if err break*/
			if (dfg->DFG_error){
				return false;
			}
#endif

#ifdef CONFIG_MAP_EN
			MRRG* mrrg = new MRRG(cgra,config_info.mrrgsize);

			Mapper* mapper = new Mapper(dfg,cgra,mrrg);

			bool mapsuccess = mapper->heuristicMap();
#ifdef CONFIG_MAP_BITSTREAM
			if (mapsuccess){
			BitStream* bitstream = new BitStream(mrrg,cgra,mapper->getII());
			bitstream->generateBitStream();
			delete bitstream;
			}
#endif
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

bool getConstraint(map<int,int>* constraintmap){
			bool hasmapconstrantJson= false;
      ifstream mapconstraint("./mapconstraint.json");
			if(mapconstraint.good())hasmapconstrantJson= true;

      if (!hasmapconstrantJson) {
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
				return true;
			}
}
