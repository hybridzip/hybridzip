#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "../../other/platform.h"
#include "../utils/common.h"

namespace hfc {

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

	class fgk_tree {
	private:
		HZIP_SIZE_T alphabet_size;
		Symbol** symbols;
		Node** zeroNode;
		Node** root;
		bool* codebuffer;

		Node* createTree();
		void updateTree(Node* currNode);
		Node* findReplaceNode(Node* currMax, Node* root);
		void swapNodes(Node* x, Node* y);
		Node* addChild(Node* parent, HZIP_SIZE_T symbol, HZIP_SIZE_T order, HZIP_SIZE_T value, bool isZero, bool isRoot);
		Node* addSymbol(HZIP_SIZE_T symbol);
		Node* getTreeFromSymbol(HZIP_SIZE_T symbol);
		bool* codeOfNode(Node* node, HZIP_SIZE_T *n);
		void reverseCode(bool *code, HZIP_SIZE_T codeSize);
	public:
		fgk_tree(HZIP_SIZE_T n);
		fgk_tree();
		//~fgk_tree();
		void encode(HZIP_SIZE_T symbol, bool** code, HZIP_SIZE_T* code_length);
		bin_t encode(HZIP_SIZE_T symbol);
	};

}