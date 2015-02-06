#ifndef _COMPONENTS_CPP_
#define _COMPONENTS_CPP_

#include "MD5.h"

class Node{
public:
	Node(int _fan_out, unsigned int* _digest, int _pageNumber);
	Node(int _fan_out);
	~Node();
	Node** getLeaves();
	int getNoOfLeaves(int fan_out);
	void setHeight(int _height);
	int getHeight();
	int getPageNumber();
	void setLeaf(Node* _node, int fan_out);
	void printNode();
	void makeDigest(bool finalize);
	unsigned int* getDigest();
private:
	Node** leaves;
	int noOfLeaves;
	int height;
	int fan_out;
	unsigned int* digest;
	int pageNumber;
};

class Tree{
public:
	Tree(int _fan_out, int _page_size, FILE* _f);
	~Tree();
	void printTree();
	Node* getRoot();
	int getNoOfLastLevelNodes(Node* node);
	
private:
	void load();
	void finalizeDigests();
	void insert(Node* _node, int page_number);
	void makeNewRoot();
	int fan_out;
	int page_size;
	Node* root;
	FILE* f;
};

void compare(Tree* t1, Tree* t2, int fan_out);
bool compare(Node* n1, Node* n2, int fan_out);
bool compare(unsigned int* d1, unsigned int* d2);
int power(int num1, int num2);
void printStatistics();

#endif

