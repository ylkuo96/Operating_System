#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <sys/time.h>
#include <list>
#define endl '\n'
using namespace std;

struct Pair{
    int head;
    int tail;
};

int *N = NULL;
int *M = NULL;
Pair edge[16]; // only use for 1~15
sem_t output[9]; // after doing sorting! use for 1~8
list<int> jobList;
sem_t job;
int count=0;

void JOB(int id){
	if(id>=1&&id<=7){ // do partition
		count++;
		int h=edge[id].head, t=edge[id].tail; // get from global variables
		int pivot=h, i, j;
		if(t>h){
			i=h; j=t+1;
			while(1){
				while(i+1<t+1 && M[++i]<M[h]);
				while(j-1>h-1 && M[--j]>M[h]);
				if(i>=j) break;
                // swap i & j
                int tmp=M[j];
                M[j]=M[i];
                M[i]=tmp;
			}
            // swap h & j			
			int tmp=M[j];
            M[j]=M[h];
            M[h]=tmp;
		}		
        // change edge array of the next two p_thread
        edge[id*2].head=h; edge[id*2].tail=j;
        edge[id*2+1].head=j+1; edge[id*2+1].tail=t;
		// -----
		sem_wait(&job); // lock
		jobList.push_back(id*2);
		jobList.push_back(id*2+1);
		sem_post(&job); // unlock
		// -----
	}
	else if(id>=8&&id<=15){ // do sorting
		count++;  
		int h=edge[id].head, t=edge[id].tail+1; // get bound from global edge array
        // bubble sort
        for(int i=h;i<t;i++){
            for(int j=i+1;j<t;j++){
                if(M[j]<M[i]){
                    int tmp=M[j];
                    M[j]=M[i];
                    M[i]=tmp;
                }
            }
        }
        sem_post(&output[id-7]);		
	}
}

void* func(void* param){
	int jobID;
	while(count<15){
		/*
		cout<<"joblist: ";
		for (list<int>::iterator it=jobList.begin(); it != jobList.end(); ++it)
			cout<<*it<<" ";
		cout<<endl;
		*/
		sem_wait(&job);
		if(!jobList.empty()){
			jobID=jobList.front();
			jobList.pop_front();
		}
		else{
			jobID=-1;
		}
		sem_post(&job);
		
		if(jobID!=-1){
			JOB(jobID);
		}		
	}
    pthread_exit(NULL);
}

int main(){
	struct timeval start, End;
	int n;
	fstream in;
	in.open("input.txt", ios::in);
	in>>n;
	N=new int[n];
	for(int i=0;i<n;i++){
		in>>N[i];
	}
	in.close();
	
	// thread pool size from 1 to 8
	for(int size=1; size<=8; size++){
		// reset
		count=0;
		jobList.clear();
		
		M=new int[n];
		for(int i=0;i<n;i++){
			M[i]=N[i];
		}
		for(int i=1;i<=8;i++){
			sem_init(&output[i], 0, 0);
		}
		sem_init(&job, 0, 1); // mutex
		
		pthread_t tid[16];
		edge[1].head=0;
		edge[1].tail=n-1;
		
		jobList.push_back(1); // at first, the thread will only do partition
		gettimeofday(&start, 0);
		for(int i=1;i<=size;i++){
			pthread_create(&tid[i], NULL, func, NULL);
		}
		// wait for sorting done
		for(int i=1;i<=8;i++){
			sem_wait(&output[i]);
		}	
		gettimeofday(&End, 0);
		
		// then can output
		fstream out;
		char outputName[]="output_n.txt";
		outputName[7]=size+'0';
		out.open(outputName, ios::out);
		for(int i=0;i<n;i++){
			out<<M[i]<<" ";
		}
		out<<endl;		
		out.close();
		delete [] M;

		int sec = End.tv_sec-start.tv_sec;
		int usec= End.tv_usec-start.tv_usec;
		
		char sorting[]="1-thread sorting: ";
		sorting[0]=size+'0';
		cout<<sorting<<sec+(usec/1000000.0)<<" sec"<<endl;
		//cout<<"Count: "<<count<<endl;
	}
	delete [] N;
	return 0;
}
