#ifndef DFGEdge_H
#define DFGEdge_H

#include <llvm/Support/FileSystem.h>
#include "DFGNode.h"
#include <string>

class DFGEdge
{
  private:
		/**The var to record DFGEdge ID 
		 */
    int m_id;
		/**The var to record this DFGEdge mean which src,src1 or src2
		 */
    int m_srcID;
		/**The var to record src DFGNode,the edge points from src DFGNode to dst DFGNode
		 */
    DFGNode *m_src;
		/**The var to record dst DFGNode,the edge points from src DFGNode to dst DFGNode
		 */
    DFGNode *m_dst;

		string m_color;

  public:
		/**The constructor function of class DFGEdge
		 * assign value to m_id,m_src and m_dst to create a Edge from src DFGNode to dst DFGNode
		 * @param t_id the id of the DFGEdge created
		 * @param t_srcID which number of operands
		 * @param t_src pointer to the src DFGNode 
		 * @param t_dst pointer to the dst DFGNode
		 */
    DFGEdge(int t_id, int t_srcID, DFGNode* t_src, DFGNode* t_dst);

		/**return m_id
		 */
    int getID();

		/**return m_srcID
		 */
    int getsrcID();

		/**return m_src
		 */
    DFGNode* getSrc();

		/**return m_dst
		 */
    DFGNode* getDst();

		/**return m_color
		 */
		string getcolor();
		
		/**connect DFGNode t_src to DFGNode* using this DFGEdge
		 */
    void connect(DFGNode*t_src, DFGNode*t_dst);

		/**set the DFGEdge color in DFG
		 */
		void setcolor(string t_color);
};

#endif
