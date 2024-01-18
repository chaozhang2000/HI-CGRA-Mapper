#ifndef CGRALink_H
#define CGRALink_H

#include "CGRANode.h"

enum {	LINK_DIRECTION_TO_N, 
				LINK_DIRECTION_TO_S,
				LINK_DIRECTION_TO_W,
				LINK_DIRECTION_TO_E};
class CGRALink
{
  private:
    int m_id;

		/**record the pointer of src CGRANode
		 */
    CGRANode *m_src;

		/**record the pointer of dst CGRANode
		 */
    CGRANode *m_dst;

		/**record the direction of the CGRALink
		 */
		int m_direction;
		
  public:
		/**The constructor function of class CGRALink
		 * this function init CGRANode's ID according the params,other var is init by default value.
		 * @param t_linkId : the id of the CGRALink
		 * @param t_direction: the direction of the CGRALink
		 */
		CGRALink(int t_linkId,int t_direction);

		/**Connect the CGRALink to src and dst CGRANodes,by set m_src=t_src and m_dst=t_dst
		 * @param t_src : the pointer to the src CGRAnode
		 * @param t_dst : the pointer to the dst CGRAnode
		 */
		void connect(CGRANode* t_src, CGRANode* t_dst);

		/**return m_id
		 */
		int getID();
		/**return m_src
		 */
		CGRANode* getsrc();
		/**return m_dst
		 */
		CGRANode* getdst();
		/**return m_direction
		 */
		int getdirection();
};
#endif
