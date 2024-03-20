#include "DFGNodeInst.h"
#include "common.h"
#include "DFGEdge.h"
#include "config.h"
#include <iostream>

const string DFGNodeInst::color = "black";
DFGNodeInst::DFGNodeInst(int t_id,Instruction*t_inst,string t_name):DFGNode(t_id,t_name){
	m_inst = t_inst;
	m_opcodeName = t_inst->getOpcodeName();
	m_succInstNodes = NULL;
	m_predInstNodes = NULL;
	m_level = 0;
	m_haveSetLevel = false;
	m_constraintedNode = false;
	m_constraintNodeID = 0;
	m_constraintMem = false;
	m_constraintMemID = 0;
	m_isMemOpts = (m_opcodeName == "load" || m_opcodeName == "store")? true:false;
	m_pipeline = false;
	m_latency = 0;

	if(config_info.execLatency.find(m_opcodeName) !=config_info.execLatency.end()){
		m_latency = config_info.execLatency[m_opcodeName];
	}
	list<string>::iterator it;
	it = find(config_info.pipeline.begin(),config_info.pipeline.end(),m_opcodeName);
	if(it != config_info.pipeline.end()){
		m_pipeline = true;
	}
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
	return m_constraintedNode;
}
int DFGNodeInst::constraintTo(){
	return m_constraintNodeID;
}
void DFGNodeInst::setConstraint(int CGRANodeID){
	m_constraintedNode = true;
	m_constraintNodeID = CGRANodeID;
}

bool DFGNodeInst::hasMemConstraint(){
	return m_constraintMem;
}
int DFGNodeInst::constraintToMem(){
	return m_constraintMemID;
}
void DFGNodeInst::setConstraintMem(int MemID){
m_constraintMem= true;
	m_constraintMemID = MemID;
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

DFGNodeParam* DFGNodeInst::getPredParamNode(int srcID){
		for(DFGEdge* edge:*getInEdges()){
			DFGNodeParam* ParamNode = dynamic_cast<DFGNodeParam*>(edge->getSrc());
			if(edge->getsrcID()==srcID and ParamNode!=NULL){
				return ParamNode;
			}
		}
		return NULL;
}
int DFGNodeInst::getlatency(){
		return m_latency;
}
bool DFGNodeInst::ispipeline(){
		return m_pipeline;
}
