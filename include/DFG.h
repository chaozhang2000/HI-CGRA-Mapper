#ifndef DFG_H
#define DFG_H
#include <list>
#include <set>
#include "DFGNodeInst.h"
#include "DFGEdge.h"
#include "DFGNode.h"
#include "DFGNodeConst.h"
#include "DFGNodeParam.h"

#define DFG_ERR(info) DFG_error = true;\
																		 OUTS(info,ANSI_FG_RED);\
																		 return
using namespace llvm;
using namespace std;

class DFG {
  private:
		/**List to save the pointer of DFGEdges in DFG
		 */
    list<DFGEdge*> m_DFGEdges; 

		/**List to save the pointer of DFGInstNodes in DFG
		 */
    list<DFGNodeInst*> m_InstNodes;

		/**List to save the pointer of DFGConstNodes in DFG
		 */
    list<DFGNodeConst*> m_ConstNodes;

		/**List to save the pointer of DFGConstNodes in DFG
		 */
    list<DFGNodeParam*> m_ParamNodes;

    void construct(Function&,int t_loopargnum);
    void showOpcodeDistribution();
    void connectDFGNodes();
    //DFGEdge* getDFGEdge(DFGNode* t_src, DFGNode* t_dst);
    //bool hasDFGEdge(DFGNode* t_src, DFGNode* t_dst);
		void reorderInstNodes();
		void DFS_findlongest(list<DFGNodeInst*>* t_longestPath);
		void reorderDFS(set<DFGNodeInst*>* t_visited, list<DFGNodeInst*>* t_targetPath,
								list<DFGNodeInst*>* t_curPath, DFGNodeInst* targetInstNode);
		int setLevelLongestPath(list<DFGNodeInst*>*longestPath,set<DFGNodeInst*>* havenotSetLevelNodes);
		void setLevelforOtherNodes(set<DFGNodeInst*>* havenotSetLevelNodes);
		void changeLongestPathColor(list<DFGNodeInst*>* t_longestPath,string t_color);
    bool shouldIgnore(Instruction*);
    string changeIns2Str(Instruction* ins);
		DFGNodeInst* getInstNode(Instruction* t_inst);
		DFGNodeConst* getConstNode(ConstantData* t_const);
		DFGNodeParam* getParamNode(Argument* t_param);
		DFGEdge* getEdgefrom(DFGNodeInst* t_src,DFGNodeInst* t_dst);

  public:
		/**The value to record if error when construct DFG
		 */
		bool DFG_error;


		/**The constructor function of class DFG
		 * @param t_F the function processed by functionpass
		 * @param t_loopparamnum the num of arg i,j,k...
		 */
		DFG(Function& t_F,int t_loopargnum);

		/**The destructor function of class DFG
		 */
		~DFG();

		/** Generate the Dot file according the DFG
		 * we can latter use dot tool to generate the png file of DFG.
 		 * @param t_F : the function pointer which the mapperPass is processing
		 */
		void generateDot(Function &t_F);

		/**get the InstNode count in DFG
		 * return the m_InstNodes.size()
		 */
		int getInstNodeCount();

		/**get the InstNodes
		 * return the &m_InstNodes
		 */
    list<DFGNodeInst*>* getInstNodes();

		/**get the InstNodes
		 * return the &m_InstNodes
		 */
		void setConstraints(map<int,int>* constraintmap,int maxCGRANodeID);
};
#endif
