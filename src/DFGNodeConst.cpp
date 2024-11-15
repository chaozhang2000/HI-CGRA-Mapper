#include "DFGNodeConst.h"
#include "common.h"

const string DFGNodeConst::color= "blue";

DFGNodeConst::DFGNodeConst(int t_id,ConstantData*t_const,string t_name):DFGNode(t_id,2,t_name){
	m_const = t_const;
}

ConstantData* DFGNodeConst::getConstant() {
  return m_const;
}
