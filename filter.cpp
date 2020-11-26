#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include "gettime.h"
#include "sequence.h"
#include "math.h"

int main(int argc,char ** argv){
    if(argc==1 or argc>3){
        std::cout<<"Command error"<<std::endl;
        exit(1);
    }
    long long n=atoi(argv[1]);
    int total_times=1;
    if (argc>=3)
        total_times=atoi(argv[2]);

    long long * A=new long long[n];
    long long* B=new long long[n];
    int *mf=new int[n];
    cilk_for(long long i=0;i<n;i++) {
        A[i]=i;
        B[i]=0;
    }
    cilk_for(long long i=0;i<n;i++) mf[i]=0;
    auto p=[&](int x){return (x&3)!=3;};
    long long m0;
	m0=sequence::filter(A,B,n,p);
	startTime();
	m0=sequence::filter(A,B,n,p);
	nextTime();
	startTime();
	m0=sequence::filter(A,B,n,p);
	nextTime();
	startTime();
	m0=sequence::filter(A,B,n,p);
	nextTime();
	startTime();
	m0=sequence::filter(A,B,n,p);
	nextTime();
	startTime();
	m0=sequence::filter(A,B,n,p);
	nextTime();
	startTime();
	m0=sequence::filter(A,B,n,p);
	nextTime();
	
    long long m1;

    double time=0;
    cilk_for(long long j=0;j<n;j++) 
        A[j]=j;
       
    m1=sequence::in_place_filter(A,n,p,false);
    if(m0!=m1){
        std::cout<<"wrong"<<std::endl;
    }
    else{
         cilk_for(long long i=0;i<m0;i++){
         if(B[i]!=A[i]){
            std::cout<<i<<" wrong"<<std::endl;
            //break;
            }
        }
    }
    cilk_for(long long j=0;j<n;j++) 
        A[j]=j;
	startTime();
    m1=sequence::in_place_filter(A,n,p,false);
	nextTime();
    cilk_for(long long j=0;j<n;j++) 
        A[j]=j;
	startTime();
    m1=sequence::in_place_filter(A,n,p,false);
	nextTime();
    cilk_for(long long j=0;j<n;j++) 
        A[j]=j;
	startTime();
    m1=sequence::in_place_filter(A,n,p,false);
	nextTime();
    cilk_for(long long j=0;j<n;j++) 
        A[j]=j;
	startTime();
    m1=sequence::in_place_filter(A,n,p,false);
	nextTime();
    cilk_for(long long j=0;j<n;j++) 
        A[j]=j;
	startTime();
    m1=sequence::in_place_filter(A,n,p,false);
	nextTime();
    cilk_for(long long j=0;j<n;j++) 
        A[j]=j;
	startTime();
    m1=sequence::in_place_filter(A,n,p,false);
	nextTime();

    
    delete []A;
    delete []B;
    delete []mf;
    return 0;

}