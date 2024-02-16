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
	m_constrainted = false;
	m_constraintTo = 0;
	m_isMemOpts = (m_opcodeName == "load" || m_opcodeName == "store")? true:false;
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

int DFGNodeInst::getLevel(){
	return m_level;
}

bool DFGNodeInst::haveSetLevel(){
	return m_haveSetLevel;
}

DFGNodeInst::~DFGNodeInst(){
	if(m_succInstNodes!=NULL){
		delete m_succInstNodes;
	}
}
bool DFGNodeInst::isMemOpts(){
	return m_isMemOpts;
}
bool DFGNodeInst::hasConstraint(){
	return m_constrainted;
}
int DFGNodeInst::constraintTo(){
	return m_constraintTo;
}
void DFGNodeInst::setConstraint(int CGRANodeID){
	m_constrainted = true;
	m_constraintTo = CGRANodeID;
}

DFGNodeConst* DFGNodeInst::getPredConstNode(int srcID){
		for(DFGEdge* edge:*getInEdges()){
			DFGNodeConst* ConstNode = dynamic_cast<DFGNodeConst*>(edge->getSrc());
			if(edge->getsrcID()==srcID and ConstNode!=NULL){
				return ConstNode;
			}
		}
		return NULL;
}
