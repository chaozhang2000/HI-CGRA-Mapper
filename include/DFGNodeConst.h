#ifndef DFGNodeConst_H
#define DFGNodeConst_H
#include <llvm/IR/Constant.h>
#include "DFGNode.h"

using namespace llvm;
using namespace std;

class DFGNodeConst:public DFGNode{
	private:
		/**to save the ConstantData of this DFGNodeConst
		 */
		ConstantData* m_const;
	public:
		static const string color;
		/**Constructor function of DFGConstNode. t_id and t_name is for
		 * parent class DFGNode's Constructor function.
		 */
		DFGNodeConst(int t_id, ConstantData* t_const,string t_name);
		~DFGNodeConst(){};

		/**return m_const
		 */
    ConstantData* getConstant();
};
#endif
