#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include "get_time.h"
#include "sequence.h"
#include "parallel.h"
#include "utils.h"
#include "math.h"
#include <utility>
//using namespace std;
 int threshold=256;




int main(int argc,char ** argv){
   
    long long n=atoi(argv[1]);
    int total_times=1;
    if (argc==3)
        total_times=atoi(argv[2]);
    
    long long * A=new long long[n];
    long long* B=new long long[n];
    int *mf=new int[n];
    parallel_for(int i=0;i<n;i++) {
        A[i]=i;
        B[i]=0;
    }
    parallel_for(int i=0;i<n;i++) mf[i]=0;
    timer t0; t0.start();
    sequence::scan(A,B,n,plus<int>(),0);
    t0.stop();
    std::cout<<t0.get_total()<<std::endl;

    //for (int i=0;i<n;i++)
        //std::cout<<B[i]<<endl;
   
    delete []A;
    delete []B;
    delete []mf;
    return 0;

}