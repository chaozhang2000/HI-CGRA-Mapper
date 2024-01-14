#ifndef DFGNode_H
#define DFGNode_H

#include <string>
#include <list>
#include <llvm/Support/FileSystem.h>
using namespace std;


class DFGEdge;
class DFGNode {
  private:
    int m_id;
		string m_name;
		/**the list to save the pointers of input DFGEdges
		 */
    list<DFGEdge*> m_inEdges;

		/**the list to save the pointers of output DFGEdges
		 */
    list<DFGEdge*> m_outEdges;

  public:
		DFGNode(int t_id,string t_name);
		virtual ~DFGNode(){};

    int getID();
		string getName();

		/** add t_dfgEdge to m_inEdges of the DFGNode
		 *  @param t_dfgEdge : the pointer of DFGEdge to be added to the m_inEdges
		 */
    void setInEdge(DFGEdge* t_dfgEdge);
		/** add t_dfgEdge to m_outEdges of the DFGNode
		 *  @param t_dfgEdge : the pointer of DFGEdge to be added to the m_outEdges
		 */
    void setOutEdge(DFGEdge* t_dfgEdge);

		/** get &m_inEdges;
		 */
		list<DFGEdge*>* getInEdges();

		/** get &m_outEdges;
		 */
		list<DFGEdge*>* getOutEdges();
};
#endif
