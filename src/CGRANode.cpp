#include "CGRANode.h"
#include "CGRALink.h"

#define COMMON_OPTS(f)\
				f(mul) f(add) f(getelementptr)
#define LOAD_STORE_OPTS(f)\
				f(load) f(store)
#define M_SUPPORTOPTS_INSERT(k) m_supportOpts.insert(#k);
CGRANode::CGRANode(int t_id, int t_x, int t_y) {
  m_id = t_id;
  m_disabled = false;
	m_hasDataMem = true;
  m_x = t_x;
  m_y = t_y;
	COMMON_OPTS(M_SUPPORTOPTS_INSERT);
	if(m_hasDataMem){
		LOAD_STORE_OPTS(M_SUPPORTOPTS_INSERT);
	}
	m_neighbors = NULL;
}

int CGRANode::getID(){
	return m_id;
}
void CGRANode::attachInLink(CGRALink* t_link) {
  m_inLinks.push_back(t_link);
}

void CGRANode::attachOutLink(CGRALink* t_link) {
  m_outLinks.push_back(t_link);
}

bool CGRANode::canSupport(string t_optsname){
	return m_supportOpts.find(t_optsname)!=m_supportOpts.end();
}

list<CGRANode*>* CGRANode::getNeighbors(){
	if(m_neighbors == NULL){
		m_neighbors = new list<CGRANode*>;
		for(CGRALink* link : m_outLinks){
			m_neighbors->push_back(link->getdst());
		}
	}
	return m_neighbors;
}

CGRALink* CGRANode::getOutLinkto(CGRANode* t_node){
	for(CGRALink* link:m_outLinks){
		if(link->getdst() == t_node)
			return link;
	}
	return NULL;
}

CGRANode::~CGRANode(){
	if(m_neighbors != NULL){
		delete m_neighbors;
	}	
}
