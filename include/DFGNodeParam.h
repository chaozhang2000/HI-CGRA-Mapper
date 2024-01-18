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

		/**to record if the Argument is i,j,k...
		 */
		bool m_isloop;

		/**to record the LoopID, the ID of the innermost loop is the smallest,start from 0
		 */
		int m_loopID;
	public:
		static const string color;
		/**Constructor function of DFGNodeParam. t_id and t_name is for
		 * parent class DFGNode's Constructor function.
		 */
		DFGNodeParam(int t_id, Argument* t_param,string t_name,bool t_isloop,int t_loopID);
		~DFGNodeParam(){};

		/**return m_param
		 */
    Argument* getParam();

		/**return m_isloop
		 */
    bool isloop();

		/**return m_loopID
		 */
    int getloopID();
};
#endif
