#ifndef DFGNodeParam_H
#define DFGNodeParam_H
#include <llvm/IR/Argument.h>
#include "DFGNode.h"

using namespace llvm;
using namespace std;

class DFGNodeParam:public DFGNode{
	private:
		/**to save the Argument of this kernel function
		 */
		Argument* m_param;
	public:
		static const string color;
		/**Constructor function of DFGNodeParam. t_id and t_name is for
		 * parent class DFGNode's Constructor function.
		 */
		DFGNodeParam(int t_id, Argument* t_param,string t_name);
		~DFGNodeParam(){};

		/**return m_param
		 */
    Argument* getParam();
};
#endif
