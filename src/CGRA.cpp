#include "CGRA.h"
#include "common.h"

using namespace llvm;
using namespace std;
/**
 * TODO: now the cgra is connect in a specific way,and every Node can support all opts, the conection and the opts should be defined by users.
 *
 * the default CGRA
 * |---| |---| |---| |---|
 * |	 | |	 | |	 | |	 | row3
 * |---| |---| |---| |---|
 *
 * |---| |---| |---| |---|
 * |	 | |	 | |	 | |	 | row2
 * |---| |---| |---| |---|
 *
 * |---| |---| |---| |---|
 * | 4 | |...| |	 | |	 | row1   ^ y
 * |---| |---| |---| |---|        |
 *                                |
 * |---| |---| |---| |---|        |
 * | 0 | | 1 | | 2 | | 3 | row0		|
 * |---| |---| |---| |---|        |
 * col0  col1  col2  col3					|
 * --------------------------------> x
 * What is in this Function:
 * 1. init some var,like m_rows、m_columns、m_FUCount and so on,new nodes and links.
 * 2. connect the CGRANode and CGRALink to Generate the CGRA.
 * 3. print CGRA information if needed.
 */
CGRA::CGRA(int t_rows,int t_columns){
	
 	//1. init some var,like m_rows、m_columns、m_FUCount and so on,new nodes and links.
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
  m_LinkCount = 2 * (t_rows * (t_columns-1) + (t_rows-1) * t_columns);
  links = new CGRALink*[m_LinkCount];

  //2. connect the CGRANode and CGRALink to Generate the CGRA.
  int link_id = 0;
  for (int i=0; i<t_rows; ++i) {
  	for (int j=0; j<t_columns; ++j) {
			//to N
			if (i < t_rows - 1) {
      	links[link_id] = new CGRALink(link_id,LINK_DIRECTION_TO_N);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i+1][j]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i+1][j]);
        ++link_id;
			}
			//to S
      if (i > 0) {
      	links[link_id] = new CGRALink(link_id,LINK_DIRECTION_TO_S);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i-1][j]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i-1][j]);
        ++link_id;
      }
			//to E
      if (j < t_columns - 1) {
        links[link_id] = new CGRALink(link_id,LINK_DIRECTION_TO_E);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i][j+1]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i][j+1]);
        ++link_id;
      }
			//to W
      if (j > 0) {
        links[link_id] = new CGRALink(link_id,LINK_DIRECTION_TO_W);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i][j-1]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i][j-1]);
        ++link_id;
			}
    }
  }
 	//3. print CGRA information if needed.
#ifdef CONFIG_CGRA_DEBUG
	//dump CGRA
  OUTS("\nCGRA DEBUG",ANSI_FG_BLUE);
	OUTS("==================================",ANSI_FG_CYAN); 
  OUTS("[CGRA Node and links count]",ANSI_FG_CYAN);
	outs()<<"CGRArows:"<<m_rows<<"\n";
	outs()<<"CGRAcolumns:"<<m_columns<<"\n";
	outs()<<"CGRANode:"<<m_FUCount<<"\n";
	outs()<<"CGRALink:"<<m_LinkCount<<"\n";
#ifdef CONFIG_CGRA_INSTMEM_SIZE
	outs()<<"CGRAInstMem size:"<<CONFIG_CGRA_INSTMEM_SIZE<<"\n";
#endif
#ifdef CONFIG_CGRA_CONSTMEM_SIZE
	outs()<<"CGRAConstMem size:"<<CONFIG_CGRA_CONSTMEM_SIZE<<"\n";
#endif

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
		outs()<<"Link"<<links[i]->getID()<<":from Node"<<links[i]->getsrc()->getID()<<"->Node"<<links[i]->getdst()->getID()<<" direction:"<<links[i]->getdirection()<<"\n";
	}
#endif
}

int CGRA::getNodeCount(){
	return m_FUCount;
}

int CGRA::getLinkCount(){
	return m_LinkCount;
}

int CGRA::getrows(){
	return m_rows;
}

int CGRA::getcolumns(){
	return m_columns;
}

CGRALink* CGRA::getLinkfrom(CGRANode* t_src,CGRANode* t_dst){
	for(int i = 0;i<m_LinkCount;i++){
		if(links[i]->getsrc()== t_src and links[i]->getdst() == t_dst)
			return links[i];
	}
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
