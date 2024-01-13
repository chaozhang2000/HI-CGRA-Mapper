#include "Mapper.h"
#include <cmath>
#include <queue>
#include "common.h"

using namespace std;
Mapper::Mapper(DFG* t_dfg,CGRA* t_cgra,MRRG* t_mrrg){
	m_cgra = t_cgra;
	m_dfg = t_dfg;
	m_mrrg = t_mrrg;
	m_II = getResMII();
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		m_mapInfo[InstNode] = new MapInfo;
	}
}

int Mapper::getResMII(){
	int ResMII=ceil(float(m_dfg->getInstNodeCount())/m_cgra->getNodeCount());
	return ResMII;
}

void Mapper::heuristicMap(){
#ifdef CONFIG_MAP_DEBUG 
	OUTS("\nMAP DEBUG",ANSI_FG_BLUE); 
#endif
	while(1){
#ifdef CONFIG_MAP_DEBUG 
		OUTS("==================================",ANSI_FG_CYAN); 
		OUTS("start heuristic algorithm with II="<<m_II,ANSI_FG_CYAN); 
#endif
		m_mrrg->MRRGclear();
		mapInfoInit();
		bool mapsuccess = false;

		//Traverse all InstNodes in dfg, try to map them one by one
		for(list<DFGNodeInst*>::iterator InstNode=m_dfg->getInstNodes()->begin();InstNode!=m_dfg->getInstNodes()->end(); InstNode ++){
			//handle the first situation, the InstNode is start DFGNode
			if(allPreInstNodeNotMapped(*InstNode)){
				PATH* path = getMapPathforStartInstNode(*InstNode);
				if(path != NULL){//find path
					schedule(path,*InstNode,true);
					delete path;
					m_mrrg->submitschedule();
					continue;
				}
				else{//not find path, map failed add II try again
#ifdef CONFIG_MAP_DEBUG 
					outs()<<"Can't find path for Start InstNode"<<(*InstNode)->getID()<<"\nFaild mapping with II = "<<m_II<<"\n";
#endif
				break;
				}
			}
			//handle the second situation,the InstNode has all preInst Mapped
			else if(allPreInstNodeMapped(*InstNode)){
				PATHS* paths = getMapPathsFromPreToInstNode(*InstNode);
				if(paths != NULL){
					bool firsttime = true;
					for(PATH* path:*paths){
						schedule(path,*InstNode,firsttime);
						m_mrrg->submitschedule();
						firsttime = false;
						delete path;
					}
					delete paths;
					if(next(InstNode,1) == m_dfg->getInstNodes()->end()){ mapsuccess = true;break;}
					continue;
				}else{
#ifdef CONFIG_MAP_DEBUG 
					outs()<<"Can't find path for InstNode"<<(*InstNode)->getID()<<"\nFaild mapping with II = "<<m_II<<"\n";
#endif
					break;
				}
			}
			else{
				assert("this DFGInstNode has some preInstNode mapped and also has some preInstNode not mapped, this should not happened,Mapper has unfixed bugs");
			}
		}
		m_II ++;
		if(m_II >m_mrrg->getMRRGcycles()/2 or mapsuccess)break;//TODO:
	}
}

bool Mapper::allPreInstNodeNotMapped(DFGNodeInst* t_InstNode){
	for(DFGNodeInst* preInstNode: *(t_InstNode->getPredInstNodes())){
		if(m_mapInfo[preInstNode]->mapped == true){
			return false;
		}
	}
	return true;
}
bool Mapper::allPreInstNodeMapped(DFGNodeInst* t_InstNode){
	for(DFGNodeInst* preInstNode: *(t_InstNode->getPredInstNodes())){
		if(m_mapInfo[preInstNode]->mapped == false){
			return false;
		}
	}
	return true;
}

//TODO:should not be any CGRANode
PATH* Mapper::getMapPathforStartInstNode(DFGNodeInst* t_InstNode){
	PATHS paths;
#ifdef CONFIG_MAP_DEBUG 
			outs()<<"\nTry to find paths for StartDFGNode"<<t_InstNode->getID()<<" to each CGRANode\n"; 
#endif
	//search every cgraNode to find a path if find push it into paths
	for(int r = 0; r< m_cgra->getrows();r++){
		for(int c = 0; c< m_cgra->getcolumns();c++){
			CGRANode* cgraNode = m_cgra->nodes[r][c];
			if(cgraNode->canSupport(t_InstNode->getOpcodeName()) and cgraNode->isdisable()==false){
				int cycle = 0;
				PATH*path = NULL;
				while(cycle <= m_mrrg->getMRRGcycles()/2){
					if(m_mrrg->canOccupyNodeInMRRG(cgraNode,cycle,1,m_II)==true){
						path = new PATH;
						(*path)[cycle] = cgraNode;
						break;
					}
					cycle ++;	
				}
				if(path!=NULL){
#ifdef CONFIG_MAP_DEBUG_PATH 
				outs()<<"Try to find path for StartDFGNode"<<t_InstNode->getID()<<" to CGRANode"<< cgraNode->getID()<<" success!\n  Path:";
				dumpPath(path);
#endif
				paths.push_back(path);
				}else{
#ifdef CONFIG_MAP_DEBUG_PATH 
				outs()<<"Try to find path for StartDFGNode"<<t_InstNode->getID()<<" to CGRANode"<< cgraNode->getID()<<" falied!\n"; 
#endif
				}
			}
		}
	}
	//check if find the paths,and choose the return path
	if(paths.size()!= 0){
	//find the mincostPath in paths
		PATH* mincostPath = getmaincostPath(&paths);
		for(PATH* deletepath : paths){//only save the mincostPath
			if(deletepath != mincostPath)
				delete deletepath;
		}
#ifdef CONFIG_MAP_DEBUG 
		outs()<<"Find mincost Path: ";
		dumpPath(mincostPath);
#endif
		return mincostPath;
	}else{//if not find a path,return NULL
		return NULL;
	}
}

PATHS* Mapper::getMapPathsFromPreToInstNode(DFGNodeInst* t_InstNode){
#ifdef CONFIG_MAP_DEBUG 
			outs()<<"\nTry to find paths for DFGNode"<<t_InstNode->getID()<<"(havePreNode) to each CGRANode\n"; 
#endif
	int minendcycle = m_mrrg->getMRRGcycles()/2;
	PATHS* minpaths = NULL;
	for(int r = 0; r<m_cgra->getrows();r++){
		for(int c = 0; c< m_cgra->getcolumns();c++){//for every CGRANode try to find paths
			CGRANode* cgraNode = m_cgra->nodes[r][c];
			if(cgraNode->canSupport(t_InstNode->getOpcodeName())and cgraNode->isdisable()==false ){
				list<DFGNodeInst*>* preNodes = t_InstNode->getPredInstNodes();
				if(preNodes->size() == 1){//when the DFGNodeInst only have one prenode.
					DFGNodeInst* preNode = preNodes->front();
					CGRANode* srccgraNode = m_mapInfo[preNode]->cgraNode;
					int srccycle = m_mapInfo[preNode]->cycle;
					PATH* pathtonode = getPathToCGRANode(srccgraNode,cgraNode,srccycle,minendcycle,false);
					if(pathtonode != NULL){//get Path to CGRANode.
#ifdef CONFIG_MAP_DEBUG_PATH 
						outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<"(onePreNode) to CGRANode"<< cgraNode->getID()<<" success!\n  Path:";
						dumpPath(pathtonode);
#endif
						PATHS* paths = new PATHS;
						paths -> push_back(pathtonode);
						if(minpaths != NULL) {for(PATH* path: *minpaths){delete path;}delete minpaths;}
						minpaths = paths;
						minendcycle= getPathEndCycle(pathtonode);
					}else{
#ifdef CONFIG_MAP_DEBUG_PATH 
						outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<"(onePreNode) to CGRANode"<< cgraNode->getID()<<" falied!\n"; 
#endif
						continue;
					}
				}else{//when the DFGNodeInst have two prenode.
					PATH* pathstartfrom[2] = {NULL,NULL};
					PATH* pathroute[2] = {NULL,NULL};
					int endcycle[2];
					endcycle[0] = m_mrrg->getMRRGcycles();
					endcycle[1] = m_mrrg->getMRRGcycles();
					for(int i = 0; i<2 ;i++){
						DFGNodeInst* predfgNode = i == 0 ? *(preNodes->begin()):*(next(preNodes->begin(),1));
						CGRANode* srccgraNode = m_mapInfo[predfgNode]->cgraNode;
						int srccycle = m_mapInfo[predfgNode]->cycle;
						pathstartfrom[i] = getPathToCGRANode(srccgraNode,cgraNode,srccycle,minendcycle,false);
						if(pathstartfrom[i] != NULL){//if the path from a srcNode is found,try to route another path to it
#ifdef CONFIG_MAP_DEBUG_PATH 
							outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<"(twoPreNode) from CGRANode"<< srccgraNode->getID() <<" to CGRANode"<< cgraNode->getID()<<" success!\n  Path:";
							dumpPath(pathstartfrom[i]);
#endif
							schedule(pathstartfrom[i],t_InstNode,false);
							DFGNodeInst* anotherpredfgNode = i == 1 ? *(preNodes->begin()):*(next(preNodes->begin(),1));
							CGRANode* anotherSrccgraNode = m_mapInfo[anotherpredfgNode]->cgraNode;
							int anotherSrccycle = m_mapInfo[anotherpredfgNode]->cycle;
							int dstcycle = getPathEndCycle(pathstartfrom[i]);
							pathroute[i] = getPathToCGRANode(anotherSrccgraNode,cgraNode,anotherSrccycle,dstcycle,true);
							if(pathroute[i]!=NULL){
								endcycle[i] = getPathEndCycle(pathroute[i]);
								if(endcycle[i]<minendcycle) minendcycle = endcycle[i];
#ifdef CONFIG_MAP_DEBUG_PATH 
								outs()<<"Try to route form another preDFGNode to CGRANode"<< cgraNode->getID()<<" success!\n  Path:";
								dumpPath(pathroute[i]);
#endif
							}else{
#ifdef CONFIG_MAP_DEBUG_PATH 
								outs()<<"Try to route form another preDFGNode to CGRANode"<< cgraNode->getID()<<" falied!\n"; 
#endif
							}
							m_mrrg->clearUnsubmit();
						}else{
#ifdef CONFIG_MAP_DEBUG_PATH 
						outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<"(twoPreNode) from CGRANode"<< srccgraNode->getID() <<" to CGRANode"<< cgraNode->getID()<<" faild!\n";
#endif
						}
					}
					//finish search ,get minpaths
					if(pathroute[0] == NULL and pathroute[1]== NULL){//route failed
						if (pathstartfrom[0] != NULL) delete pathstartfrom[0];
						if (pathstartfrom[1] != NULL) delete pathstartfrom[1];
					}else{//route success;
						int betterPathID = endcycle[0] < endcycle[1] ? 0:1;
						int deletePathID = endcycle[0] < endcycle[1] ? 1:0;
						if(pathstartfrom[deletePathID] != NULL) delete pathstartfrom[deletePathID];
						if(pathroute[deletePathID] != NULL) delete pathroute[deletePathID];
						minpaths = new PATHS;
						minpaths->push_back(pathroute[betterPathID]);
						minpaths->push_back(pathstartfrom[betterPathID]);
					}
				}
			}
		}
	}
#ifdef CONFIG_MAP_DEBUG 
	if(minpaths!=NULL){
		outs()<<"Find mincost Paths:\n";
		int cnt = 1;
		for(PATH* path:*minpaths){
			outs()<<cnt<<") ";
			dumpPath(path);
			cnt++;
		}
	}
#endif
	return minpaths;
}

PATH* Mapper::getPathToCGRANode(CGRANode* src_CGRANode, CGRANode* dst_CGRANode, int src_cycle,int dst_cycle,bool isroute){
	if(m_mrrg->haveSpaceforNode(dst_CGRANode,m_II) == false){
		return NULL;
	}
	map<pair<CGRANode*,int>,pair<CGRANode*,int>> preMRRGNode;
	queue<pair<CGRANode*,int>> q;
	pair<CGRANode*,int> MRRGpathend;
	bool success = false;

	//BFS find path
	pair<CGRANode*,int> startMRRGnode = make_pair(src_CGRANode,src_cycle);
	CGRANode* null = NULL;
	preMRRGNode[startMRRGnode] = make_pair(null,src_cycle);
	q.push(startMRRGnode);
	while(!q.empty()){
		pair<CGRANode*,int> currentMRRGNode = q.front();
		q.pop();
		CGRANode* currentCGRANode = currentMRRGNode.first;
		int currentcycle = currentMRRGNode.second;

		if(currentCGRANode == dst_CGRANode and m_mrrg->canOccupyNodeInMRRG(currentCGRANode,currentcycle,1,m_II)
			 and (isroute ? currentcycle == dst_cycle:true)//isroute add end condition
			 ) {
			success = true;			
			MRRGpathend = make_pair(currentCGRANode,currentcycle);
			break;
		}
		if(currentcycle >= dst_cycle) continue;
		//Search
		//consider to neighbor CGRANode at next cycle
		for(CGRANode* neighbor : *(currentCGRANode->getNeighbors())){
			CGRALink* linktoNeighbor = m_cgra->getEdgefrom(currentCGRANode,neighbor);
			if(m_mrrg->canOccupyLinkInMRRG(linktoNeighbor,currentcycle,1,m_II)and m_mrrg->canOccupyLinkInUnSubmit(linktoNeighbor,currentcycle,1,m_II)){
				pair<CGRANode*,int> nextMRRGNode = make_pair(neighbor,currentcycle + 1);
				preMRRGNode[nextMRRGNode] = currentMRRGNode;
				q.push(nextMRRGNode);
			}
		}
		//consider delay on current CGRANode at next cycle
		if(canDelayInCGRANodeatCycle(currentCGRANode,currentcycle+1,&preMRRGNode)){
			pair<CGRANode*,int> nextMRRGNode = make_pair(currentCGRANode,currentcycle + 1);
			preMRRGNode[nextMRRGNode] = currentMRRGNode;
			q.push(nextMRRGNode);
		}
	}
	//return the path
	if(!success){
		return NULL;
	}else{
		PATH* returnpath = new PATH;
		(*returnpath)[MRRGpathend.second] =MRRGpathend.first;
		pair<CGRANode*,int> node = MRRGpathend;
		while(preMRRGNode[node].first!=NULL){
			(*returnpath)[preMRRGNode[node].second] = preMRRGNode[node].first;
			node = preMRRGNode[node];
		}
		return returnpath;
	}
}

/**This function is used to judge if the path can delay one cycle at a cgraNode
 * is used in function getPathToCGRANode
 * @param: cgraNode the CGRANode we want to delay
 * @param: MRRGpath the MRRGpath record the preNodes of cgraNode
 * @param: cycle  the cgraNode delay at cycle for one cycle;
 */
bool Mapper::canDelayInCGRANodeatCycle(CGRANode* cgraNode,int cycle,map<pair<CGRANode*,int>,pair<CGRANode*,int>>* MRRGpath){
	pair<CGRANode*,int> preMRRGNode = make_pair(cgraNode,cycle-1);
	while(1){
		if(preMRRGNode.first != cgraNode) break;
		preMRRGNode = (*MRRGpath)[preMRRGNode];
	}
	CGRANode* preCGRANode = preMRRGNode.first;
	if(preCGRANode == NULL){//the cgraNode is the start of path
		if(m_mrrg->canOccupyNodeInMRRG(cgraNode,cycle,1,m_II)){
			return true;
		}
	}else{//the cgraNode is not the statr of path
		CGRALink* cgralink = m_cgra->getEdgefrom(preCGRANode,cgraNode);
		if(m_mrrg->canOccupyLinkInMRRG(cgralink,cycle-1,1,m_II)and m_mrrg->canOccupyLinkInUnSubmit(cgralink,cycle-1,1,m_II)){
			return true;
		}
	}
	return false;
}

/** this function try to find a path with min cost,(choose a path to schedule)
 * TODO: now only consider the distance cost,more should be consider later
 */
PATH* Mapper::getmaincostPath(PATHS* paths){
		PATH::reverse_iterator rit = paths->front()->rbegin();
		int mincost = (*rit).first;
		PATH* mincostpath = paths->front();
		for(PATH* path : *paths){
				rit = path->rbegin();
				if((*rit).first < mincost){
					mincostpath = path;
					mincost = (*rit).first;
				}
		}
		return mincostpath;
}

/** this function try to schedule the path in MRRG
 * @param t_InstNode: the dst DFGNode at the end of path
 *
 * TODO:handle the CGRANode and CGRALink wait status.
 */
bool Mapper::schedule(PATH* path,DFGNodeInst* t_InstNode,bool t_IncludeDstCGRANode){
	//schedule the Node.
	if(t_IncludeDstCGRANode){
	PATH::reverse_iterator ri = path->rbegin();
	CGRANode* dstCGRANode = (*ri).second;
#ifdef CONFIG_MAP_DEBUG_SCHEDULE 
			outs()<<"Schedule DFG node["<<t_InstNode->getID()<<"] onto CGRANode["<<dstCGRANode->getID()<<"] at cycle "<< (*ri).first <<" with II: "<<m_II<<"\n"; 
#endif
	m_mrrg->scheduleNode(dstCGRANode,t_InstNode,(*ri).first,1,m_II);
	m_mapInfo[t_InstNode]->cgraNode = dstCGRANode;
	m_mapInfo[t_InstNode]->cycle = (*ri).first;
	m_mapInfo[t_InstNode]->mapped = true;
	}
	
	//schedule the Link.
	
	PATH::iterator it = path->begin();
	int cyclepre = 0;
	int cyclecurrent = 0;
	CGRANode* cgraNodecurrent = NULL;
	CGRANode* cgraNodepre = NULL;
	CGRALink* currentLink = NULL;
	bool srcNodeDelay = true;
	for(it = path->begin();it != path->end();it++){
			cyclecurrent = (*it).first;
			cgraNodecurrent = (*it).second;
			if(cgraNodepre != cgraNodecurrent and cgraNodepre!=NULL){
				currentLink = m_cgra->getEdgefrom(cgraNodepre,cgraNodecurrent);
			}

			if(it != path->begin()){//the first node in path don't need process

				//handle the srcNode Delay
				if(cgraNodecurrent == cgraNodepre and srcNodeDelay){
					m_mrrg->scheduleNode(cgraNodepre,t_InstNode,cyclecurrent,1,m_II);//TODO:should not occupied by t_InstNode,should be occupied by empty Inst
				}else{
					srcNodeDelay = false;//srcNodedelay will never happen again if program arrive here
#ifdef CONFIG_MAP_DEBUG_SCHEDULE 
					outs()<<"Schedule CGRALink["<<currentLink->getID()<<"] from CGRANode["<<(*path)[cyclepre]->getID()<<"] to CGRANode["<< (*path)[cyclecurrent]->getID()<<"] at cycle "<<cyclepre<<" to cycle "<<cyclepre + 1<<" with II = "<<m_II<<"\n"; 
#endif
					m_mrrg->scheduleLink(currentLink,cyclepre,1,m_II);
				}	
			}	
			cyclepre= cyclecurrent;
			cgraNodepre = cgraNodecurrent;
	}
	return true;
}

void Mapper::mapInfoInit(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		m_mapInfo[InstNode]->cgraNode = NULL;
		m_mapInfo[InstNode]->cycle = 0;
		m_mapInfo[InstNode]->mapped = false;
	}
}

void Mapper::dumpPath(PATH* path){
	 for(PATH::iterator it =path->begin();it != path->end();it++) {
				outs()<<"CGRANode"<<(it->second)->getID()<<"("<<it->first<<")"<<"->";
	 }
	 outs()<<"\n";
}

int Mapper::getPathEndCycle(PATH* path){
	PATH::reverse_iterator rit = path->rbegin();
	return (*rit).first;
}

Mapper::~Mapper(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		delete m_mapInfo[InstNode];
	}
}

