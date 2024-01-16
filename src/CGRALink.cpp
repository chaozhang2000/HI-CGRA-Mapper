#include "CGRALink.h"

CGRALink::CGRALink(int t_linkId) {
	m_id = t_linkId;
}

void CGRALink::connect(CGRANode* t_src, CGRANode* t_dst) {
  m_src = t_src;
  m_dst = t_dst;
}

int CGRALink::getID(){
	return m_id;
}

CGRANode* CGRALink::getsrc(){
	return m_src;
}

CGRANode* CGRALink::getdst(){
	return m_dst;
}
