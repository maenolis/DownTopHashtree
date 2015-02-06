#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "components.h"


using namespace std;

int main(int argc, char* argv[]){
	
	//arguments number check
	if(argc != 5){
		cout << "You gave wrong arguments." << endl;
		cout << "Try again with 5 (4 + 'filesync')." << endl;
		return -1;
	}
	
	//arguments reading
	FILE* f1, * f2;
	int page_size, fan_out;
	f1 = fopen(argv[1],"rb");
	f2 = fopen(argv[2],"rb");
	sscanf(argv[3], "%d", &page_size);
	
	//arguments checking
	if(page_size < 1){
		cout << "Wrong page size." << endl;
		exit(EXIT_FAILURE);
	}
	sscanf(argv[4], "%d", &fan_out);
	if(fan_out < 2){
		cout << "Wrong fan out." << endl;
		exit(EXIT_FAILURE);
	}
	
	//file opening
	if(f1 == NULL){
		cout << "f1" << endl;
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	if(f2 == NULL){
		cout << "f2" << endl;
		perror("fopen");
		fclose(f1);
		exit(EXIT_FAILURE);
	}
	
	//creating trees
	Tree* t1 = new Tree(fan_out, page_size, f1);
	Tree* t2 = new Tree(fan_out, page_size, f2);

	//print trees if we want to
	//t1->printTree();
	//t2->printTree();
	
	//comparing trees
	compare(t1, t2, fan_out);
	
	//deleting trees
	delete t2;
	delete t1;
	
	//closing files
	fclose(f1);
	fclose(f2);
	
	//print statistics if we want to
	printStatistics();
	return 0;
}

