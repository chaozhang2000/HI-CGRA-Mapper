#include "MRRG.h"
#include "config.h"

#define DIRECTION_MAP(f)\
				f(N) f(S) f(W) f(E)
#define LOOP_MAP(f)\
				f(0) f(1) f(2)
#define LINKOCCUPY(k) linkOccupyDirectionMap[LINK_DIRECTION_TO_##k] = LINK_OCCUPY_FROM_##k;
#define SRC_DIRECTION_OCCUPY(k) srcOccupyDirectionMap[LINK_DIRECTION_TO_##k] = SRC_OCCUPY_FROM_##k;
#define SRC_LOOP_OCCUPY(k) srcOccupyLoopMap[k] = SRC_OCCUPY_FROM_LOOP##k;

/**what is in this function:
 * 1. Apply space and init data for LinkInfos 
 * 2. Apply space and init data for NodeInfos 
 * 3. Apply space adn init data for DatamemInfos
 * 4. init some maps
 */
MRRG::MRRG(CGRA*t_cgra, int t_cycles){
	m_cgra = t_cgra;
	m_cycles = t_cycles;
	//1. Apply space and init data for LinkInfos
	for(int i=0;i<m_cgra->getLinkCount();i++){
		m_LinkInfos[m_cgra->links[i]] = new LinkInfo;
		m_LinkInfos[m_cgra->links[i]]->m_occupied_state = new int[m_cycles];
		m_LinkInfos[m_cgra->links[i]]->m_Mappednum = 0;
		m_LinkInfos[m_cgra->links[i]]->m_lastcycle = 0;
		for(int c=0;c<m_cycles;c++){
			m_LinkInfos[m_cgra->links[i]]->m_occupied_state[c] = LINK_NOT_OCCUPY;
		}
	}	
  //2. Apply space and init data for NodeInfos 
  for (int i=0; i<m_cgra->getrows(); ++i) {
    for (int j=0; j<m_cgra->getcolumns(); ++j) {
    	m_NodeInfos[m_cgra->nodes[i][j]]=new NodeInfo;
			m_NodeInfos[m_cgra->nodes[i][j]]->m_OccupiedByNode = new DFGNodeInst*[m_cycles];
			m_NodeInfos[m_cgra->nodes[i][j]]->m_Src1OccupyState = new int[m_cycles];
			m_NodeInfos[m_cgra->nodes[i][j]]->m_Src2OccupyState = new int[m_cycles];
			m_NodeInfos[m_cgra->nodes[i][j]]->m_fuinoccupied = new bool[m_cycles];
			m_NodeInfos[m_cgra->nodes[i][j]]->m_fuoutoccupied = new bool[m_cycles];
			m_NodeInfos[m_cgra->nodes[i][j]]->m_Mappednum = 0;
			m_NodeInfos[m_cgra->nodes[i][j]]->m_lastcycle = 0;
			for(int c=0;c<m_cycles;c++){
				m_NodeInfos[m_cgra->nodes[i][j]]->m_OccupiedByNode[c] = NULL;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_Src1OccupyState[c] = SRC_NOT_OCCUPY;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_Src2OccupyState[c] = SRC_NOT_OCCUPY;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_fuinoccupied[c] = false;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_fuoutoccupied[c] = false;
			}
    }
  }
	//3. Apply space adn init data for DatamemInfos
	for(int id = 0; id< config_info.datamemnum;++id){
	 m_DatamemInfos[id] = new bool[m_cycles];
		for(int c=0;c<m_cycles;c++){
			m_DatamemInfos[id][c] = false;
		}
	}
	//4. Init the map of CGRALink direction to CGRALink occupied state.
	DIRECTION_MAP(LINKOCCUPY);
	DIRECTION_MAP(SRC_DIRECTION_OCCUPY);
	LOOP_MAP(SRC_LOOP_OCCUPY);
}

/**what is in this function:
 * 1. delete m_LinkInfos 
 * 2. delete m_NodeInfos 
 * 3. delete m_unsubmitlink and m_unsubmitnode
 */
MRRG::~MRRG(){
  //1. delete m_LinkInfos 
	for(int i=0;i<m_cgra->getLinkCount();i++){
		delete[] m_LinkInfos[m_cgra->links[i]]->m_occupied_state;
		delete m_LinkInfos[m_cgra->links[i]];
	}	
  //2. delete m_NodeInfos 
  for (int i=0; i<m_cgra->getrows(); ++i) {
    for (int j=0; j<m_cgra->getcolumns(); ++j) {
			delete[] m_NodeInfos[m_cgra->nodes[i][j]]->m_OccupiedByNode;
			delete[] m_NodeInfos[m_cgra->nodes[i][j]]->m_Src1OccupyState;
			delete[] m_NodeInfos[m_cgra->nodes[i][j]]->m_Src2OccupyState;
			delete[] m_NodeInfos[m_cgra->nodes[i][j]]->m_fuinoccupied;
			delete[] m_NodeInfos[m_cgra->nodes[i][j]]->m_fuoutoccupied;
    	delete m_NodeInfos[m_cgra->nodes[i][j]];
    }
  }
	// delete DatamemInfos
	for(int i = 0; i< config_info.datamemnum; i++){
		delete [] m_DatamemInfos[i];
	}
  //3. delete m_unsubmitlink and m_unsubmitnode
	clearUnsubmit();
}

void MRRG::MRRGclear(){
	for(int i=0;i<m_cgra->getLinkCount();i++){
		for(int c=0;c<m_cycles;c++){
			m_LinkInfos[m_cgra->links[i]]->m_occupied_state[c] = LINK_NOT_OCCUPY;
		}
		m_LinkInfos[m_cgra->links[i]]->m_Mappednum = 0;
		m_LinkInfos[m_cgra->links[i]]->m_lastcycle = 0;
	}	
  for (int i=0; i<m_cgra->getrows(); ++i) {
    for (int j=0; j<m_cgra->getcolumns(); ++j) {
			for(int c=0;c<m_cycles;c++){
				m_NodeInfos[m_cgra->nodes[i][j]]->m_OccupiedByNode[c] = NULL;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_Src1OccupyState[c] = SRC_NOT_OCCUPY;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_Src2OccupyState[c] = SRC_NOT_OCCUPY;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_fuinoccupied[c] = false;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_fuoutoccupied[c] = false;
			}
				m_NodeInfos[m_cgra->nodes[i][j]]->m_Mappednum = 0;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_lastcycle = 0;
    }
  }
	for(int id = 0; id< config_info.datamemnum;++id){
		for(int c=0;c<m_cycles;c++){
			m_DatamemInfos[id][c] = false;
		}
	}
	clearUnsubmit();
}

void MRRG::clearUnsubmit(){
	for(unSubmitLinkInfo* unsubmitlink: m_unSubmitLinkInfos){
		delete unsubmitlink;
	}
	for(unSubmitNodeInfo* unsubmitnode: m_unSubmitNodeInfos){
		delete unsubmitnode;
	}
	m_unSubmitNodeInfos.clear();
	m_unSubmitLinkInfos.clear();
}

void MRRG::clearTempUnsubmit(){
	for(auto it = m_unSubmitLinkInfos.begin();it != m_unSubmitLinkInfos.end();){
		if((*it)->temp){
			delete *it;
			it = m_unSubmitLinkInfos.erase(it);
		}else{
			++ it;
		}
	}
	for(auto it = m_unSubmitNodeInfos.begin();it != m_unSubmitNodeInfos.end();){
		if((*it)->temp){
			delete *it;
			it = m_unSubmitNodeInfos.erase(it);
		}else{
			++ it;
		}
	}
}

bool MRRG::canOccupyLinkInMRRG(CGRALink* t_cgraLink,int t_cycle,int t_duration,int t_II){
	//in MRRG
	for(int c=t_cycle;c<m_cycles;c=c+t_II){
		for(int d = 0; d < t_duration and c+d < m_cycles; d++){
			if(m_LinkInfos[t_cgraLink]->m_occupied_state[c+d] != LINK_NOT_OCCUPY)
						return false;
		}
	}
	return true;
}

bool MRRG::canOccupyLinkInUnSubmit(CGRALink* t_cgraLink,int t_cycle,int t_duration,int t_II){
	for(unSubmitLinkInfo* unsubmitlink: m_unSubmitLinkInfos){
		if(unsubmitlink->link == t_cgraLink){
			for(int d = 0; d<t_duration; d++){
				if(unsubmitlink->cycle >= t_cycle+d and (unsubmitlink->cycle - t_cycle-d)%t_II == 0 and 
												unsubmitlink->OccupyState != LINK_NOT_OCCUPY){
					return false;
				}
			}
		}
	}
	return true;
}

bool MRRG::haveSpaceforNode(CGRANode*t_cgraNode,DFGNodeInst* t_dfgnode,int start_cycle,int t_II){
	for(int i = start_cycle;i<m_cycles/2;i++){
		if(canOccupyNodeInMRRG(t_cgraNode,t_dfgnode,i,1,t_II))return true;
	}
	return false;
}

bool MRRG::canOccupyNodeInMRRG(CGRANode* t_cgraNode,DFGNodeInst* t_dfgnode,int t_cycle,int t_duration,int t_II){
	int latency = t_dfgnode->getlatency();
	for(int c=t_cycle;c<m_cycles;c=c+t_II){
		for(int d= 0;d<t_duration and c+d < m_cycles;d++){
			if(m_NodeInfos[t_cgraNode]->m_fuinoccupied[c+d] == true || m_NodeInfos[t_cgraNode]->m_fuoutoccupied[c+d+latency] == true)return false;
			if(t_dfgnode->isMemOpts() and t_dfgnode->hasMemConstraint() and m_DatamemInfos[t_dfgnode->constraintToMem()][c+d] == true)
				return false;
		}
	}
	return true;
}

/*unsubmit node occupy only happens in scheduleLinkInPath so the t_dfgnode can only be NULL, so don't need to consider the complex situation.*/
bool MRRG::canOccupyNodeInUnSubmit(CGRANode* t_cgraNode,DFGNodeInst* t_dfgnode,int t_cycle,int t_duration,int t_II){
	for(unSubmitNodeInfo* unsubmitnode: m_unSubmitNodeInfos){
		if(unsubmitnode->node == t_cgraNode){
			for(int d = 0; d<t_duration; d++){
				if(unsubmitnode->cycle >= t_cycle+d and (unsubmitnode->cycle - t_cycle-d)%t_II == 0){
					return false;
				}
			}
		}
	}
	return true;
}

/**TODO: m_Src1OccupyState and m_Src2OccupyState not be handled yet
 */
void MRRG::scheduleNode(CGRANode* t_cgraNode,DFGNodeInst* t_dfgNode,int t_cycle,int duration,int t_II,int t_Src1OccupyState,int t_Src2OccupyState,bool temp){
	bool firsttime= true;
	for(int c=t_cycle;c<m_cycles;c=c+t_II){
		for(int d =0;d<duration and c+d < m_cycles;d++){
			unSubmitNodeInfo* unsubmitnode = new unSubmitNodeInfo;
			unsubmitnode->node = t_cgraNode;
			unsubmitnode->cycle = c+d;
			unsubmitnode->dfgNode = t_dfgNode;
			unsubmitnode->Src1OccupyState = t_Src1OccupyState;
			unsubmitnode->Src2OccupyState = t_Src2OccupyState;
			unsubmitnode->add_Mappednum = false;
			unsubmitnode->temp = temp;
			if(firsttime){
				unsubmitnode->add_Mappednum = true;
				firsttime= false;
			}
			m_unSubmitNodeInfos.push_back(unsubmitnode);
		}
	}
}

void MRRG::scheduleLink(CGRALink* t_cgraLink,int t_cycle,int duration,int t_II,int occupy_state,bool temp){
	bool firsttime = true;
	for(int c=t_cycle;c<m_cycles;c=c+t_II){
		for(int d = 0;d<duration and c+d < m_cycles;d++){
			unSubmitLinkInfo* unsubmitlink = new unSubmitLinkInfo;
			unsubmitlink-> link = t_cgraLink;
			unsubmitlink-> cycle = c+d;
			unsubmitlink-> OccupyState = occupy_state;
			unsubmitlink-> add_Mappednum = false;
			unsubmitlink-> temp = temp;
			if(firsttime){
				unsubmitlink->add_Mappednum = true;
				firsttime = false;
			}
			m_unSubmitLinkInfos.push_back(unsubmitlink);
		}
	}
}

void MRRG::submitschedule(){
	for(unSubmitNodeInfo* unsubmitnode: m_unSubmitNodeInfos){
		CGRANode* node = unsubmitnode->node;
		int cycle = unsubmitnode->cycle;
		if(m_NodeInfos[node]->m_fuinoccupied[cycle]==true){
			//outs()<<"Node"<<node->getID()<<" have been occupyed by DFGNode"<<m_NodeInfos[node]->m_OccupiedByNode[cycle]->getID()<<" at cycle "<<cycle<<"\n";
			outs()<<"Node"<<node->getID()<<" have been occupyed by DFGNode"<<" at cycle "<<cycle<<"\n";
			llvm_unreachable("The CGRANode in path can't be schedule,this should not happen, the Mapper has some bugs");
		}
		else if(unsubmitnode->temp != false){
			llvm_unreachable("the unsubmitnode->temp == true,this should not happen,the Mapper has some bugs");
		}
		else{
			DFGNodeInst* dfgnode = unsubmitnode->dfgNode;
			m_NodeInfos[node]->m_OccupiedByNode[cycle] = dfgnode;
			m_NodeInfos[node]->m_fuinoccupied[cycle] = true;
			if(dfgnode != NULL and dfgnode->isMemOpts() and dfgnode->hasMemConstraint()){
				m_DatamemInfos[dfgnode->constraintToMem()][cycle] = true;
			}
			m_NodeInfos[node]->m_Src1OccupyState[cycle] = unsubmitnode->Src1OccupyState;
			m_NodeInfos[node]->m_Src2OccupyState[cycle] = unsubmitnode->Src2OccupyState;
			if(unsubmitnode->add_Mappednum){
							m_NodeInfos[node]->m_Mappednum++;
							m_NodeInfos[node]->m_lastcycle = cycle > m_NodeInfos[node]->m_lastcycle ? cycle:m_NodeInfos[node]->m_lastcycle;
			}
			if(dfgnode != NULL){//occupied by InstNode
				if(cycle + dfgnode->getlatency() < m_cycles) m_NodeInfos[node]->m_fuoutoccupied[cycle + dfgnode->getlatency()] = true;
			}
		}
	}
	for(unSubmitLinkInfo* unsubmitlink: m_unSubmitLinkInfos){
		CGRALink* link = unsubmitlink->link;
		int cycle = unsubmitlink->cycle;
		if(m_LinkInfos[link]->m_occupied_state[cycle]!=LINK_NOT_OCCUPY){
			outs()<<"Link"<<link->getID()<<":from Node"<<link->getsrc()->getID()<<"->Node"<<link->getdst()->getID()<<" direction:"<<link->getdirection()<<"have been occupyed by state "<<m_LinkInfos[link]->m_occupied_state[cycle]<<" at cycle "<<cycle<<"\n";
			llvm_unreachable("The CGRALink in path can't be schedule,this should not happen, the Mapper has some bugs");
		}
		else if(unsubmitlink->temp != false){
			llvm_unreachable("the unsubmitlink->temp == true,this should not happen,the Mapper has some bugs");
		}
		else{
			m_LinkInfos[link]->m_occupied_state[cycle] = unsubmitlink->OccupyState;
			if(unsubmitlink->add_Mappednum){
							m_LinkInfos[link]->m_Mappednum++;
							m_LinkInfos[link]->m_lastcycle = cycle > m_LinkInfos[link]->m_lastcycle ? cycle:m_LinkInfos[link]->m_lastcycle;
			}
		}
	}
	clearUnsubmit();
}

int MRRG::getMRRGcycles(){
	return m_cycles;
}

int MRRG::getLastcycleofNode(CGRANode* node){
	return m_NodeInfos[node]->m_lastcycle;
}
int MRRG::getMapednumofNode(CGRANode* node){
	return m_NodeInfos[node]->m_Mappednum;
}
int MRRG::getLastcycleofLink(CGRALink* link){
	return m_LinkInfos[link]->m_lastcycle;
}
int MRRG::getMapednumofLink(CGRALink* link){
	return m_LinkInfos[link]->m_Mappednum;
}

int MRRG::getFirstcycleofNode(CGRANode* node){
	int startcyclenode = 0;
	int startcyclelink = 0;
	NodeInfo* nodeinfo = m_NodeInfos[node];
	for(int i=0;i<m_cycles;i++){
		if(nodeinfo->m_fuinoccupied[i] && nodeinfo->m_OccupiedByNode[i]!=NULL){
			startcyclenode = i;
			break;
		}
	}	

	bool find = false;
	for(int i = 0;i<m_cycles;i++){
		for(CGRANode* neighbor: *(node->getNeighbors())){
			CGRALink* link = node->getOutLinkto(neighbor);
			LinkInfo* linkinfo = m_LinkInfos[link];
			if(linkinfo->m_occupied_state[i]!=LINK_NOT_OCCUPY && linkinfo->m_occupied_state[i]!=LINK_OCCUPY_EMPTY){
				startcyclelink = i;
				find = true;
				break;
			}
		}
		if(find) break;
	}
	return startcyclenode > startcyclelink ? startcyclelink:startcyclenode;
}

NodeInfo* MRRG::getNodeInfoOf(CGRANode* node){
	return m_NodeInfos[node];
}

LinkInfo* MRRG::getLinkInfoOf(CGRALink* link){
	return m_LinkInfos[link];
}
