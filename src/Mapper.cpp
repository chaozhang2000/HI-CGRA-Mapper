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

/**this funciton get ResMII,this is the min possible II.
 */
int Mapper::getResMII(){
	int ResMII=ceil(float(m_dfg->getInstNodeCount())/m_cgra->getNodeCount());
	return ResMII;
}

/** what is in this function:
 * 1. start from min II to try map
 * 2. clear MRRG,m_mapInfo
 * 3. traverse all InstNode in DFG, try to map them one by one, the InstNode is divided into two classes, InstNode whose all preNode(StartNode) is not mapped and InsoNode whose all preNode is mapped.For each kind of InstNode,try to find a path in MRRG to map them,if found,then schedule the path.
 *
 */
void Mapper::heuristicMap(){
	IFDEF(CONFIG_MAP_DEBUG,OUTS("\nMAP DEBUG",ANSI_FG_BLUE)); 

	bool mapsuccess = false;
 	//1. start from min II to try map
	while(1){
		IFDEF(CONFIG_MAP_DEBUG,OUTS("==================================",ANSI_FG_CYAN));
		IFDEF(CONFIG_MAP_DEBUG,OUTS("start heuristic algorithm with II="<<m_II,ANSI_FG_CYAN)); 

 		//2. clear MRRG,m_mapInfo
		m_mrrg->MRRGclear();
		mapInfoInit();
		
		//3. Traverse all InstNodes in dfg, try to map them one by one
		for(list<DFGNodeInst*>::iterator InstNode=m_dfg->getInstNodes()->begin();InstNode!=m_dfg->getInstNodes()->end(); InstNode ++){
			//handle the first situation, the InstNode is start DFGNode
			if(allPreInstNodeNotMapped(*InstNode)){
				IFDEF(CONFIG_MAP_DEBUG,OUTS("----------------------------------",ANSI_FG_CYAN));
				IFDEF(CONFIG_MAP_DEBUG,outs()<<"Try to find paths for StartDFGNode"<<(*InstNode)->getID()<<" to each CGRANode\n"); 
				PATH* path = getMapPathforStartInstNode(*InstNode);
				if(path != NULL){//find path
					int src1state = SRC_NOT_OCCUPY;
					int src2state = SRC_NOT_OCCUPY;
					getSrcStateOfStartNode(*InstNode,&src1state,&src2state);
					scheduleDstNodeInPath(path,*InstNode,src1state,src2state);
					setmapInfo(getPathEndCGRANode(path),*InstNode,getPathEndCycle(path));
					IFDEF(CONFIG_MAP_DEBUG,outs()<<"Find mincost Path: ");
					IFDEF(CONFIG_MAP_DEBUG,dumpPath(path));
					delete path;
					m_mrrg->submitschedule();
					if(next(InstNode,1) == m_dfg->getInstNodes()->end()){ mapsuccess = true;break;}
					continue;
				}
				else{//not find path, map failed add II try again
					IFDEF(CONFIG_MAP_DEBUG,outs()<<"Can't find path for Start InstNode"<<(*InstNode)->getID()<<"\nFaild mapping with II = "<<m_II<<"\n");
				break;
				}
			}
			//handle the second situation,the InstNode has all preInst Mapped
			else if(allPreInstNodeMapped(*InstNode)){
				IFDEF(CONFIG_MAP_DEBUG,OUTS("----------------------------------",ANSI_FG_CYAN));
				IFDEF(CONFIG_MAP_DEBUG,outs()<<"Try to find paths for DFGNode"<<(*InstNode)->getID()<<"(havePreNode) to each CGRANode\n"); 
				PATHS* paths = getMapPathsFromPreToInstNode(*InstNode);
				if(paths != NULL){
					for(PATH* path:*paths){
						scheduleLinkInPath(path);
					}
					scheduleDstNodeInPath(paths->front(),*InstNode,SRC_NOT_OCCUPY,SRC_NOT_OCCUPY);
					m_mrrg->submitschedule();
					setmapInfo(getPathEndCGRANode(paths->front()),*InstNode,getPathEndCycle(paths->front()));
					for(PATH* path: *paths){delete path;}
					delete paths;
					if(next(InstNode,1) == m_dfg->getInstNodes()->end()){ mapsuccess = true;break;}
					continue;
				}else{
					IFDEF(CONFIG_MAP_DEBUG,outs()<<"Can't find path for InstNode"<<(*InstNode)->getID()<<"\nFaild mapping with II = "<<m_II<<"\n");
					break;
				}
			}
			else{
				assert("this DFGInstNode has some preInstNode mapped and also has some preInstNode not mapped, this should not happened,Mapper has unfixed bugs");
			}
		}
		if(m_II >m_mrrg->getMRRGcycles()/2 or mapsuccess)break;//TODO:
		m_II ++;
	}
	IFDEF(CONFIG_MAP_DEBUG,OUTS("==================================\nMapping result",ANSI_FG_CYAN));
	if(mapsuccess)IFDEF(CONFIG_MAP_DEBUG,outs()<<"Mapping successful with II = "<<m_II<<"\n");	
	else IFDEF(CONFIG_MAP_DEBUG,outs()<<"Mapping failed with II = "<<m_II<<"\n");	
}

void Mapper::getSrcStateOfStartNode(DFGNodeInst* InstNode,int* src1state,int *src2state){
	for (DFGEdge* edge: *(InstNode->getInEdges())){
		int srcID = edge->getsrcID();
		int state = SRC_OCCUPY_FROM_CONST_MEM;
		if(DFGNodeParam* param_node = dynamic_cast<DFGNodeParam*>(edge->getSrc())){
			if(param_node->isloop()) state = m_mrrg->srcOccupyLoopMap[param_node->getloopID()];
		}
		if(srcID == 0) *src1state = state;
		if(srcID == 1) *src2state = state;
	}
}
/**this funciton judge if the all PreInstNode of t_InstNode is not mapped
 */
bool Mapper::allPreInstNodeNotMapped(DFGNodeInst* t_InstNode){
	for(DFGNodeInst* preInstNode: *(t_InstNode->getPredInstNodes())){
		if(m_mapInfo[preInstNode]->mapped == true){
			return false;
		}
	}
	return true;
}

/**this funciton judge if all PreInstNode of t_InstNode is mapped
 */
bool Mapper::allPreInstNodeMapped(DFGNodeInst* t_InstNode){
	for(DFGNodeInst* preInstNode: *(t_InstNode->getPredInstNodes())){
		if(m_mapInfo[preInstNode]->mapped == false){
			return false;
		}
	}
	return true;
}

//TODO:should not be any CGRANode
/**This function is used to find a path for a StartInstNode in MRRG,if found return the pointer of path,else return NULL
 * what is in this function:
 * 1. search every cgraNode to find a path,if found,pushed it to paths
 * 2. find the mincost path in paths and return
 */
PATH* Mapper::getMapPathforStartInstNode(DFGNodeInst* t_InstNode){
	PATHS paths;
	//1. search every cgraNode to find a path if found push it into paths
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
					IFDEF(CONFIG_MAP_DEBUG_PATH,outs()<<"Try to find path for StartDFGNode"<<t_InstNode->getID()<<" to CGRANode"<< cgraNode->getID()<<" success!\n  Path:");
					IFDEF(CONFIG_MAP_DEBUG_PATH,dumpPath(path));
					paths.push_back(path);
				}else{
					IFDEF(CONFIG_MAP_DEBUG_PATH,outs()<<"Try to find path for StartDFGNode"<<t_InstNode->getID()<<" to CGRANode"<< cgraNode->getID()<<" falied!\n"); 
				}
			}
		}
	}
 	//2. find the mincost path in paths and return
	if(paths.size()!= 0){
		PATH* mincostPath = getmaincostPath(&paths);
		for(PATH* deletepath : paths){//only save the mincostPath
			if(deletepath != mincostPath)
				delete deletepath;
		}
		return mincostPath;
	}else{
		return NULL;
	}
}

/**This function is used to find paths for a InstNode which have one or two preInstNode,if found return the pointer of paths,else return NULL
 * what is in this function:
 * 1. search every cgraNode consider the cgraNode as the target of mapping t_InstNode.
 * 2.if the t_InstNode have only one preNode, just find the path from it's preNode to this Node. only save the shortest path.
 * 3.if the t_InstNode have two preNode, find a path from one preNode to the cgraNode first,if find a path, schedule it,then route another preNode to the end of found path.if found save the paths,only save the shortest paths.
 */
PATHS* Mapper::getMapPathsFromPreToInstNode(DFGNodeInst* t_InstNode){
	int minendcycle = m_mrrg->getMRRGcycles()/2;
	PATHS* minpaths = NULL;
  //1. search every cgraNode consider the cgraNode as the target of mapping t_InstNode.
	for(int r = 0; r<m_cgra->getrows();r++){
		for(int c = 0; c< m_cgra->getcolumns();c++){
			CGRANode* cgraNode = m_cgra->nodes[r][c];
			if(cgraNode->canSupport(t_InstNode->getOpcodeName())and cgraNode->isdisable()==false ){
				list<DFGNodeInst*>* preNodes = t_InstNode->getPredInstNodes();

 				//2.if the t_InstNode have only one preNode, just find the path from it's preNode to this Node. only save the shortest path.
				if(preNodes->size() == 1){
					DFGNodeInst* preNode = preNodes->front();
					CGRANode* srccgraNode = m_mapInfo[preNode]->cgraNode;
					int srccycle = m_mapInfo[preNode]->cycle;
					PATH* pathtonode = getPathToCGRANode(srccgraNode,cgraNode,srccycle,minendcycle,false);
					if(pathtonode != NULL){
						IFDEF(CONFIG_MAP_DEBUG_PATH,outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<"(onePreNode) to CGRANode"<< cgraNode->getID()<<" success!\n  Path:");
						IFDEF(CONFIG_MAP_DEBUG_PATH,dumpPath(pathtonode));
						if(minpaths != NULL) {for(PATH* path: *minpaths){delete path;}delete minpaths;}
						minpaths = new PATHS;
						minpaths -> push_back(pathtonode);
						minendcycle= getPathEndCycle(pathtonode);
					}else{
						IFDEF(CONFIG_MAP_DEBUG_PATH,outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<"(onePreNode) to CGRANode"<< cgraNode->getID()<<" falied!\n"); 
						continue;
					}
 				//3.if the t_InstNode have two preNode, find a path from one preNode to the cgraNode first,if find a path, schedule it,then route another preNode to the end of found path.if found save the paths,only save the shortest paths.
				}else{
					PATH* pathstartfrom[2] = {NULL,NULL};
					PATH* pathroute[2] = {NULL,NULL};
					int endcycle[2];
					endcycle[0] = m_mrrg->getMRRGcycles();
					endcycle[1] = m_mrrg->getMRRGcycles();
					for(int i = 0; i<2 ;i++){//try to find a path start from one preNode first,because there are two preNode,so need try twice
						DFGNodeInst* predfgNode = i == 0 ? *(preNodes->begin()):*(next(preNodes->begin(),1));
						CGRANode* srccgraNode = m_mapInfo[predfgNode]->cgraNode;
						int srccycle = m_mapInfo[predfgNode]->cycle;
						pathstartfrom[i] = getPathToCGRANode(srccgraNode,cgraNode,srccycle,minendcycle,false);
						if(pathstartfrom[i] != NULL){//if the path from a srcNode is found,try to route another path to it
							IFDEF(CONFIG_MAP_DEBUG_PATH,outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<"(twoPreNode) from CGRANode"<< srccgraNode->getID() <<" to CGRANode"<< cgraNode->getID()<<" success!\n  Path:");
							IFDEF(CONFIG_MAP_DEBUG_PATH,dumpPath(pathstartfrom[i]));

							scheduleLinkInPath(pathstartfrom[i]);
							DFGNodeInst* anotherpredfgNode = i == 1 ? *(preNodes->begin()):*(next(preNodes->begin(),1));
							CGRANode* anotherSrccgraNode = m_mapInfo[anotherpredfgNode]->cgraNode;
							int anotherSrccycle = m_mapInfo[anotherpredfgNode]->cycle;
							int dstcycle = getPathEndCycle(pathstartfrom[i]);
							pathroute[i] = getPathToCGRANode(anotherSrccgraNode,cgraNode,anotherSrccycle,dstcycle,true);
							if(pathroute[i]!=NULL){
								endcycle[i] = getPathEndCycle(pathroute[i]);
								if(endcycle[i]<minendcycle) minendcycle = endcycle[i];
								IFDEF(CONFIG_MAP_DEBUG_PATH,outs()<<"Try to route form another preDFGNode to CGRANode"<< cgraNode->getID()<<" success!\n  Path:");
								IFDEF(CONFIG_MAP_DEBUG_PATH,dumpPath(pathroute[i]));
							}else{
								IFDEF(CONFIG_MAP_DEBUG_PATH,outs()<<"Try to route form another preDFGNode to CGRANode"<< cgraNode->getID()<<" falied!\n"); 
							}
							m_mrrg->clearUnsubmit();
						}else{
							IFDEF(CONFIG_MAP_DEBUG_PATH,outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<"(twoPreNode) from CGRANode"<< srccgraNode->getID() <<" to CGRANode"<< cgraNode->getID()<<" faild!\n");
						}
					}
					//finish search ,save minpaths
					if(pathroute[0] == NULL and pathroute[1]== NULL){//route failed
						if (pathstartfrom[0] != NULL) delete pathstartfrom[0];
						if (pathstartfrom[1] != NULL) delete pathstartfrom[1];
					}else{//route success;
						int betterPathID = endcycle[0] < endcycle[1] ? 0:1;
						int deletePathID = endcycle[0] < endcycle[1] ? 1:0;
						if(pathstartfrom[deletePathID] != NULL) delete pathstartfrom[deletePathID];
						if(pathroute[deletePathID] != NULL) delete pathroute[deletePathID];
						if(minpaths != NULL) {for(PATH* path: *minpaths){delete path;}delete minpaths;}
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

/**if the isroute == true.This function is used to find path from src_CGRANode at cycle src_cycle to dst_CGRANode at dst_cycle in MRRG.
 * else this function used to find a path from src_CGRANode at src_cycle to dst_CGRANode.the search range in MRRG is src_cycle to dst_cycle.
 * this function use BFS to find path and route now
 * TODO: BFS need two much time,try A* or ...
 */
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

		//because the path's frond occupied link in MRRG,so we need to consider the front end of path.
		PATH* frontpath = new PATH;
		(*frontpath)[currentMRRGNode.second] =currentMRRGNode.first;
		pair<CGRANode*,int> node = currentMRRGNode;
		while(preMRRGNode[node].first!=NULL){
			(*frontpath)[preMRRGNode[node].second] = preMRRGNode[node].first;
			node = preMRRGNode[node];
		}
		if(frontpath->size()>0) scheduleLinkInPath(frontpath);

		//Search
		//consider to neighbor CGRANode at next cycle
		for(CGRANode* neighbor : *(currentCGRANode->getNeighbors())){
			CGRALink* linktoNeighbor = m_cgra->getLinkfrom(currentCGRANode,neighbor);
			if(linktoNeighbor != NULL){
				if(m_mrrg->canOccupyLinkInMRRG(linktoNeighbor,currentcycle,1,m_II)and m_mrrg->canOccupyLinkInUnSubmit(linktoNeighbor,currentcycle,1,m_II)){
					pair<CGRANode*,int> nextMRRGNode = make_pair(neighbor,currentcycle + 1);
					preMRRGNode[nextMRRGNode] = currentMRRGNode;
					q.push(nextMRRGNode);
				}
			}
		}
		//consider delay on current CGRANode at next cycle
		if(canDelayInCGRANodeatCycle(currentCGRANode,currentcycle+1,&preMRRGNode)){
			pair<CGRANode*,int> nextMRRGNode = make_pair(currentCGRANode,currentcycle + 1);
			preMRRGNode[nextMRRGNode] = currentMRRGNode;
			q.push(nextMRRGNode);
		}

		m_mrrg->clearUnsubmit();
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
 * consider two situation,the data need to save in start CGRANode's fu result reg,or the data need to save the preCGRANode's input link.
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
	}else{//the cgraNode is not the start of path
		CGRALink* cgralink = m_cgra->getLinkfrom(preCGRANode,cgraNode);
		if(m_mrrg->canOccupyLinkInMRRG(cgralink,cycle-1,1,m_II)and m_mrrg->canOccupyLinkInUnSubmit(cgralink,cycle-1,1,m_II)){
			return true;
		}
	}
	return false;
}

/** this function find a path with min cost in paths
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

/** this function try to schedule the dstCGRANode in path, it will call m_mrrg->scheduleNode function,to create unsubmit schedule information.
 * @param t_InstNode: the dst DFGNodeInst at the end of path
 * @param path: the path 
 */
void Mapper::scheduleDstNodeInPath(PATH* path,DFGNodeInst* t_InstNode,int src1_state,int src2_state){
	PATH::reverse_iterator ri = path->rbegin();
	CGRANode* dstCGRANode = (*ri).second;
	m_mrrg->scheduleNode(dstCGRANode,t_InstNode,(*ri).first,1,m_II,src1_state,src2_state);
}

/** this function try to schedule the links in path, it will call m_mrrg->scheduleLink function,to create unsubmit schedule information.
 * if data delay in a CGRANode,we need to consider where this data is really saved.
 * for example CGRANode0(0)->CGRANode0(1)->CGRANode1(2),data delay in start CGRANode for one cycle,at this cycle we don't want CGRANode0's fu do other calculate,because it will modify the data in fu reg.so we need to use mrrg->scheduleNode to occupied CGRANode0.
 * path CGRANode0(0)->CGRANode1(1)->CGRANode1(2),data delay in CGRANode1 for one cycle,at this cycle,the data save in the reg in CGRALink from CGRANode0 to CGRANode1,so we need to occupied this Link using mrrg->scheduleLink.
 * @param t_InstNode: the dst DFGNodeInst at the end of path
 * @param path: the path 
 */
void Mapper::scheduleLinkInPath(PATH* path){
	PATH::iterator it = path->begin();
	int cyclepre = 0;
	int cyclecurrent = 0;
	CGRANode* cgraNodecurrent = NULL;
	CGRANode* cgraNodepre = NULL;
	CGRALink* currentLink = NULL;
	CGRALink* preLink = NULL;
	bool srcNodeDelay = true;
	for(it = path->begin();it != path->end();it++){
			cyclecurrent = (*it).first;
			cgraNodecurrent = (*it).second;
			if(cgraNodepre != cgraNodecurrent and cgraNodepre!=NULL){
				preLink = currentLink;
				currentLink = m_cgra->getLinkfrom(cgraNodepre,cgraNodecurrent);
			}

			if(it != path->begin()){//the first node in path don't need process

				if(cgraNodecurrent == cgraNodepre and srcNodeDelay){//handle the srcNode Delay
					if(next(it,1) != path->end())m_mrrg->scheduleNode(cgraNodepre,NULL,cyclecurrent,1,m_II,SRC_NOT_OCCUPY,SRC_NOT_OCCUPY); //if(next(it,1) != path->end()) to avoid the situation the all CGRANode in path is the same ,for example CGRANode0(0)->CGRANode0(1).avoid it's end Node is scheduled for more than one time.
				}else{//occupy the link
					srcNodeDelay = false;//srcNodedelay will never happen again if program arrive here
					int occupystate = LINK_OCCUPY_EMPTY;
					occupystate = cgraNodecurrent == cgraNodepre ? LINK_OCCUPY_EMPTY :  preLink == NULL ? LINK_OCCUPY_FROM_FU : m_mrrg->linkOccupyDirectionMap[preLink->getdirection()];//if cgraNodecurrent == cgraNodepre mean current link is occupied to save data,should be occupied with state = LINKOCCUPY_EMPTY,else if preLink == NULL,mean the the Link's data comes from the preNode's fu, set the occupied state = LINK_OCCUPY_FROM_FU,else the out link's data come's from the in link
					m_mrrg->scheduleLink(currentLink,cyclepre,1,m_II,occupystate);
				}	
			}	
			cyclepre= cyclecurrent;
			cgraNodepre = cgraNodecurrent;
	}
}

/** init(clear) the m_mapInfo. Ready to do mapping
 */
void Mapper::mapInfoInit(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		m_mapInfo[InstNode]->cgraNode = NULL;
		m_mapInfo[InstNode]->cycle = 0;
		m_mapInfo[InstNode]->mapped = false;
	}
}

/** set the m_mapInfo,call this function after the mrrg->submitschedule
 * mean the t_dstCGRANode's mapping is finished.
 * record DFGNodeInst's mapping result in m_mapInfo.
 */
void Mapper::setmapInfo(CGRANode*t_dstCGRANode,DFGNodeInst* t_InstNode,int t_cycle){
	m_mapInfo[t_InstNode]->cgraNode = t_dstCGRANode;
	m_mapInfo[t_InstNode]->cycle = t_cycle;
	m_mapInfo[t_InstNode]->mapped = true;
	
}

/**print the path
 */
void Mapper::dumpPath(PATH* path){
	 for(PATH::iterator it =path->begin();it != path->end();it++) {
				outs()<<"CGRANode"<<(it->second)->getID()<<"("<<it->first<<")"<<"->";
	 }
	 outs()<<"\n";
}

/**get the path end cycle
 */
int Mapper::getPathEndCycle(PATH* path){
	PATH::reverse_iterator rit = path->rbegin();
	return (*rit).first;
}

/**get the path end CGRANode
 */
CGRANode* Mapper::getPathEndCGRANode(PATH* path){
	PATH::reverse_iterator rit = path->rbegin();
	return (*rit).second;
}

Mapper::~Mapper(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		delete m_mapInfo[InstNode];
	}
}

