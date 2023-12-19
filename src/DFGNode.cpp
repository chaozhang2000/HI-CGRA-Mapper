/**
 * @file DFGNode.cpp
 * @author Cheng Tan 
 * @brief  the implementation of DFGNode class
 * @version 0.1
 */

#include "DFGNode.h"
#include "common.h"

DFGNode::DFGNode(int t_id, string t_name){
	m_id = t_id;
	m_name = t_name;
}
int DFGNode::getID() {
  return m_id;
}
string DFGNode::getName(){
	return m_name;
}

void DFGNode::setInEdge(DFGEdge* t_dfgEdge) {
  if (find(m_inEdges.begin(), m_inEdges.end(), t_dfgEdge) ==
      m_inEdges.end())
    m_inEdges.push_back(t_dfgEdge);
}

void DFGNode::setOutEdge(DFGEdge* t_dfgEdge) {
  if (find(m_outEdges.begin(), m_outEdges.end(), t_dfgEdge) ==
      m_outEdges.end())
    m_outEdges.push_back(t_dfgEdge);
}
