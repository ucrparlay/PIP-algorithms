struct internalNode {
  intT parent;
  intT leftChild;
  intT rightChild;
};

void treeContraction(intT* nodes, internalNode* internal, intT n, intT r);
