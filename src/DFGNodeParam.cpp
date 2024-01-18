#include "DFGNodeParam.h"
#include "common.h"

const string DFGNodeParam::color= "red";
DFGNodeParam::DFGNodeParam(int t_id,Argument*t_param,string t_name,bool t_isloop,int t_loopID):DFGNode(t_id,t_name){
	m_param = t_param;
	m_isloop = t_isloop;
	m_loopID = t_loopID;
}

Argument* DFGNodeParam::getParam() {
  return m_param;
}

bool DFGNodeParam::isloop(){
	return m_isloop;
}

int DFGNodeParam::getloopID(){
	return m_loopID;
}
