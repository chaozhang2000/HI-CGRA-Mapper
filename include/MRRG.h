#ifndef MRRG_H
#define MRRG_H

#include "CGRA.h"
#include "DFGNodeInst.h"
#include <map>
using namespace std;

enum {  SRC_NOT_OCCUPY,
				SRC_OCCUPY_FROM_FU,
				SRC_OCCUPY_FROM_CONST_MEM,
				SRC_OCCUPY_FROM_N, 
				SRC_OCCUPY_FROM_S,
				SRC_OCCUPY_FROM_W,
				SRC_OCCUPY_FROM_E,
				SRC_OCCUPY_FROM_LOOP0,
				SRC_OCCUPY_FROM_LOOP1,
				SRC_OCCUPY_FROM_LOOP2};

enum {  LINK_NOT_OCCUPY,
				LINK_OCCUPY_EMPTY,
				LINK_OCCUPY_FROM_N, 
				LINK_OCCUPY_FROM_S,
				LINK_OCCUPY_FROM_W,
				LINK_OCCUPY_FROM_E,
				LINK_OCCUPY_FROM_FU};

/**this struct is used to record the information of a CGRANode in MRRG
 */
struct NodeInfo{
/** this var is used to record the CGRANode is occupyed by which DFGNode at certain cycle
* m_OccupyedByNode = new DFGNodeInst*[cycle];
*/
		DFGNodeInst** m_OccupiedByNode;
/** this var is used to record the CGRANode is occupyed or not at certain cycle.
 * if the m_occupied == true and m_OccupyedByNode ==NULL,mean this Node is occupied by empty Inst(data delay in fu reg)
 */
		bool * m_occupied;

/**record if the Src1 or Src2 input mux is occpuied at a certain cycle,and it's occupied state
* for example
* SRC_OCCUPY_N,mean the mux is occupy and the src1 data is come from the input CGRALink from north
* SRC_OCCUPY_FU,mean the CGRALink is occupy and the output data is come from the fu out last time
*/
		int *m_Src1OccupyState;
		int *m_Src2OccupyState;

/** record how many DFGInstNode has been mapped to this CGRANode
* m_OccupyedByNode = new DFGNodeInst*[cycle];
*/
		int m_Mappednum;
};

/**this struct is used to record the information of a CGRALink in MRRG
 */
struct LinkInfo{
/**record if the CGRALink is occpuied at a certain cycle,and it's occupied state
* for example
* LINK_OCCUPY_N,mean the CGRALink is occupy and the output data is come from the input from north
* LINK_OCCUPY_FU,mean the CGRALink is occupy and the output data is come from the fu out
*/
		int *m_occupied_state;
};

/**this struct is used to record the Possible but not yet submitted modification information for CGRANodes in MRRG
 */
struct unSubmitNodeInfo{
	CGRANode* node;
	int cycle;

	DFGNodeInst* dfgNode;
	int Src1OccupyState;
	int Src2OccupyState;
	bool add_Mappednum;//when submit,if cause m_Mappednum in NodeInfo ++
	bool temp;
};

/**this struct is used to record the Possible but not yet submitted modification information for CGRALink in MRRG
 */
struct unSubmitLinkInfo{
	CGRALink* link;
	int cycle;
	int OccupyState;
	bool temp;
};

class MRRG {
  private:
		CGRA* m_cgra;

		/** record the cycle size of MRRG
		 */
		int m_cycles;

		/** Used to save MRRG Data,each CGRANode* will have a NodeInfo to record information,and each CGRALink* have a LinkInfo to record information
		 */
		map<CGRANode*,NodeInfo*> m_NodeInfos;
		map<CGRALink*,LinkInfo*> m_LinkInfos;
		
		/** Used to save unsubmit Data
		 */
		list<unSubmitNodeInfo*> m_unSubmitNodeInfos;
		list<unSubmitLinkInfo*> m_unSubmitLinkInfos;

  public:
		/**The constructor function of class MRRG 
		 */
		MRRG(CGRA* t_cgra,int m_cycles);
		~MRRG();

		/** Used to map the LINK_DIRECTION to LINK_OCCUPY_FROM_
		 * now [4] is enough
		 */
		int linkOccupyDirectionMap[4];

		/** Used to map the LINK_DIRECTION to SRC_OCCUPY_FROM_
		 * now [4] is enough
		 */
		int srcOccupyDirectionMap[4];

		/** Used to map the loopID to SRC_OCCUPY_FROM_LOOP_
		 * now [3] is enough
		 */
		int srcOccupyLoopMap[3];

		/**Clear the MRRG, init datas in it,init the m_LinkInfos,m_NodeInfos,and clearUnsubmit
		 */
		void MRRGclear();

		/**free up space for unSubmitLinkInfo in m_unSubmitLinkInfos
		 * clear the list m_unSubmitLinkInfos
		 */
		void clearUnsubmit();

		/**
		 * clear the temp unsubmit link and node in list m_unSubmitLinkInfos and m_unSubmitNodeInfos
		 */
		void clearTempUnsubmit();

		/** return m_cycles
		 */
		int getMRRGcycles();

		/**judge if the CGRANode in MRRG have space to map
		 * if the m_Mappednum >= II, mean the CGRANode in MRRG don't have space to map
		 */
		bool haveSpaceforNode(CGRANode*t_cgraNode,int t_II);

		/**judge if the t_cgraLink can be occupy in MRRG,during cycle t_cycle to t_cycle+t_duration,when II = t_II
		 */
		bool canOccupyLinkInMRRG(CGRALink* t_cgraLink,int t_cycle,int t_duration,int t_II);
		/**judge if the cgraLink can be occupy in unsubmit CGRALinks,during cycle t_cycle to t_cycle+t_duration,when II = t_II
		 */
		bool canOccupyLinkInUnSubmit(CGRALink* t_cgraLink,int t_cycle,int t_duration,int t_II);
		/**judge if the cgraNode can be occupy in MRRG,during cycle t_cycle to t_cycle+t_duration,when II = t_II
		 */
		bool canOccupyNodeInMRRG(CGRANode* t_cgraNode,int t_cycle,int t_duration,int t_II);

		/**schedule the CGRANode,new unSubmitNodeInfo* and push_back to m_unSubmitNodeInfos
		 * call this function one time will cause one add_Mappednum in unSubmitNodeInfo be set true
		 * if finally call submitschedule will cause add_Mappednum in NodeInfo ++, if the Mappednum of t_cgraNode >= II, the Mapper will not map any DFGNodeInst to t_cgraNode,so don't call this function for the same occupied in MRRG for more than one time.TODO:fix this 
		 */
		void scheduleNode(CGRANode* t_cgraNode,DFGNodeInst* t_dfgNode,int t_cycle,int duration,int t_II,int t_Src1OccupyState,int t_Src2OccupyState,bool temp);

		/**schedule the CGRALink,new unSubmitLinkInfo* and push_back to m_unSubmitLinkInfos
		 */
		void scheduleLink(CGRALink* t_cgraLink,int t_cycle,int duration,int t_II,int occupy_state,bool temp);

		/** submit the unsubmitInfo to the MRRG(m_NodeInfos and m_LinkInfos)
		 * then clearUnsubmit
		 */
		void submitschedule();
};

#endif

