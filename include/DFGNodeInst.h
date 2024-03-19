#ifndef DFGNodeInst_H
#define DFGNodeInst_H
#include <llvm/IR/Instruction.h>
#include "DFGNode.h"
#include "DFGNodeConst.h"
#include "DFGNodeParam.h"

using namespace llvm;
using namespace std;

class DFGNodeInst:public DFGNode{
	private:
		/**var to save the inst 
		 */
		Instruction* m_inst;
		/**var to save the opcode name of inst
		 */
		string m_opcodeName;	
		/**list to save the succInstNode
		 */
		list<DFGNodeInst*>* m_succInstNodes;
		/**list to save the succInstNode
		 */
		list<DFGNodeInst*>* m_predInstNodes;
		/**var to save the level of InstNode in DFG,the level will be given when reorder the DFG
		 * the dfgnode with lower level execute first
		 */
		int m_level;
		/**var to record if the DFGNodeInst have been set level
		 */
		bool m_haveSetLevel;
		/**var to save if the InstNode is a memopts like load and store.
		 */
		bool m_isMemOpts;
		/**var to save if the InstNode is constrainted to a CGRANode by the mapconstraint.json.
		 */
		bool m_constrainted;
		/**the CGRANode ID which this DFGNodeInst is constrained to be mapped to.
		 */
		int m_constraintTo;

		/**the opt delay.
		 */
		int m_latency;

		/**if opt support pipeline
		 */
		bool m_pipeline;

	public:
		static const string color;
		/**The constructor function of class DFGNodeInst
		 * @param t_id :the id that give the DFGNode
		 * @param t_inst :The instruction corresponding to this DFGNodeInst
		 */
		DFGNodeInst(int t_id, Instruction* t_inst,string t_name);
		~DFGNodeInst();

		/**return m_inst
		 */
    Instruction* getInst();

		/**return m_opcodeName
		 */
    string getOpcodeName();

		/**function to get Succ InstNode of this InstNode
		 * first call will new list<DFGNodeInst*> for m_succInstNodes
		 * and find succ InstNodes and add them to m_succInstNodes
		 * return m_succInstNodes
		 */
		list<DFGNodeInst*>* getSuccInstNodes();
		list<DFGNodeInst*>* getPredInstNodes();

		/**set m_level the value of t_level
		 * set m_haveSetLevel true;
		 */
		void setLevel(int t_level);

		/**return m_level
		 */
		int getLevel();
		/**return m_haveSetLevel;
		 */
		bool haveSetLevel();
		/**return m_isMemOpts;
		 */
		bool isMemOpts();
		/**return m_constraintTo;
		 */
		int constraintTo();
		/**return m_constrainted;
		 */
		bool hasConstraint();
		/** set constraint,set m_constraintTo the value of CGRANodeID,and set m_constrainted true
		 */
		void setConstraint(int CGRANodeID);

		/**return a pre ConstNode of this InstNode; if not found return NULL;
		 * @param: srcID, the srcID of the ConstNode.
		 */
		DFGNodeConst* getPredConstNode(int srcID);
		DFGNodeParam* getPredParamNode(int srcID);

		int getlatency();
		bool ispipeline();
};
#endif
