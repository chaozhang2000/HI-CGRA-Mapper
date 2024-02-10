#ifndef Mapper_H
#define Mapper_H

#include "CGRA.h"
#include "DFG.h"
#include "MRRG.h"
#include "common.h"
using namespace std;

typedef map<int,CGRANode*> PATH;
typedef list<map<int,CGRANode*>*> PATHS;

/**The class to record the DFGNodeInst map result, cgra record which CGRANode the DFGNodeInst is mapped to ,and the cycle record which clock is mapped to.mapped record if the DFGNodeInst is mapped
 */
struct MapInfo{
	CGRANode* cgraNode;
	int cycle;
	bool mapped;
};

class Mapper{
  private:
		CGRA* m_cgra;

		DFG* m_dfg;

		MRRG* m_mrrg;

		int m_II;

		/**used to record every DFGNodeInst's map result.
		 */
		map<DFGNodeInst*,MapInfo*> m_mapInfo;

		int getResMII();
		void mapInfoInit();
		void setmapInfo(CGRANode*,DFGNodeInst* t_InstNode,int t_cycle);
		bool allPreInstNodeNotMapped(DFGNodeInst* t_InstNode);
		bool allPreInstNodeMapped(DFGNodeInst* t_InstNode);
		PATH* getMapPathforStartInstNode(DFGNodeInst* t_InstNode);
		void getSrcStateOfStartNode(DFGNodeInst* InstNode,int* src1state,int *src2state);
		void getSrcStateOfNode(PATHS* paths,DFGNodeInst* InstNode,int* src1state,int *src2state);
		PATHS* getMapPathsFromPreToInstNode(DFGNodeInst* t_InstNode);
		PATH* getPathToCGRANode(CGRANode* src_CGRANode, CGRANode* dst_CGRANode, int src_cycle,int dst_cycle,bool isroute);
		bool canDelayInCGRANodeatCycle(CGRANode* cgraNode,int cycle,map<pair<CGRANode*,int>,pair<CGRANode*,int>>* MRRGpath);

		PATH* getmaincostPath(PATHS* paths);

		void scheduleDstNodeInPath(PATH* path,DFGNodeInst* t_InstNode,int src1_state,int src2_state);
		void scheduleLinkInPath(PATH* path,bool temp);

		int getPathEndCycle(PATH* path);
		CGRANode* getPathEndCGRANode(PATH* path);
		void dumpPath(PATH* path);
#ifdef CONFIG_MAP_A
		PATH* AxGetPath(CGRANode* src_CGRANode, CGRANode* dst_CGRANode, int src_cycle,int dst_cycle);
		int calculateCost(pair<CGRANode*,int>* currentnode,pair<CGRANode*,int>* nextnode,pair<CGRANode*,int>*dstnode);
		PATH* BFSgetPath(CGRANode* src_CGRANode, CGRANode* dst_CGRANode, int src_cycle,int dst_cycle);
#endif

  public:
		/**The constructor function of class MRRG 
		 * will set m_cgra,m_dfg,m_mrrg,m_II,and apply space for MapInfos for each DFGNodeInst.put them in m_mapInfo
		 */
		Mapper(DFG*t_dfg,CGRA* t_cgra,MRRG* t_mrrg);

		/** delete the MapInfo in m_mapInfo
		 */
		~Mapper();

		/** do heuristicMap
		 */
		void heuristicMap();

		/** return m_II
		 */
		int getII();

};
#endif

