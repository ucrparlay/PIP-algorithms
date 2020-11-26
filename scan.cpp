#include <iostream>
using namespace std;

 int threshold=4096;

void myupsweep (long long *A,long long  s,long long t){
    if (s==t){
        return;
    }
    if (t-s+1<=threshold){
        for (long long i=s;i<t;i++)
            A[t]+=A[i];
        return;
    }
    long long mid=(s+t)/2;
    myupsweep(A,s,mid);
    myupsweep(A,mid+1,t);
    A[t]+=A[mid];
}

void mydownsweep(long long *A,long long s,long long t,long long p){
    if (s==t){
        A[s]=p;
        return;
    }
    if (t-s<=threshold){
        long long temp=A[s],newtemp;

        A[s]=p;

        for(long long i=s+1;i<=t;i++){
            newtemp=temp;
            temp=A[i];
            A[i]=A[i-1]+newtemp;
        }
        return;
    }
    long long mid=(s+t)/2,temp=A[mid];
    mydownsweep(A,s,mid,p);
    mydownsweep(A,mid+1,t,p+temp);
}

long long my_inplace_scan(long long *A,long long n){
    if (n<=threshold){
        long long temp=A[0],newtemp;

        A[0]=0;

        for(long long i=1;i<n;i++){
            newtemp=temp;
            temp=A[i];
            A[i]=A[i-1]+newtemp;
        }
        return A[n-1]+temp;
    }
    myupsweep(A,0,n-1);
    long long sigma=A[n-1];
    mydownsweep(A,0,n-1,0);
    return sigma;
}


int main(int argc,char ** argv){
    if(argc==1 or argc>4){
        std::cout<<"Command error"<<std::endl;
        exit(1);
    }
    long long n=atoi(argv[1]);
    long long * A=new long long[n];
    for(long long i=0;i<n;i++) {
        A[i]=i;
    }
    my_inplace_scan(A,n);

    delete []A;
    return 0;
}