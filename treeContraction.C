// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "parallel.h"
#include "utils.h"
#include <iostream>
#include "sequence.h"
#include "speculative_for.h"
#include "treeContraction.h"
#include "get_time.h"
#include "parseCommandLine.h"
using namespace std;

struct contractStep {
  char* R;
  intT* nodes;
  internalNode* internal;
  intT n;
  contractStep(char* _R, intT* _nodes, internalNode* _internal, intT _n) 
    : R(_R), nodes(_nodes), internal(_internal), n(_n) {}

  bool reserve(intT i) {
    intT parent = nodes[i];
    intT grandparent = internal[parent].parent;
    intT right = internal[parent].rightChild;
    intT left = internal[parent].leftChild;
    if(grandparent == -1) { 
      //make sure you don't compete with other leaves anymore
      R[i] = 2;
      return 1; 
    }
    
    //check other leaves
    intT sibling = (i == right) ? left : right;
    if(sibling < i) return 1; //sibling is leaf and has higher priority

    if(sibling >= n && sibling != INT_T_MAX) {
      //sibling is internal node
      intT rightS = internal[sibling].rightChild;
      intT leftS = internal[sibling].leftChild;
      if(rightS < i || leftS < i) return 1;
    }

    intT rightP = internal[grandparent].rightChild;
    intT leftP = internal[grandparent].leftChild;
    intT uncle = (parent == rightP) ? leftP : rightP;

    if(uncle < i) return 1; //uncle is leaf, has higher priority

    if(uncle >= n && uncle != INT_T_MAX) { //uncle is internal node
      intT rightU = internal[uncle].rightChild;
      intT leftU = internal[uncle].leftChild;
      if(rightU < i || leftU < i) return 1;
    }

    R[i] = 1; //winner
    return 1;
  }

  bool commit(intT i) {
    if(R[i] == 1) {
      intT parent = nodes[i];
      intT grandparent = internal[parent].parent;
      //sibling should shortcut to grandparent
      intT right = internal[parent].rightChild;
      intT left = internal[parent].leftChild;
      intT sibling = (i == right) ? left : right;
      if(sibling < n) nodes[sibling] = grandparent; //sibling is leaf
      else internal[sibling].parent = grandparent; //sibling is internal
      if(internal[grandparent].rightChild == parent) 
	internal[grandparent].rightChild = sibling;
      else internal[grandparent].leftChild = sibling;
      return 1;
    } else if(R[i] == 2) { 
      //parent is root, don't compete with other leaves in future rounds
      intT parent = nodes[i];
      intT right = internal[parent].rightChild;
      intT left = internal[parent].leftChild;
      if(i == right) internal[parent].rightChild = INT_T_MAX;
      else internal[parent].leftChild = INT_T_MAX;
      return 1;
    }

    else return 0;
  }
};

void treeContraction(intT* nodes, internalNode* internal, intT n, intT r) {
  char* R = newArray(n, (char) 0);
  contractStep cStep(R,nodes,internal,n);
  speculative_for1(cStep, 0, n, (r != -1) ? r : 50, 0);
  free(R);
}

void timeTreeContraction(intT* nodes, internalNode* internal, intT n, int rounds, intT ratio) {
  intT* nodesCopy = newA(intT,n);
  internalNode* internalCopy = newA(internalNode,n-1);
  internalNode* internalCopyO = internalCopy - n;
  parallel_for(intT i=0;i<n;i++) nodesCopy[i] = nodes[i];
  parallel_for(intT i=0;i<n-1;i++) {
    internalCopy[i].parent = internal[n+i].parent;
    internalCopy[i].leftChild = internal[n+i].leftChild;
    internalCopy[i].rightChild = internal[n+i].rightChild;
  }
  treeContraction(nodesCopy, internalCopyO, n, ratio);
  double time=0;
  for (int i=0; i < rounds; i++) {
    parallel_for(intT i=0;i<n;i++) nodesCopy[i] = nodes[i];
    parallel_for(intT i=0;i<n-1;i++) {
      internalCopy[i].parent = internal[n+i].parent;
      internalCopy[i].leftChild = internal[n+i].leftChild;
      internalCopy[i].rightChild = internal[n+i].rightChild;
    }
    timer t; t.start();
    treeContraction(nodesCopy, internalCopyO, n, ratio);
    t.stop();
    time+=t.get_total()/double(rounds);
  }
  std::cout <<time<< endl;
  free(nodesCopy);
  free(internalCopy);
}

struct nonNegF{bool operator() (intT a) {return (a>=0);}};

int main(int argc, char* argv[]) {
  commandLine P(argc,argv,"[-r <rounds>] <n>");
  int rounds = P.getOptionIntValue("-r",1);
  intT ratio = P.getOptionIntValue("-ratio",-1);
  char* filename1 = P.getOptionValue("-nodes");
  char* filename2 = P.getOptionValue("-parents");
  char* filename3 = P.getOptionValue("-leftChildren");
  char* filename4 = P.getOptionValue("-rightChildren");
  if(0) {
   1;
  } else {
    intT n = atoi(P.getArgument(0));
    intT* nodes = newA(intT,n);
    parallel_for(intT i=0;i<n;i++) nodes[i] = -1;
    internalNode* internal = newA(internalNode,n-1);
    parallel_for(intT i=0;i<n-1;i++) internal[i].parent = -1;

    intT* IDs = newA(intT,2*n-1);
    parallel_for(intT i=0;i<2*n-1;i++) IDs[i] = i;
    intT* newIDs = newA(intT,2*n-1);

    //internalNode* internal = newA(internalNode,n-1);
    internalNode* internalO = internal - n;

    intT numIDs = 2*n-1;
    intT numnodes = n;
    intT numProcessed = 0;
    intT maxID = n;
    //generate a tree
    for(intT i=0;i<n-1;i++) {
      //pick two at random
      intT j = 1;
      intT a = utils::hash(i) % numnodes;
      while(IDs[a] == -1) { a = utils::hash(i+3*j) % numnodes; j++; }
      j = 1;
      intT b = utils::hash2(i) % numnodes;
      while(b == a || IDs[b] == -1) { b = utils::hash2(i+j) % numnodes; j++; }
      if(IDs[a] < n) nodes[IDs[a]] = maxID; else internalO[IDs[a]].parent = maxID; 
      if(IDs[b] < n) nodes[IDs[b]] = maxID; else internalO[IDs[b]].parent = maxID; 
      // nodes[IDs[a]] = maxID;
      // nodes[IDs[b]] = maxID;
      internalO[maxID].leftChild = IDs[a];
      internalO[maxID].rightChild = IDs[b];
      IDs[a] = -1;
      IDs[b] = -1;
      numnodes++; maxID++;
      numProcessed +=2;
      if(numnodes/4 < numProcessed) {
  //pack
  numIDs = sequence::filter(IDs, newIDs, numIDs, nonNegF());
  swap(IDs,newIDs);
  numnodes = numnodes - numProcessed;
  numProcessed = 0;
      }
    }
    free(IDs);
    free(newIDs);

    // for(intT i=0;i<n;i++)cout<<nodes[i]<<" ";cout<<endl;
    // for(intT i=n;i<2*n-1;i++)cout<<"("<<i<<","<<internalO[i].parent<<","<<internalO[i].leftChild<<","<<internalO[i].rightChild<<") ";cout<<endl;

    timeTreeContraction(nodes, internalO, n, rounds, ratio);
    free(nodes); free(internal);
  }
}
