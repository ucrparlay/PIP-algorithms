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
// changed logic of keep[] from sfor2
#include "parallel.h"
#include "utils.h"
#include "sequence.h"
//#include "gettime.h"
#include <iostream>
using namespace std;

struct reservation {
  intT r;
  reservation() : r(INT_T_MAX) {}
  void reserve(intT i) { utils::writeMin(&r, i); }
  bool reserved() { return (r < INT_T_MAX);}
  void reset() {r = INT_T_MAX;}
  bool check(intT i) { return (r == i);}
  bool checkReset(intT i) {
    if (r==i) { r = INT_T_MAX; return 1;}
    else return 0;
  }
};

inline void reserveLoc(intT& x, intT i) {utils::writeMin(&x,i);}

template <class S>
intT speculative_for(S step, intT s, intT e, int granularity,
		     bool hasState=1, int maxTries=-1) {
  if (maxTries < 0) maxTries = 100 + 200*granularity;
  intT maxRoundSize = (e-s)/granularity+1;
  intT currentRoundSize = maxRoundSize;
  intT *I = newA(intT,maxRoundSize);
  //intT *Ihold = newA(intT,maxRoundSize);
  //bool *keep = newA(bool,maxRoundSize);
  S *state;
  if (hasState) {
    state = newA(S, maxRoundSize);
    parallel_for (intT i=0; i < maxRoundSize; i++) 
      state[i] = step;
  }

  int round = 0; 
  intT numberDone = s; // number of iterations done
  intT numberKeep = 0; // number of iterations to carry to next round
  intT totalProcessed = 0; // number done including wasteds tries
  //cout << sizeof(state) << endl;
  
  //int prevdone = 0;

  while (numberDone < e) {
	//startTime();
    if (round++ > maxTries) {
      cout << "speculative_for: too many iterations, increase maxTries"<< endl;
      abort();
    }
    intT size = min(currentRoundSize, e - numberDone);
    totalProcessed += size;
	//cout << round << ' ' << numberDone << ' ' << numberDone-prevdone << ' ' << totalProcessed << ' ' << size << endl;

    if (hasState) {
      parallel_for (intT i =0; i < size; i++) {/////////////////
	if (i >= numberKeep) I[i] = numberDone + i;
	state[i].reserve(I[i], i);
      } 
    } else {
      parallel_for (intT i =0; i < size; i++) {
	if (i >= numberKeep) I[i] = numberDone + i;
	step.reserve(I[i], i);
      } 
    }
    
    if (hasState) {
      auto pred =[&](int i){return !state[i].commit(I[i], i);};
      numberKeep = sequence::in_place_filter(I, size, pred);
	
    } else {
       auto pred =[&](int i){return !step.commit(I[i], i);};
      numberKeep = sequence::in_place_filter(I,  size, pred);
    }
	
    // keep iterations that failed for next round
    
    //swap(I, Ihold);
	   //prevdone = numberDone;
    numberDone += size - numberKeep;
    //nextTimeN();
	//cout << round << ' ' << numberDone << ' ' << numberDone-prevdone << ' ' << totalProcessed << endl;

    // adjust round size based on number of failed attempts
    if (float(numberKeep)/float(size) > .2) 
      currentRoundSize = max(currentRoundSize/2, 
			     max(maxRoundSize/64 + 1, numberKeep));
    else if (float(numberKeep)/float(size) < .1) 
      currentRoundSize = min(currentRoundSize * 2, maxRoundSize);
    //cout << size << " : " << numberKeep << " : " << numberDone << endl;
  }
  free(I); //free(Ihold); //free(keep); 
  if(hasState) free(state);
  return totalProcessed;
}
