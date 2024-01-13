#include "CGRA.h"
#include "common.h"
#include "cassert"

using namespace llvm;
using namespace std;
/**
 * What is in this Function:
 * 1. init some var,like m_rows、m_columns、m_FUCount and so on.
 * 2. init the CGRANode and CGRALink according to default value.
 * 3. connect the CGRANode and CGRALink to Generate the CGRA.
 */
CGRA::CGRA(int t_rows,int t_columns){
	
  m_rows = t_rows;
  m_columns = t_columns;
  m_FUCount = t_rows * t_columns;
  nodes = new CGRANode**[t_rows];

  int node_id = 0;
  for (int i=0; i<t_rows; ++i) {
  	nodes[i] = new CGRANode*[t_columns];
    for (int j=0; j<t_columns; ++j) {
    	nodes[i][j] = new CGRANode(node_id++, j, i);
    }
  }
		//这里LinkCount的数量不再是由paramCGRA中来读取，而是采用下面的方式来进行计算，可以发现，这是一种默认的链接方式。
		//这是一种这样的连接方式:每个node的上下左右都双向连接，除了最外围的一圈只往里面连接
  m_LinkCount = 2 * (t_rows * (t_columns-1) + (t_rows-1) * t_columns);
  links = new CGRALink*[m_LinkCount];

    // Connect the CGRA nodes with links.
  int link_id = 0;
  for (int i=0; i<t_rows; ++i) {
  	for (int j=0; j<t_columns; ++j) {
			//right
			if (i < t_rows - 1) {
      	links[link_id] = new CGRALink(link_id);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i+1][j]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i+1][j]);
        ++link_id;
			}
			//left
      if (i > 0) {
      	links[link_id] = new CGRALink(link_id);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i-1][j]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i-1][j]);
        ++link_id;
      }
			//down
      if (j < t_columns - 1) {
        links[link_id] = new CGRALink(link_id);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i][j+1]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i][j+1]);
        ++link_id;
      }
			//up
      if (j > 0) {
        links[link_id] = new CGRALink(link_id);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i][j-1]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i][j-1]);
        ++link_id;
			}
    }
  }
#ifdef CONFIG_CGRA_DEBUG
	//dump CGRA
  OUTS("\nCGRA DEBUG",ANSI_FG_BLUE);
	OUTS("==================================",ANSI_FG_CYAN); 
  OUTS("[CGRA Node and links count]",ANSI_FG_CYAN);
	outs()<<"CGRArows:"<<m_rows<<"\n";
	outs()<<"CGRAcolumns:"<<m_columns<<"\n";
	outs()<<"CGRANode:"<<m_FUCount<<"\n";
	outs()<<"CGRALink:"<<m_LinkCount<<"\n";

	OUTS("==================================",ANSI_FG_CYAN); 
  OUTS("[CGRA Node and links information]",ANSI_FG_CYAN);
  for (int i=0; i<m_rows; ++i) {
    for (int j=0; j<m_columns; ++j) {
			outs()<< "Node("<<nodes[i][j]->getx()<<","<<nodes[i][j]->gety()<<"); ";
			outs()<< "ID:"<<nodes[i][j]->getID()<<"; ";
			outs()<< "hasDataMem:"<<nodes[i][j]->hasDataMem()<<"\n";
    }
  }
	for (int i=0; i<m_LinkCount;i++){
		outs()<<"Link"<<links[i]->getID()<<":from Node"<<links[i]->getsrc()->getID()<<"->Node"<<links[i]->getdst()->getID()<<"\n";
	}
#endif
}

CGRALink* CGRA::getEdgefrom(CGRANode* t_src,CGRANode* t_dst){
	for(int i = 0;i<m_LinkCount;i++){
		if(links[i]->getsrc()== t_src and links[i]->getdst() == t_dst)
			return links[i];
	}
	assert("ERROR cannot find the CGRALink from srcCGRANode to dstCGRANode");
	return NULL;
}

CGRA::~CGRA(){
	for (int i=0; i<m_LinkCount;i++){
					delete links[i];
	}
  for (int i=0; i<m_rows; ++i) {
    for (int j=0; j<m_columns; ++j) {
			delete nodes[i][j];
		}
	}
	for (int i=0;i<m_rows;i++){
		delete[] nodes[i];
	}
	delete[] nodes;
	delete[] links;
}
