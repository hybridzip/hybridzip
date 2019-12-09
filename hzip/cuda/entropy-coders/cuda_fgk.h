#include <iostream>
#include <chrono>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../other/platform.h"

namespace CUDA_FGK {

	const int INVALID = -1;
	typedef struct node_t {
		bool isZero;
		bool isRoot;
		bool isLeaf;

		struct node_t *parent;
		struct node_t *left_child;
		struct node_t *right_child;

		HZIP_SIZE_T symbol;
		unsigned long long int value;
		HZIP_SIZE_T order;
	}Node;

	typedef struct symbol_t {
		HZIP_SIZE_T symbol;
		Node* tree;
	}Symbol;

	class FGKTree {
	private:
		HZIP_SIZE_T alphabet_size;
		Symbol** symbols;
		Node** zeroNode;
		Node** root;
		bool* codebuffer;

		HZIP_CUDA_HOST_DEVICE Node* createTree();
		HZIP_CUDA_HOST_DEVICE void updateTree(Node* currNode);
		HZIP_CUDA_HOST_DEVICE Node* findReplaceNode(Node* currMax, Node* root);
		HZIP_CUDA_HOST_DEVICE void swapNodes(Node* x, Node* y);
		HZIP_CUDA_HOST_DEVICE Node* addChild(Node* parent, HZIP_SIZE_T symbol, HZIP_SIZE_T order, HZIP_SIZE_T value, bool isZero, bool isRoot);
		HZIP_CUDA_HOST_DEVICE Node* addSymbol(HZIP_SIZE_T symbol);
		HZIP_CUDA_HOST_DEVICE Node* getTreeFromSymbol(HZIP_SIZE_T symbol);
		HZIP_CUDA_HOST_DEVICE bool* codeOfNode(Node* node, HZIP_SIZE_T *n);
		HZIP_CUDA_HOST_DEVICE void reverseCode(bool *code, HZIP_SIZE_T codeSize);
	public:
		HZIP_CUDA_HOST_DEVICE FGKTree(HZIP_SIZE_T n);
		//~FGKTree();
		HZIP_CUDA_HOST_DEVICE void encode(HZIP_SIZE_T symbol, bool** code, HZIP_SIZE_T* code_length);
	};

}

//void launch_fgk_benchmark_kernel(int n);


