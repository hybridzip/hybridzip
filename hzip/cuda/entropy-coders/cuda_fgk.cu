#include "cuda_fgk.h"

using namespace CUDA_FGK;

HZIP_CUDA_HOST_DEVICE Node* FGKTree::createTree() {
	Node* root = (Node*)malloc(sizeof(Node));
	root->isRoot = true;
	root->isLeaf = true;
	root->isZero = true;

	root->left_child = NULL;
	root->right_child = NULL;
	root->parent = NULL;

	root->symbol = INVALID;
	root->value = 0;
	root->order = alphabet_size << 1;

	return root;
}

HZIP_CUDA_HOST_DEVICE Node* FGKTree::findReplaceNode(Node* currMax, Node* root) {
	Node* result = currMax;
	if (result->value > root->value && !root->isLeaf) {
		Node* greatestLeft = findReplaceNode(result, root->left_child);
		if (greatestLeft) result = greatestLeft;

		Node* greatestRight = findReplaceNode(result, root->right_child);
		if (greatestRight) result = greatestRight;
	}
	else if (result->value == root->value && root->order > result->order) {
		result = root;
	}

	return (result != currMax) ? result : NULL;
}

HZIP_CUDA_HOST_DEVICE void FGKTree::swapNodes(Node* x, Node* y) {
	HZIP_SIZE_T temp_order = x->order;
	x->order = y->order;
	y->order = temp_order;

	if (x->parent->left_child == x) {
		x->parent->left_child = y;
	}
	else if (x->parent->right_child == x) {
		x->parent->right_child = y;
	}

	if (y->parent->left_child == y) {
		y->parent->left_child = x;
	}
	else if (y->parent->right_child == y) {
		y->parent->right_child = x;
	}

	Node* temp_parent = x->parent;
	x->parent = y->parent;
	y->parent = temp_parent;

}

HZIP_CUDA_HOST_DEVICE Node* FGKTree::addChild(Node* parent, HZIP_SIZE_T symbol, HZIP_SIZE_T order, HZIP_SIZE_T value, bool isZero, bool isRoot) {
	Node* child = (Node*)malloc(sizeof(Node));
	child->isLeaf = true;
	child->isRoot = isRoot;
	child->isZero = isZero;
	child->order = order;
	child->symbol = symbol;
	child->parent = parent;
	child->value = value;
	return child;
}

HZIP_CUDA_HOST_DEVICE Node* FGKTree::addSymbol(HZIP_SIZE_T symbol) {
	Node* previousZeroNode = *zeroNode;
	Node* rightChild = addChild(*zeroNode, symbol, previousZeroNode->order - 1, 1, false, false);
	Node* leftChild = addChild(*zeroNode, INVALID, previousZeroNode->order - 2, 0, true, false);
	previousZeroNode->isLeaf = false;
	previousZeroNode->isZero = false;
	previousZeroNode->left_child = leftChild;
	previousZeroNode->right_child = rightChild;

	symbols[symbol] = (Symbol*)malloc(sizeof(Symbol));
	symbols[symbol]->symbol = symbol;
	symbols[symbol]->tree = rightChild;


	*zeroNode = leftChild;
	return previousZeroNode;
}

HZIP_CUDA_HOST_DEVICE void FGKTree::updateTree(Node *currNode) {
	while (!currNode->isRoot) {
		Node *replaceNode = findReplaceNode(currNode, *root);

		if (replaceNode && currNode->parent != replaceNode) {
			swapNodes(currNode, replaceNode);
		}

		(currNode->value)++;
		currNode = currNode->parent;
	}

	(currNode->value)++;
}

HZIP_CUDA_HOST_DEVICE void FGKTree::reverseCode(bool *code, HZIP_SIZE_T codeSize) {
	if (code == NULL) {
		return;
	}

	bool *start = code;
	bool *end = code + (codeSize - 1);

	while (start < end) {
		int temp = *start;
		*start = *end;
		*end = temp;
		start++;
		end--;
	}
}

HZIP_CUDA_HOST_DEVICE bool* FGKTree::codeOfNode(Node *node, HZIP_SIZE_T *n) {
	Node *current = node;
	 
	int i = 0;
	while (!current->isRoot) {
		Node *parent = current->parent;
		codebuffer[i] = (parent->left_child == current) ? 0 : 1;
		current = current->parent;
		i++;
	}
	reverseCode(codebuffer, i);

	*n = i;
	return codebuffer;
}

HZIP_CUDA_HOST_DEVICE Node* FGKTree::getTreeFromSymbol(HZIP_SIZE_T symbol) {
	Symbol *symbolPtr = symbols[symbol];

	if (!symbolPtr) {
		return NULL;
	}

	return symbolPtr->tree;
}

HZIP_CUDA_HOST_DEVICE FGKTree::FGKTree(HZIP_SIZE_T n) {
	alphabet_size = n;
	root = new Node*;
	zeroNode = new Node*;
	*root = createTree();
	*zeroNode = *root;
	symbols = (Symbol**)malloc(sizeof(Symbol*) * n);
	memset(symbols, 0, n);
	codebuffer = (bool*)malloc(sizeof(int) * alphabet_size * 2);
}

void FGKTree::encode(HZIP_SIZE_T symbol, bool** code, HZIP_SIZE_T *code_length) {
	Node* node = getTreeFromSymbol(symbol);
	if (node) {
		*code = codeOfNode(node, code_length);
		updateTree(node);
	}
	else {
		*code = codeOfNode(*zeroNode, code_length);
		Node* newNode = addSymbol(symbol);
		updateTree(newNode);

	}

}


/*
__global__ void fgk_kernel() {
	CUDA_FGK::FGKTree tree(0x100);
	HZIP_SIZE_T *n = (HZIP_SIZE_T*)malloc(sizeof(HZIP_SIZE_T));
	bool **code = (bool**)malloc(sizeof(bool*));
	int i;
	for (i = 0; i < 1024; i++) {
		tree.encode(i % 2, code, n);
	}
}

void launch_fgk_benchmark_kernel(int n) {
	std::cout << "Running FGK CUDA benchmark ..." << std::endl;
	auto start = std::chrono::high_resolution_clock::now();
	fgk_kernel << <1, n >> > ();
	cudaDeviceSynchronize();
	std::cout << "Time taken: " << (float)(std::chrono::high_resolution_clock::now() - start).count() / 1000000000 << " s" << std::endl;
}
*/