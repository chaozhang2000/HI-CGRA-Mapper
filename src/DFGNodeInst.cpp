#include "DFGNodeInst.h"
#include "common.h"
#include "DFGEdge.h"

const string DFGNodeInst::color = "black";
DFGNodeInst::DFGNodeInst(int t_id,Instruction*t_inst,string t_name):DFGNode(t_id,t_name){
	m_inst = t_inst;
	m_opcodeName = t_inst->getOpcodeName();
	m_succInstNodes = NULL;
	m_predInstNodes = NULL;
	m_level = 0;
	m_haveSetLevel = false;
}

Instruction* DFGNodeInst::getInst() {
  return m_inst;
}

string DFGNodeInst::getOpcodeName() {
  return m_opcodeName;
}

list<DFGNodeInst*>* DFGNodeInst::getSuccInstNodes(){
	if(m_succInstNodes==NULL){
		m_succInstNodes = new list<DFGNodeInst*>;

		for(DFGEdge* edge:*getOutEdges()){
			if(DFGNodeInst* InstNode = dynamic_cast<DFGNodeInst*>(edge->getDst())){
				m_succInstNodes->push_back(InstNode);
			}
		}
	}
	return m_succInstNodes;
}

list<DFGNodeInst*>* DFGNodeInst::getPredInstNodes(){
	if(m_predInstNodes==NULL){
		m_predInstNodes = new list<DFGNodeInst*>;

		for(DFGEdge* edge:*getInEdges()){
			if(DFGNodeInst* InstNode = dynamic_cast<DFGNodeInst*>(edge->getSrc())){
				m_predInstNodes->push_back(InstNode);
			}
		}
	}
	return m_predInstNodes;
}

void DFGNodeInst::setLevel(int t_level){
	m_level = t_level;
	m_haveSetLevel = true;
}

DFGNodeInst::~DFGNodeInst(){
	if(m_succInstNodes!=NULL){
		delete m_succInstNodes;
	}
}
