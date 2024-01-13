#ifndef Mapper_H
#define Mapper_H

#include "CGRA.h"
#include "DFG.h"
#include "MRRG.h"
using namespace std;

typedef map<int,CGRANode*> PATH;
typedef list<map<int,CGRANode*>*> PATHS;
class Mapper{
  private:
		CGRA* m_cgra;

		DFG* m_dfg;

		MRRG* m_mrrg;

		int m_II;

		/**The class to record the map info, cgra record which CGRANode the DFGNodeInst is mapped to ,and the cycle record which clock is mapped to.
		 */
		class MapInfo{
			public:
				CGRANode* cgraNode;
				int cycle;
				bool mapped;
		};
		map<DFGNodeInst*,MapInfo*> m_mapInfo;

		void mapInfoInit();
		bool allPreInstNodeNotMapped(DFGNodeInst* t_InstNode);
		bool allPreInstNodeMapped(DFGNodeInst* t_InstNode);
		PATH* getMapPathforStartInstNode(DFGNodeInst* t_InstNode);
		PATHS* getMapPathsFromPreToInstNode(DFGNodeInst* t_InstNode);
		PATH* getPathToCGRANode(CGRANode* src_CGRANode, CGRANode* dst_CGRANode, int src_cycle,int dst_cycle,bool isroute);
		bool canDelayInCGRANodeatCycle(CGRANode* cgraNode,int cycle,map<pair<CGRANode*,int>,pair<CGRANode*,int>>* MRRGpath);

		PATH* getmaincostPath(PATHS* paths);

		bool schedule(PATH* path,DFGNodeInst* t_InstNode,bool t_IncludeDstCGRANode);

		int getPathEndCycle(PATH* path);
		void dumpPath(PATH* path);

  public:
		/**The constructor function of class MRRG 
		 */
		Mapper(DFG*t_dfg,CGRA* t_cgra,MRRG* t_mrrg);
		~Mapper();

		int getResMII();

		void heuristicMap();
};
#endif

