#ifndef MRRG_H
#define MRRG_H

#include "CGRA.h"
#include "DFGNodeInst.h"
#include <map>
using namespace std;

enum {SRC_NOT_OCCUPY,
				SRC_OCCUPY_FROM_N, 
				SRC_OCCUPY_FROM_S,
				SRC_OCCUPY_FROM_W,
				SRC_OCCUPY_FROM_E,
				SRC_OCCUPY_FROM_FU,
				SRC_OCCUPY_FROM_CONST_MEM,
				SRC_OCCUPY_FROM_LOOP0,
				SRC_OCCUPY_FROM_LOOP1,
				SRC_OCCUPY_FROM_LOOP2};

enum {LINK_NOT_OCCUPY,
				LINK_OCCUPY_WAIT,
				LINK_OCCUPY_FROM_N, 
				LINK_OCCUPY_FROM_S,
				LINK_OCCUPY_FROM_W,
				LINK_OCCUPY_FROM_E,
				LINK_OCCUPY_FROM_FU};

struct NodeInfo{
/** this var is used to record the CGRANode is occupyed by a DFGNode at certain cycle
* m_OccupyedByNode = new DFGNodeInst*[cycle];
*/
		DFGNodeInst** m_OccupiedByNode;

/**record if the Src1 or Src2 input mux is occpuied at a certain cycle,and it's occupied state
* for example
* SRC_OCCUPY_N,mean the mux is occupy and the src1 data is come from the input CGRALink from north
* SRC_OCCUPY_FU,mean the CGRALink is occupy and the output data is come from the fu out last time
*/
		int *m_Src1OccupyState;
		int *m_Src2OccupyState;

		int m_Mappednum;
};

struct LinkInfo{
/**record if the CGRALink is occpuied at a certain cycle,and it's occupied state
* for example
* LINK_OCCUPY_N,mean the CGRALink is occupy and the output data is come from the input from north
* LINK_OCCUPY_FU,mean the CGRALink is occupy and the output data is come from the fu out
*/
		int *m_occupied_state;
};

struct unSubmitNode{
	CGRANode* node;
	int cycle;

	DFGNodeInst* dfgNode;
	int Src1OccupyState;
	int Src2OccupyState;
	bool add_Mappednum;
};
struct unSubmitLink{
	CGRALink* link;
	int cycle;

	int OccupyState;
};

class MRRG {
  private:
		CGRA* m_cgra;

		int m_cycles;

		/** Used to save MRRG Data
		 */
		map<CGRANode*,NodeInfo*> m_NodeInfos;
		map<CGRALink*,LinkInfo*> m_LinkInfos;
		
		/** Used to save unsubmit Data
		 */
		list<unSubmitNode*> m_unSubmitNodes;
		list<unSubmitLink*> m_unSubmitLinks;

  public:
		/**The constructor function of class MRRG 
		 */
		MRRG(CGRA* t_cgra,int m_cycles);
		~MRRG();

		/**Clear the MRRG, init datas in it.
		 */
		void MRRGclear();
		void clearUnsubmit();

		int getMRRGcycles(){return m_cycles;}

		/**judge if the CGRANode in MRRG have space to map
		 * if the m_Mappednum >= II, mean the CGRANode in MRRG don't have space to map
		 */
		bool haveSpaceforNode(CGRANode*t_cgraNode,int t_II);

		/**judge if the cgraLink can be occupy in MRRG
		 */
		bool canOccupyLinkInMRRG(CGRALink* t_cgraLink,int t_cycle,int t_duration,int t_II);
		/**judge if the cgraLink can be occupy in unsubmit CGRALinks
		 */
		bool canOccupyLinkInUnSubmit(CGRALink* t_cgraLink,int t_cycle,int t_duration,int t_II);
		/**judge if the cgraNode can be occupy in MRRG
		 */
		bool canOccupyNodeInMRRG(CGRANode* t_cgraNode,int t_cycle,int t_duration,int t_II);


		/**schedule the CGRANode in the MRRG
		 */
		void scheduleNode(CGRANode* t_cgraNode,DFGNodeInst* t_dfgNode,int t_cycle,int duration,int t_II);

		/**schedule the CGRALink in the MRRG
		 */
		void scheduleLink(CGRALink* t_cgraLink,int t_cycle,int duration,int t_II);

		/**submit the change to MRRG
		 */
		void submitschedule();
};

#endif

