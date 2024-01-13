#ifndef CGRANode_H
#define CGRANode_H

#include <list>
#include <string>
#include <set>

using namespace std;
class CGRALink;
class CGRANode {

  private:
    int m_id;
    int m_x;
    int m_y;
		/**true main disable the CGRANode
		 */
    bool m_disabled;
		bool m_hasDataMem;
		/**the list to record input CGRALinks of this CGRANode
		 */
    list <CGRALink*> m_inLinks;
		/**the list to record output CGRALinks of this CGRANode
		 */
    list <CGRALink*> m_outLinks;

		/**the list to record all opname that this CGRANode is support
		 */
		set<string> m_supportOpts;

		/**the list to record all neighbor CGRANode
		 */
		list<CGRANode*>* m_neighbors;

  public:
		/**The constructor function of class CGRANode
		 * this function init CGRANode's ID,x and y according the params,other var is init by default value.
		 * the function add,mul,shift and so on is turned on by default,but the load and store is turned off default
		 * @param t_id : the id of the CGRANode
		 * @param t_x : the x of the CGRANode
		 * @param t_y : the y of the CGRANode 
		 */
		CGRANode(int t_id, int t_x, int t_y);
		~CGRANode();

		/**add the CGRALink to the list of this CGRANode's InCGRALinks
		 * @param t_link : the pointer of the in CGRALink
		 */
		void attachInLink(CGRALink* t_link);

		/**add the CGRALink to the list of this CGRANode's outCGRALinks
		 * @param t_link : the pointer of the out CGRALink
		 */
		void attachOutLink(CGRALink* t_link);


		int getID();
		int getx(){return m_x;};
		int gety(){return m_y;};
		bool hasDataMem(){return m_hasDataMem;};
		bool isdisable(){return m_disabled;};

		/**judge if this CGRANode can support Opt which opcodeName is t_optsname
		 * @param t_optsname: the name of the Opt
		 */
		bool canSupport(string t_optsname);

		list<CGRANode*>* getNeighbors();
		CGRALink* getOutLinkto(CGRANode* t_neighbor);
};
#endif
