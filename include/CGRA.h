#ifndef CGRA_H
#define CGRA_H
#include "CGRANode.h"
#include "CGRALink.h"

class CGRA {
  private:
		/** the var to save the num of CGRAlinks in the CGRA
		 */
    int m_FUCount;

		/** the var to save the num of CGRAlinks in the CGRA
		 */
    int m_LinkCount;

		/** the var to save the rows of CGRAlinks in the CGRA
		 */
    int m_rows;

		/** the var to save the columns of CGRAlinks in the CGRA
		 */
    int m_columns;

  public:
		/**save the CGRANodes in CGRA
		 * pointer of nodes[m_rows][m_columns]
		 */
    CGRANode ***nodes;

		/**save the CGRALinks in CGRA
		 * pointer of links[m_LinkCount]
		 */
    CGRALink **links;

		/**The constructor function of class CGRA
		 */
		CGRA(int t_rows,int t_columns);
		~CGRA();

		/**return m_FUCount
		 */
		int getNodeCount();

		/**return m_LinkCount
		 */
		int getLinkCount();

		/**return m_rows
		 */
		int getrows();

		/**return m_columns
		 */
		int getcolumns();
		
		/**get the CGRALink form CGRANode t_src to CGRANode t_dst
		 * if not found return NULL,it found return the pointer to the link
		 */
		CGRALink* getLinkfrom(CGRANode* t_src,CGRANode* t_dst);

		
};
#endif
