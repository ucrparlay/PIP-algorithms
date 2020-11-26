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
//original sfor, r removed, modify prev
#include "parallel.h"
#include "utils.h"
#include <iostream>
#include "sequence.h"
#include "speculative_for2_nokeep.h"
#include "listRanking.h"
#include "randPerm.C"
#include "get_time.h"
#include "parseCommandLine.h"
using namespace std;

struct listRankingStep {
  //bool* R;
  node* nodes;
  intT n;
  listRankingStep( node* _nodes, intT _n) :  nodes(_nodes), n(_n) {}

  bool reserve(intT i, intT loc) {
    intT next = nodes[i].next, prev = nodes[i].prev;
    if(i < next && i < prev) {
      
       nodes[i].next=-next-1;
       

    }; //check if local min
    return 1; }

  bool commit (intT i, intT loc) {
    if(nodes[i].next<0 ){ //local min 
      intT next = -(nodes[i].next+1);
      intT prev = nodes[i].prev; 
      
      if(next != n) nodes[next].prev = prev;
      if(prev != n) nodes[prev].next = next;
      //R[loc] = 0;
      //nodes[i].next=0;
      //nodes[i].prev=0;
      return 1; } 
    else return 0; //lost 
  }};

void listRanking(node *A, intT n, intT r = -1) {
  //bool* R = newArray(n / ((r != -1) ? r : 100) + 1, (bool) 0);
  listRankingStep lStep(A,n);
  speculative_for(lStep, 0, n, (r != -1) ? r : 100, 0);
  //free(R);
}




void timeListRanking(node* A, intT n, int rounds, intT ratio) {
  node* B = newA(node,n);
  parallel_for(intT i=0;i<n;i++) {
    B[i].prev = A[i].prev;
    B[i].next = A[i].next;
  }
  listRanking(B, n, ratio);
  double time=0;
  for (int i=0; i < rounds; i++) {
    parallel_for(intT i=0;i<n;i++) {
      B[i].prev = A[i].prev;
      B[i].next = A[i].next;
    }
    timer t; t.start();
    listRanking(B, n, ratio);
    t.stop();
    time+=t.get_total()/double(rounds);
    }
  std::cout <<time<< endl;
  free(B);
}

struct addG { 
  intT* A;
  addG(intT* _A): A(_A) {}
  intT operator() (const intT i) { return A[i] == i; }
};

int main(int argc, char* argv[]) {
  commandLine P(argc,argv,"[-r <rounds>] <n>");
  int rounds = P.getOptionIntValue("-r",1);
  intT ratio = P.getOptionIntValue("-ratio",-1);
  intT n = atoi(P.getArgument(0));
  //intT numLists = min(n,P.getOptionIntValue("-l",1));
  intT* A = newA(intT,n);
  parallel_for(intT i=0;i<n;i++) A[i] = i;
  randPerm(A, n, -1);
  std::cout<<rounds<<endl;
  bool* processed = newArray(n,(bool)0);
  
  //get rid of cycles
  for(intT i=0;i<n;i++) {
    if(!processed[i]) {
      intT j = i;
      while(A[j] != j) {
    processed[j] = 1;
    j = A[j];
    if(j == i) { A[i] = i; break; }
      }
    }
  }
  free(processed);
  intT nl = sequence::reduce<intT>((intT)0, n, utils::addF<intT>(), addG(A));
  cout<<"num lists = "<<nl<<endl;
  // intT i = 0;
  // while(nl < numLists) {
  //   if(A[i] != i) {
  //     A[i] = i; nl++;
  //   }
  //   i++;
  // }

  //for(int i=0;i<n;i++)cout<<A[i]<<" ";cout<<endl;
  node* nodes = newA(node,n);
  parallel_for(intT i=0;i<n;i++) nodes[i].next = nodes[i].prev = n;
  parallel_for(intT i=0;i<n;i++) {
    if(A[i] != i) { 
      nodes[i].next = A[i];
      nodes[A[i]].prev = i;
    }
  }
  
  //last node of list has next = n, first node of list has prev = n

  free(A);
  timeListRanking(nodes, n, rounds, ratio);
  free(nodes);
}
