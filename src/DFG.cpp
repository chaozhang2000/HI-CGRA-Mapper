#include "DFG.h"
#include <llvm/Analysis/LoopInfo.h>
#include <fstream>
#include "common.h"
#include <limits>

using namespace std;
/**
 * How this function is implemented:
 * 1. init some var
 * 2. use construct() function to construct DFG
 */
DFG::DFG(Function& t_F,int t_loopargnum) {
	DFG_error = false;
  construct(t_F,t_loopargnum);
}

/**
 * How this function is implemented:
 * 1. delete the DFGNodes in nodes list
 * 2. delete the DFGEdges in m_DFGEdges
 */
DFG::~DFG() {
				list<DFGNodeInst*>::iterator inst_node_it;
				for (inst_node_it=m_InstNodes.begin();inst_node_it!=m_InstNodes.end();++inst_node_it){
					delete *inst_node_it;
				}
				list<DFGNodeConst*>::iterator const_node_it;
				for (const_node_it=m_ConstNodes.begin();const_node_it!=m_ConstNodes.end();++const_node_it){
					delete *const_node_it;
				}
				list<DFGEdge*>::iterator edge_it;
				for (edge_it=m_DFGEdges.begin();edge_it!=m_DFGEdges.end();++edge_it){
					delete *edge_it;
				}
}

/** Used to determine whether the instruction should be ignored(not add to the dfg)
 * @param t_inst instruction that need to be judged
 * @return true if the inst should be ignored
 */
bool DFG::shouldIgnore(Instruction* t_inst) {
	string ret = "ret";
	if(t_inst->getOpcodeName()== ret)return true;
	else return false;
}

 /** Extract DFG from specific function 
 	* @param t_F the function pointer which the mapperPass is processing
	*
	* what is in his function
	* 1. Clear the data structions which is used to save nodes and edges
	* 2. make sure there has only one basic block in target function,and every inst has less than two operands
	* 3. create DFGNode for all inst,const,and function argument
	* 4. create DFGEdge
	* 5. connect all DFGNode to generate the DFG,and reorder the DFGNode and give every node a level.
  * 6. print some information,if debug print is enable.
 */
void DFG::construct(Function& t_F,int t_loopargnum) {
	//1.Clear the data structions which is used to save nodes and edges
  m_DFGEdges.clear();
  m_InstNodes.clear();
  int nodeID = 0;
  int dfgEdgeID = 0;
	

	//2.make sure this function has only one basic block
	if(t_F.getBasicBlockList().size()>1) {
		DFG_ERR("The II have more then one basic block!");
	}
	//make sure every inst has less than two operands
  for (BasicBlock::iterator II=t_F.begin()->begin(),IEnd=t_F.begin()->end(); II!=IEnd; ++II) {
    Instruction* curII = &*II;
		if (curII->getNumOperands()>2){
			DFG_ERR("The Inst have more then two operands, not support yet!\n"<<changeIns2Str(curII));
		}
	}


	//3.Construct dfg nodes
	IFDEF(CONFIG_DFG_DEBUG,
	OUTS("\nDFG DEBUG",ANSI_FG_BLUE); 
	OUTS("==================================",ANSI_FG_CYAN); 
  OUTS("[constructing DFG of target function: "<< t_F.getName().str()<<"]",ANSI_FG_CYAN););
	//3.1 InstNode
  for (BasicBlock::iterator II=t_F.begin()->begin(),IEnd=t_F.begin()->end(); II!=IEnd; ++II) {
    Instruction* curII = &*II;
    // Ignore this IR if it is ret.
    if (shouldIgnore(curII)) {
      continue;
    }
    DFGNodeInst* dfgNodeInst;
    dfgNodeInst = new DFGNodeInst(nodeID++,curII,MUXDEF(CONFIG_DFG_FULL_INST,changeIns2Str(curII),curII->getOpcodeName()));
    m_InstNodes.push_back(dfgNodeInst);

		IFDEF(CONFIG_DFG_DEBUG,outs()<< *curII<<" -> (dfgNode ID: "<<dfgNodeInst->getID()<<")\n");
  }
	//3.2 const nodes
  for (BasicBlock::iterator II=t_F.begin()->begin(),IEnd=t_F.begin()->end(); II!=IEnd; ++II) {
    Instruction* curII = &*II;
    for (Instruction::op_iterator op = curII->op_begin(), opEnd = curII->op_end(); op != opEnd; ++op) {
			if(ConstantData* const_data = dyn_cast<ConstantData>(*op)){
				if(getConstNode(const_data) == NULL){
					ConstantInt * const_int = dyn_cast<ConstantInt>(*op);
					if (const_int==NULL){
						DFG_ERR("The Inst's const is not Int, not support yet!\n"<<changeIns2Str(curII));
					}
					string name;
					raw_string_ostream stream(name);
					stream << *const_int;
					DFGNodeConst* constNode = new DFGNodeConst(nodeID++,const_data,stream.str());
					m_ConstNodes.push_back(constNode);
					IFDEF(CONFIG_DFG_DEBUG,outs()<< *const_data<<" -> (dfgNode ID: "<<constNode->getID()<<")\n");
				}
			}
  	}
	}
	//3.2 param nodes
	int loopargID = 0;
	int argsize = t_F.arg_size();
	for(Argument& arg : t_F.args()){
		if(getParamNode(&arg) == NULL){
			string name;
			raw_string_ostream stream(name);
			stream << arg;
			bool isloop = (int)arg.getArgNo() >= argsize-t_loopargnum;
			DFGNodeParam*	paramNode = new DFGNodeParam(nodeID++,&arg,stream.str(),isloop,isloop?loopargID:0);
			if(isloop)loopargID++;
			m_ParamNodes.push_back(paramNode);
			IFDEF(CONFIG_DFG_DEBUG,outs()<< arg<<" -> (dfgNode ID: "<<paramNode->getID()<<")\n");
		}
	}

  //4.Construct data flow edges.
  for (DFGNodeInst* nodeInst: m_InstNodes) {
    Instruction* curII = nodeInst->getInst();
		int opcnt = 0;
    for (Instruction::op_iterator op = curII->op_begin(), opEnd = curII->op_end(); op != opEnd; ++op) {
			//the operands comes from other inst, need to create DFGEdge
			//if 
    	if (Instruction* tempInst = dyn_cast<Instruction>(*op)) {
      	DFGEdge* dfgEdge;
        dfgEdge = new DFGEdge(dfgEdgeID++,opcnt,getInstNode(tempInst), nodeInst);
        m_DFGEdges.push_back(dfgEdge);
				opcnt ++;
      } 
			//if the operand is a const
			else if(ConstantData* const_data = dyn_cast<ConstantData>(*op)){
        DFGEdge* dfgEdge;
        dfgEdge = new DFGEdge(dfgEdgeID++,opcnt,getConstNode(const_data), nodeInst);
        m_DFGEdges.push_back(dfgEdge);
				opcnt ++;
      } 
			//if the operand is a param
			else if (Argument* arg = dyn_cast<Argument>(op)){
        DFGEdge* dfgEdge;
        dfgEdge = new DFGEdge(dfgEdgeID++,opcnt,getParamNode(arg), nodeInst);
        m_DFGEdges.push_back(dfgEdge);
				opcnt ++;
			}
			else{
				DFG_ERR("The Inst have unknow operand!\n"<<changeIns2Str(curII));
			}
    }
	}

	//5. connectDFGNode and reorder.
  connectDFGNodes();
	reorderInstNodes();

	//6. print summary information if debug is enable
	IFDEF(CONFIG_DFG_DEBUG,showOpcodeDistribution());
}

/**reorder the InstNodes, give every Node a level,and reorder them according to the leavel.
 * what is in his function
 * 1. find the longest path in dfg
 * 2. set level for nodes,first the nodes in longestPath,second other node;
 * 3. reorder the InstNode in DFG
 */
void DFG::reorderInstNodes(){

	//1.find the longest path in dfg
	list<DFGNodeInst*> longestPath;
	DFS_findlongest(&longestPath);
	IFDEF(CONFIG_DFG_DEBUG,
	OUTS("==================================",ANSI_FG_CYAN); 
  OUTS("[Reorder Inst Nodes]",ANSI_FG_CYAN);
	outs()<<"longestPath: ";
	for(DFGNodeInst* InstNode:longestPath){
		outs()<<"Node"<<InstNode->getID()<<" -> ";
	}
	outs()<<"end\n");
	IFDEF(CONFIG_DFG_LONGEST,changeLongestPathColor(&longestPath,"orange"));

  //2.set level for nodes,first the nodes in longestPath,second other node;
	set<DFGNodeInst*> havenotSetLevelNodes;
	int maxlevel = 0;
	maxlevel =setLevelLongestPath(&longestPath,&havenotSetLevelNodes) ;
	setLevelforOtherNodes(&havenotSetLevelNodes);

	//3.reorder the InstNode in DFG,and save in m_InstNodes
	list<DFGNodeInst*> temp;
	for(int i = 0;i<=maxlevel;i++){
		for(DFGNodeInst* InstNode: m_InstNodes){
			if(InstNode->getLevel() == i){
				temp.push_back(InstNode);
			}
		}
	}
	m_InstNodes.clear();
	for(DFGNodeInst* InstNode: temp){
		m_InstNodes.push_back(InstNode);
		IFDEF(CONFIG_DFG_DEBUG,outs()<<"Node"<<InstNode->getID()<<" "<<" level: "<< InstNode->getLevel()<<*(InstNode->getInst())<<"\n");
	}
}

/** get the DFGEdge form t_src DFGInstNode to t_dst DFGInstNode
 */
DFGEdge* DFG::getEdgefrom(DFGNodeInst* t_src,DFGNodeInst* t_dst){
	for(DFGEdge* edge: m_DFGEdges){
		if(dynamic_cast<DFGNodeInst*>(edge->getSrc()) == t_src and dynamic_cast<DFGNodeInst*>(edge->getDst())== t_dst){
			return edge;
		}
	}
	llvm_unreachable("ERROR cannot find the DFGEdge from src to dst");
	return NULL;
}

/** set Level for the InstNode on the longest Path in DFG,and erase them from havenotSetLevelNodes list
*/
int DFG::setLevelLongestPath(list<DFGNodeInst*>*longestPath,set<DFGNodeInst*>* havenotSetLevelNodes){
	int level = 0;
	for(DFGNodeInst* InstNode: m_InstNodes){
					havenotSetLevelNodes->insert(InstNode);
	}
	// set level for node in the longest path
	for(DFGNodeInst* InstNode: *longestPath){
		InstNode -> setLevel(level);
		havenotSetLevelNodes->erase(InstNode);
		level ++;
	}
	return level;
}

/** set Level for the InstNode not on the longest Path in DFG,and erase them from havenotSetLevelNodes list
 * 1.search all nodes which have not been set level,and get four kinds of nodes,(1)nodes where all succnodes have level.(2)nodes where all prednodes have level.(3)node has highest proportion of succnodes which has been set(4)node has highest proportion of prednodes which has been set
 * 2.set level for (1)nodes,then search all nodes to get four kinds of nodes again.if kind (1) nodes not exsist,then set level for kind(2) nodes,and so on.
*/
void DFG::setLevelforOtherNodes(set<DFGNodeInst*>* havenotSetLevelNodes){
		DFGNodeInst* mostSuccNodeHasLevel;
		DFGNodeInst* mostPredNodeHasLevel;
		set<DFGNodeInst*> allSuccNodeHasLevel;
		set<DFGNodeInst*> allPredNodeHasLevel;
		float succNodeHasLevelPercent=0;
		float predNodeHasLevelPercent=0;
	while(havenotSetLevelNodes->size() > 0){
		allSuccNodeHasLevel.clear();
		allPredNodeHasLevel.clear();
		mostSuccNodeHasLevel = NULL;
		mostSuccNodeHasLevel = NULL;
		succNodeHasLevelPercent = 0;
		predNodeHasLevelPercent = 0;
		//find four kinds InstNodes
		for(DFGNodeInst* InstNode:*havenotSetLevelNodes){
						bool flag = true;
						int succnotfindcnt = 0;
						for(DFGNodeInst* succNode:*(InstNode->getSuccInstNodes())){
							if(havenotSetLevelNodes->find(succNode) != havenotSetLevelNodes->end()){
											flag = false;
											succnotfindcnt ++;
							}
						}
						if(flag == true)allSuccNodeHasLevel.insert(InstNode);
						if(succNodeHasLevelPercent < (InstNode->getSuccInstNodes()->size()==0)?0:1-((float)succnotfindcnt)/((float)(InstNode->getSuccInstNodes()->size()))){
							succNodeHasLevelPercent = (InstNode->getSuccInstNodes()->size()==0)?0:1-((float)succnotfindcnt)/((float)(InstNode->getSuccInstNodes()->size()));
							mostSuccNodeHasLevel = InstNode;
						}
						

						flag = true;
						int prednotfindcnt = 0;
						for(DFGNodeInst* predNode:*(InstNode->getPredInstNodes())){
							if(havenotSetLevelNodes->find(predNode) != havenotSetLevelNodes->end()){
											flag = false;
											prednotfindcnt ++;					
							}
						}
						if(flag == true)allPredNodeHasLevel.insert(InstNode);
						if(predNodeHasLevelPercent < (InstNode->getPredInstNodes()->size()==0)?0:1-((float)prednotfindcnt)/((float)(InstNode->getPredInstNodes()->size()))){
							predNodeHasLevelPercent = (InstNode->getPredInstNodes()->size()==0)?0:1-((float)prednotfindcnt)/((float)(InstNode->getPredInstNodes()->size()));
							mostPredNodeHasLevel = InstNode;
						}
		}
		//give kind(1) nodes level
		if(allSuccNodeHasLevel.size() > 0){
						for(DFGNodeInst* InstNode:allSuccNodeHasLevel){
							int level = numeric_limits<int>::max();	
							for(DFGNodeInst* succNode:*(InstNode->getSuccInstNodes())){
								if(level > succNode->getLevel()-1) level = succNode->getLevel() - 1;
							}
							InstNode->setLevel(level);
							havenotSetLevelNodes->erase(InstNode);
						}
			continue;
		//give kind(2) nodes level
		}else if (allPredNodeHasLevel.size()>0){
						for(DFGNodeInst* InstNode:allPredNodeHasLevel){
							int level = 0;	
							for(DFGNodeInst* predNode:*(InstNode->getPredInstNodes())){
								if(level < predNode->getLevel()+1) level = predNode->getLevel() + 1;
							}
							InstNode->setLevel(level);
							havenotSetLevelNodes->erase(InstNode);
						}
			continue;
		//give kind(3) node level
		}else if(mostSuccNodeHasLevel !=NULL){
							int level = numeric_limits<int>::max();	
							for(DFGNodeInst* succNode:*(mostSuccNodeHasLevel->getSuccInstNodes())){
								if(level > succNode->getLevel()-1 and succNode->haveSetLevel()) level = succNode->getLevel() - 1;
							}
							mostPredNodeHasLevel->setLevel(level);
							havenotSetLevelNodes->erase(mostPredNodeHasLevel);
			continue;
		//give kind(4) node level
		}else if(mostPredNodeHasLevel !=NULL){
							int level = 0;
							for(DFGNodeInst* predNode:*(mostPredNodeHasLevel->getPredInstNodes())){
								if(level < predNode->getLevel()+1) level = predNode->getLevel() + 1;
							}
							mostPredNodeHasLevel->setLevel(level);
							havenotSetLevelNodes->erase(mostPredNodeHasLevel);
			continue;
		}else{
						llvm_unreachable("ERROR has DFGNode which do not have INEdge and OutEdge");
			continue;
		}
	}
}

/** change the DFGEdge's color in Longest Path in DFG
*/
void DFG::changeLongestPathColor(list<DFGNodeInst*>* t_longestPath,string t_color){
	list<DFGNodeInst*>::iterator itr = t_longestPath->begin();
	DFGNodeInst* src = *itr;
	DFGNodeInst* dst = NULL;
	DFGEdge* edge = NULL;
	for(auto it=next(itr);it != t_longestPath->end();++it){
					dst = *it;
					edge = getEdgefrom(src,dst);
					edge->setcolor(t_color);
					src = *it;
	}
}
 /** 
	* get longest Path in DFG.
	* what is in his function
	* Traverse all nodes, treat each node as the initial node, and use DFS to find the longest path from the starting node in DFG. Save the longest path starting from different starting nodes.
	*
 */
void DFG::DFS_findlongest(list<DFGNodeInst*>* t_longestPath){
	list<DFGNodeInst*> currentPath;
	set <DFGNodeInst*> visited;
	for(DFGNodeInst* InstNode:m_InstNodes){
		currentPath.clear();
		visited.clear();
		reorderDFS(&visited,t_longestPath,&currentPath,InstNode);
	}
}

 /** 
	* get longest Path in DFG from start node.
	* use DFS to find the longest path from the starting node in DFG.
	* DFS(Depth-First Search) algorithm is an algorithm used to traverse or search for data in graphs. It starts from the starting node and follows a path as deep as possible into each previously unreachable node in the graph until it reaches the deepest node. Then, go back to the previous node and continue exploring other branches until all nodes are accessed.
 */
void DFG::reorderDFS(set<DFGNodeInst*>* t_visited, list<DFGNodeInst*>* t_targetPath,
								list<DFGNodeInst*>* t_curPath, DFGNodeInst* targetInstNode){
	t_visited->insert(targetInstNode);	
	t_curPath->push_back(targetInstNode);
	if(t_curPath->size() > t_targetPath->size()){
		t_targetPath->clear();
		for(DFGNodeInst* InstNode: *t_curPath){
			t_targetPath->push_back(InstNode);
		}
	}
	for(DFGNodeInst* succNode: *(targetInstNode->getSuccInstNodes())){
		if(t_visited->find(succNode) == t_visited->end()){
			reorderDFS(t_visited,t_targetPath,t_curPath,succNode);
			t_visited->erase(succNode);
			t_curPath->pop_back();
		}
	}
}

 /** 
	* this function is used to connect DFGNodes to generate DFG
	* what is in this function:
 	* Traverse all DFGEdges ,setOutEdge for the src DFGNode and setInEdeg for the dst DFGNode
 	*/
void DFG::connectDFGNodes() {
  for (DFGEdge* edge: m_DFGEdges) {
    DFGNode* left = edge->getSrc();
    DFGNode* right = edge->getDst();
    left->setOutEdge(edge);
    right->setInEdge(edge);
  }
}

void DFG::generateDot(Function &t_F) {
  error_code error;
//  sys::fs::OpenFlags F_Excl;
  string func_name = t_F.getName().str();
  string file_name = func_name + ".dot";
  StringRef fileName(file_name);
  raw_fd_ostream file(fileName, error, sys::fs::OF_None);

  file << "digraph \"DFG for'"  << t_F.getName() << "\' function\" {\n";

  //Dump InstDFG nodes.
  for (DFGNodeInst* node: m_InstNodes) {
     	file << "\tNode" << node->getID() << "[shape=record,"<<"color="<<node->color<<",label=\"" << "(" << node->getID() << ") " << node->getName();
			IFDEF(CONFIG_DFG_LEVEL,file << " level="<<node->getLevel());
		 	file	<< "\"];\n";
		}
  //Dump ConstDFG nodes.
  for (DFGNodeConst* node: m_ConstNodes) {
      file << "\tNode" << node->getID() <<  "[shape=record,"<<"color="<<node->color<<",label=\"" << "(" << node->getID() << ") " << node->getName() << "\"];\n";
		}
  //Dump ParamDFG nodes.
  for (DFGNodeParam* node: m_ParamNodes) {
      file << "\tNode" << node->getID() <<  "[shape=record,"<<"color="<<node->color<<",label=\"" << "(" << node->getID() << ") " << node->getName();
		  if(node->isloop())	file << " loop:"<<node->getloopID()<<"\"];\n";
			else	file <<"\"];\n";
		}
  // Dump data flow.
  for (DFGEdge* edge: m_DFGEdges) {
    // Distinguish data and control flows. Make ctrl flow invisible.
      file << "\tNode" << edge->getSrc()->getID() << " -> Node" << edge->getDst()->getID() << " [color="<< edge->getcolor();
			IFDEF(CONFIG_DFG_SRC_ID,file<<",label="<<edge->getsrcID());
			file <<"]" << "\n";
  }
  file << "}\n";
  file.close();

}

/** Print the information of DFG.
 * Print the name and number of occurrences of each operation that appears in the data flow graph
 * Print the number of DFGNodes and DFGLinks in the DFG
 */
void DFG::showOpcodeDistribution() {

	OUTS("==================================",ANSI_FG_CYAN); 
  OUTS("[show opcode count]",ANSI_FG_CYAN);
  map<string, int> opcodeMap;
  for (DFGNodeInst* node: m_InstNodes) {
    opcodeMap[node->getOpcodeName()] += 1;
  }
  for (map<string, int>::iterator opcodeItr=opcodeMap.begin();
      opcodeItr!=opcodeMap.end(); ++opcodeItr) {
    outs()<< (*opcodeItr).first << " : " << (*opcodeItr).second << "\n";
  }
  outs()<< "Inst DFG node count: "<<m_InstNodes.size()<<";\n";
  outs()<< "Const DFG node count: "<<m_ConstNodes.size()<<";\n";
  outs()<< "Param DFG node count: "<<m_ParamNodes.size()<<";\n";
	outs()<< "DFG edge count: "<<m_DFGEdges.size()<<";\n";
}

/**search the DFGNodeInst using the Instruction*
 */
DFGNodeInst* DFG::getInstNode(Instruction* t_inst) {
  for (DFGNodeInst* node: m_InstNodes) {
					if(node->getInst() == t_inst){
      return node;
					}
  }
  llvm_unreachable("ERROR cannot find the corresponding DFG node.");
  return NULL;
}

/**search the DFGNodeConst in m_ConstNodes using the ConstantData 
 */
DFGNodeConst* DFG::getConstNode(ConstantData* t_const) {
  for (DFGNodeConst* node: m_ConstNodes) {
					if(node->getConstant() == t_const){
      return node;
					}
  }
  return NULL;
}

/**search the DFGNodeParam in m_ParamNodes using the Argument 
 */
DFGNodeParam* DFG::getParamNode(Argument* t_param) {
  for (DFGNodeParam* node: m_ParamNodes) {
					if(node->getParam() == t_param){
      return node;
					}
  }
  return NULL;
}


/**Get the pointer of DFGEdge from t_src to t_dst DFGNode.The DFGEdge must be confirmed to have been created.You can use hasDFGEdge() to check this.
*/
/*
DFGEdge* DFG::getDFGEdge(DFGNode* t_src, DFGNode* t_dst) {
  for (DFGEdge* edge: m_DFGEdges) {
    if (edge->getSrc() == t_src and
        edge->getDst() == t_dst) {
      return edge;
    }

  }
  llvm_unreachable("ERROR cannot find the corresponding DFG edge.");
  return NULL;
}
*/


/**Check if the DFGEdge from t_src to t_dst DFGNode has be created
 * @return true main the DFGEdge from t_src to t_dst is in m_DFGEdges,has been created in the past
 */
/*
bool DFG::hasDFGEdge(DFGNode* t_src, DFGNode* t_dst) {
  for (DFGEdge* edge: m_DFGEdges) {
    if (edge->getSrc() == t_src and
        edge->getDst() == t_dst) {
      return true;
    }
  }
  return false;
}
*/

/** get inst's name return a string
 */
string DFG::changeIns2Str(Instruction* t_ins) {
  string temp_str;
  raw_string_ostream os(temp_str);
  os << *t_ins;
  return os.str();
}

int DFG::getInstNodeCount(){
	return m_InstNodes.size();
}

list<DFGNodeInst*>* DFG::getInstNodes(){
	return &m_InstNodes;
}
