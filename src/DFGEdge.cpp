
#include "DFGEdge.h"
#include "common.h"


DFGEdge::DFGEdge(int t_id,int t_srcID, DFGNode* t_src, DFGNode* t_dst) {
  m_id = t_id;
  m_src = t_src;
  m_dst = t_dst;
	m_srcID = t_srcID;
	m_color = "black";
}

int DFGEdge::getID() {
  return m_id;
}

int DFGEdge::getsrcID() {
  return m_srcID;
}

DFGNode* DFGEdge::getSrc() {
  return m_src;
}

DFGNode* DFGEdge::getDst() {
  return m_dst;
}

void DFGEdge::connect(DFGNode* t_src, DFGNode* t_dst) {
  m_src = t_src;
  m_dst = t_dst;
}
void DFGEdge::setcolor(string t_color) {
				m_color = t_color;
}
string DFGEdge::getcolor(){
	return m_color;
}
