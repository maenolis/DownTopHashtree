#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "components.h"

using namespace std;

//variables for statistics printing
static int created = 0, deleted = 0, inserted = 0, differentPages = 0;

//Node constructor with digest directly from the file
Node::Node(int _fan_out, unsigned int* _digest, int _pageNumber){
	pageNumber = _pageNumber;
	fan_out = _fan_out;
	//this node will never have leaves
	leaves = NULL;
	noOfLeaves = 0;
	digest = new unsigned int[4];
	//copy digest to node's digest
	memcpy(digest, _digest, 4*sizeof(unsigned int));
	//statistics variable
	created++;
}

Node::Node(int _fan_out){
	//this is not an last level node and has no page from the file
	pageNumber = -999;
	fan_out = _fan_out;
	leaves = new Node*[fan_out];
	//nulify children
	for(int i = 0; i < fan_out; i++){
		leaves[i] = NULL;
	}
	noOfLeaves = 0;
	//for now digest is NULL
	digest = NULL;
	//statistics variable
	created++;
}

//Node destructor : recursive functionality
//delete node's children and then delete node
Node::~Node(){
	if(leaves != NULL){
		//at the end delete leaves.
		for(int i = 0; i < fan_out; i++){
			//delete every child
			delete leaves[i];
			leaves[i] = NULL;
		}
		//delete children array
		delete[] leaves;
		leaves = NULL;
	}
	if(digest != NULL){
		//delete digest
		delete digest;
		digest = NULL;
	}
	//statistics variable
	deleted++;
}

Node** Node::getLeaves(){
	return leaves;
}

int Node::getNoOfLeaves(int fan_out){
	int count = 0;
	for(int i = 0; i < fan_out; i++){
		if(leaves == NULL || leaves[i] == NULL){
			break;
		}
		count++;
	}
	return count;
}

void Node::setHeight(int _height){
	height = _height;
}

int Node::getHeight(){
	return height;
}

int Node::getPageNumber(){
	return pageNumber;
}

//Make "_node" leaf to the node that called the function
void Node::setLeaf(Node* _node, int fan_out){
	for(int i = 0; i < fan_out; i++){
		if(leaves[i] == NULL){
			leaves[i] = _node;
			leaves[i]->setHeight(height - 1);
			break;
		}
	}
	//statistics variable
	inserted++;
}

//printing a node : recursive functionality
void Node::printNode(){
	if(leaves != NULL){
		for(int i = 0; i < fan_out; i++){
			if(leaves[i] != NULL){
				//print children if any
				leaves[i]->printNode();
				continue;
			}
			break;
		}
	}
	//then print self
	if(getHeight() > 0){
		//if not last level node there is no page number to print
		cout << "this is an upperHeight Node with no page in!" << endl;
	}
	else{
		cout << "Node with pageNumber : " << pageNumber << " ." << endl;
	}
	//at last print the digest
	cout << "digest = ";
	if(digest != NULL){
		MD5Print(digest);
		cout << endl;
	}
	cout << endl;
}

//making digest for a node : recursive functionality
//if no argument makes digest only in a full node
//if argument == true makes digest for every node that has not already made digest either is full or not
void Node::makeDigest(bool finalize = false){
	//if digest is made return
	if(digest != NULL){
		return;
	}
	//if not finalizing and node is not full return
	if(!finalize && getNoOfLeaves(fan_out) != fan_out){
		return;
	}
	//MD5 context declaration and initialization
	MD5_CTX context;
	MD5Init(&context);
	digest = new unsigned int[4];
	if(leaves != NULL){
		for(int i = 0; i < getNoOfLeaves(fan_out); i++){
			//make digest for children
			leaves[i]->makeDigest(finalize);
			//then make digest for current node
			MD5Update(&context, (char*)leaves[i]->getDigest() , 4 * sizeof(int));
		}
	}
	//get the final digest
	MD5Final(digest, &context);
	/*cout << "Digest made : ";
	MD5Print(digest);
	cout << endl;
	cout << "node with page number : " << pageNumber << endl;
	
	cout << "=========" << endl;
	cout << "makeDigest() : END" << endl;*/
}

unsigned int* Node::getDigest(){
	return digest;
}
	
//Tree constructor
//loads the tree from the file and finalize digests
//so the tree is ready for the comparison
Tree::Tree(int _fan_out,int _page_size, FILE* _f){
	fan_out = _fan_out;
	page_size = _page_size;
	root = NULL;
	f = _f;
	//load tree
	load();
	//finalize tree
	finalizeDigests();
	cout << "A tree with fan-out : " << fan_out << " was just created and loaded!!" << endl;
}

Tree::~Tree(){
	cout << "A tree with fan-out : " << fan_out << " is about to be destroyed." << endl;
	if(root != NULL){
		//calling recursive Node destructor
		delete root;
	}
}

void Tree::load(){
	size_t read;
	char buffer[page_size];
	size_t buffer_size = sizeof(buffer);
	int page_number = 0;
	MD5_CTX context;
	unsigned int buffer_digest[4];
	do{
		//read file page by page
		read = fread(buffer, 1, sizeof(buffer), f);
		if(read > 0){
			//if reading was ok
			if(read < buffer_size){
				//if reached end and buffer is not filled properly
				//set rest to 0
				memset(buffer + read, 0, buffer_size - read);
				//cout << "reached end of document." << endl;
			}
			//making the digest
			MD5Init(&context);
			MD5Update(&context, buffer , read);
			MD5Final(buffer_digest, &context);
			
			//inserting in the tree
			
			//cout << "inserting page_number : " << page_number << endl;
			insert(new Node(fan_out, buffer_digest, page_number), page_number);
			/*for(int i = 0; i < (int)read; i++){
			 cout << buffer[i];
			}*/
		}
		
		
		page_number++;
	}while(read == buffer_size);
}

void Tree::finalizeDigests(){
	//if empty tree
	if(root == NULL){
		return;
	}
	//else finalize tree's nodes
	root->makeDigest(true);
}

//inserting _node in tree
void Tree::insert(Node* _node,int page_number){
	//variable checking how many pages got in tree
	//and show if digests have to be made
	bool timeToMakeDigest = false;
	//variable for math computations
	int pageNumberTemp = page_number;
	
	//if empty tree.
	if(root == NULL && page_number == 0){
		root = new Node(fan_out);
		if(root == NULL){
			cout << "New didn't allocate memory properly." << endl;
			return;
		}
		inserted++;
		root->setHeight(1);
		root->setLeaf(_node, fan_out);
		return;
	}
	
	//if fan_out^root->getHeight() < page_number
	//in otherwords if new "root" needed
	if(page_number >= power(fan_out, root->getHeight())){
		makeNewRoot();
	}
	
	//if this time have digests to be made
	if((page_number + 1) % fan_out == 0){
		timeToMakeDigest = true;
	}
	
	//else
	Node* temp = root;
	while(temp->getHeight() > 1){
		for(int i = 0; i < fan_out; i++){
			if(pageNumberTemp < (i + 1) * power(fan_out, temp->getHeight() - 1)){
				//if this node doe not exist, create one
				if(temp->getLeaves()[i] == NULL){
					//set new node as leaf to the temp
					temp->setLeaf(new Node(fan_out), fan_out);
				}
				//else go down one level and check all over again!!
				pageNumberTemp = pageNumberTemp - i*power(fan_out, temp->getHeight() - 1);
				temp = temp->getLeaves()[i];
				break;
			}
		}
	}
	//finaly set the new node to the proper leaf as leaf
	temp->setLeaf(_node, fan_out);
	
	//making digests to non last level nodes
	if(timeToMakeDigest){
		temp = root;
		bool notFound = true;
		pageNumberTemp = page_number;
		while(temp->getHeight() > 0 && notFound){
			for(int i = 0; i < fan_out; i++){
				if(pageNumberTemp < (i + 1) * power(fan_out, temp->getHeight() - 1)){
					//if this is the right node
					if(pageNumberTemp + 1 == (i + 1) * power(fan_out, temp->getHeight() - 1)){
						//make digest for this leaf
						if(i != fan_out - 1){
							temp->getLeaves()[i]->makeDigest();
						}
						//or make digest for hole the temp sub-tree
						else{
							temp->makeDigest();
						}
						notFound = false;
						break;
					}
					//else go down one level and check all over again!!
					pageNumberTemp = pageNumberTemp - i*power(fan_out, temp->getHeight() - 1);
					temp = temp->getLeaves()[i];
					break;
				}
			}
		}
	}
}

//makes a new root one level up from the last one to make new empty leaves for inserting digests
void Tree::makeNewRoot(){
	Node* temp = root;
	root = new Node(fan_out);
	root->setHeight(temp->getHeight() + 1);
	root->setLeaf(temp, fan_out);
}

//prints tree : recursive functionality
void Tree::printTree(){
	cout << "/////////////////////////////" << endl;
	cout << "START PRINT TREE" << endl;
	cout << "/////////////////////////////" << endl;
	root->printNode();
	cout << "/////////////////////////////" << endl;
	cout << "END PRINT TREE" << endl;
	cout << "/////////////////////////////" << endl;
}

Node* Tree::getRoot(){
	return root;
}

//returns number of last level nodes : recursive functionality
int Tree::getNoOfLastLevelNodes(Node* node){
	//if empty node return 0
	if(node == NULL){
		return 0;
	}
	//if last level node return 1
	if(node->getLeaves() == NULL){
		return 1;
	}
	//else return the sum from leaves!
	int count = 0;
	for(int i = 0; i < node->getNoOfLeaves(fan_out); i++){
		count = count + getNoOfLastLevelNodes(node->getLeaves()[i]);
	}
	return count;
}

//compare two trees
void compare(Tree* t1, Tree* t2, int fan_out){

	cout << endl << "/////////////////////////////" << endl;
	cout << "START COMPARE" << endl;
	cout << "/////////////////////////////" << endl << endl;
	
	//if files have not the same size.
	if(t1->getNoOfLastLevelNodes(t1->getRoot()) != t2->getNoOfLastLevelNodes(t2->getRoot())){
		cout << "Files have not the same size!!" << endl << endl;
	}
	//if trees are the same!
	//if not compare(Node* n1, Node* n2, int fan_out) will print where they were not same
	else if(compare(t1->getRoot(), t2->getRoot(), fan_out)){
		cout << "Files are exactly the same!" << endl << endl;
	}
	else{
		cout << "Files are not the same!!" << endl << endl;
	}
	cout << "/////////////////////////////" << endl;
	cout << "END COMPARE" << endl;
	cout << "/////////////////////////////" << endl << endl;
	
}

//comparing two nodes : recursive functionality
bool compare(Node* n1, Node* n2, int fan_out){
	//if nodes are empty
	if((n1 == NULL && n2 == NULL)){
		return true;
	}
	//if one only is empty
	if(n1 == NULL || n2 == NULL){
		return false;
	}
	
	//if have the same digest
	if(compare(n1->getDigest(), n2->getDigest())){
		return true;
	}
	//if nodes are at the last level print differences
	if(n1->getHeight() == 0 && n2->getHeight() == 0){
		cout << "Total pages with difference so far : " << ++differentPages << endl;
		cout << "Files are not the same at page n1->getPage() : " << n1->getPageNumber() << endl;
		cout << "Digest at n1 : ";
		MD5Print(n1->getDigest());
		cout << endl;
		cout << "The following printing is just for checking that page numbers are the same." << endl;
		cout << "Files are not the same at page n2->getPage() : " << n2->getPageNumber() << endl;
		cout << "Digest at n2 : ";
		MD5Print(n2->getDigest());
		cout << endl << endl;
		return false;
	}
	//else compare nodes from lower levels to find where are the differences
	for(int i = 0; i < n1->getNoOfLeaves(fan_out) && i < n2->getNoOfLeaves(fan_out); i++){
		compare(n1->getLeaves()[i], n2->getLeaves()[i], fan_out);
	}
	return false;
}

//compare two digests cell by cell
bool compare(unsigned int* d1, unsigned int* d2){
	for(int i = 0; i < 4; i++){
		if(d1[i] != d2[i]){
			return false;
		}
	}
	return true;
}

//return num1^num2
int power(int num1, int num2){
	if(num2 == 0){
		return 1;
	}
	int oldValue = num1;
	for(int i = 1; i < num2; i++){
		num1 = num1*oldValue;
	}
	return num1;
}

//print statistics for how many nodes created, inserted and deleted to ensure that every node was in a tree and then deleted!
void printStatistics(){
	cout << "Nodes created : " << created << " ." << endl;
	cout << "Nodes inserted : " << inserted << " ." << endl;
	cout << "Nodes deleted : " << deleted << " ." << endl;
}

