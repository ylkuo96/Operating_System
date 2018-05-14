#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <sys/time.h>
#define endl '\n'
using namespace std;

struct Pair{
    int head;
    int tail;
};

struct timeval start, End;
int n;

int* M = NULL; // for multi-thread
Pair edge[16]; // only use for 1~15
sem_t sid[15]; // use for 1~14
sem_t output[9]; // after doing sorting! use for 1~8
void* func_m(void *param);

// int* M is also used for single thread 
void bubble_sort(int h, int t);
void* func_s(void *param);
sem_t test;

void writefile(int i){
	fstream out;
    if(i==1){ // MT
		out.open("output1.txt",ios::out);	
	}
	else if(i==2){ // ST
		out.open("output2.txt",ios::out);
	}
	
    for(int i=0;i<n;i++){
        out<<M[i]<<" ";
    }
    out<<endl;
    out.close();
}

int main(){
    string input;
	cout << "Enter the filename: ";
	getline(cin, input);
	fstream in;
	in.open(input.c_str(),ios::in);

	in >> n;
	M = new int[n];
	for (int i = 0; i < n; i++) {
		in >> M[i];
	}
	in.close();

	/* ----------------------- multi-thread ----------------------- */
    for(int i=1;i<15;i++){
        sem_init(&sid[i], 0, 0);
    }
    for(int i=1;i<9;i++){
        sem_init(&output[i], 0, 0);
    }
    pthread_t tid[16]; // thread identifier, use for 1~15
    edge[1].head=0;
    edge[1].tail=n-1;

    for(int i=1;i<16;i++){
        pthread_create(&tid[i], NULL, func_m, (void*)(intptr_t)i);
    }
    for(int i=1;i<9;i++){
        sem_wait(&output[i]);
    }
    gettimeofday(&End, 0);
    int sec = End.tv_sec-start.tv_sec;
	int	usec= End.tv_usec-start.tv_usec;
    cout<<"Multi-thread sorting: "<<sec+(usec/1000000.0)<<" sec"<<endl;
	
	int c=1;
	writefile(c);
	
	delete [] M;
	/* ----------------------- multi-thread ----------------------- */
	
	/* ----------------------- single-thread ----------------------- */
	// read file again
	gettimeofday(&start, 0);
	in.open(input.c_str(),ios::in);	
	in >> n;
	M = new int[n];
	for (int i = 0; i < n; i++) {
		in >> M[i];
	}
	in.close();
	
	sem_init(&test, 0, 0);
	pthread_t single;
	pthread_create(&single, NULL, func_s, (void*)(intptr_t)n);
	sem_wait(&test);

	c=2;
	writefile(c);
	
    gettimeofday(&End, 0);
    sec = End.tv_sec-start.tv_sec;
    usec= End.tv_usec-start.tv_usec;
    cout<<"Single-thread sorting: "<<sec+(usec/1000000.0)<<" sec"<<endl;	
	
	delete [] M;
	/* ----------------------- single-thread ----------------------- */

    return 0;
}

void* func_m(void* param){ // for multi-thread
    int id=*((int*)&param);
	if(id==1){
		gettimeofday(&start, 0);	
	}
    if(id>=1 && id<=7){ // T1~T7 for quick-sort partition
        if(id>1){
            sem_wait(&sid[id-1]);
        }
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
		/* ---
		int h=edge[id].head, t=edge[id].tail;
		int i=h-1;
		for(int j=h;j<t;j++){
			if(M[j]<M[t]){ // pivot is M[t]
				i++;
				// swap i & j
				int tmp=M[i];
				M[i]=M[j];
				M[j]=tmp;
			}
		}
		// swap i+1 & t
		int tmp=M[t];
		M[t]=M[i+1];
		M[i+1]=tmp;

		int pivot=i+1;
		edge[id*2].head=h;
		edge[id*2].tail=pivot;
		edge[id*2+1].head=pivot+1;
		edge[id*2+1].tail=t;
		--- */
        sem_post(&sid[2*id-1]);
        sem_post(&sid[2*id]);
    }
    else if(id>=8 && id<=15){ // T8~T15 for bubble sort
        sem_wait(&sid[id-1]);
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
    pthread_exit(NULL);
}

void bubble_sort(int h, int t){
    for(int i=h;i<t+1;i++){
        for(int j=i+1;j<t+1;j++){
            if(M[j]<M[i]){
                int tmp=M[j];
                M[j]=M[i];
                M[i]=tmp;
            }
        }
    }
}

void* func_s(void* param){ // for single-thread
    int n=*((int*)&param);
	/*
	h                                              t     
	h1                  t1h2                       t2
	h3      t3h4        t4h5         t5h6          t6
	h7 t7h8 t8h9 t9h10 t10h11 t11h12 t12h13 t13h14 t14
	*/	
	int h=0, t=n-1; // get from global variables
	int pivot=h, i, j;
	if(t>h){
		i=h; j=t+1;
		while(1){
			while(i+1<t+1 && M[++i]<M[h]);
			while(j-1>h-1 && M[--j]>M[h]);
			if(i>=j) break;
			// swap M[i] and M[j]
			int tmp=M[j];
            M[j]=M[i];
            M[i]=tmp;
		}
        // swap(M[h],M[j]);			
		int tmp=M[j];
        M[j]=M[h];
        M[h]=tmp;
	}
    int h1=h, t1=j; int h2=j+1, t2=t;

	pivot=h1;
	if(t1>h1){
		i=h1; j=t1+1;
		while(1){
			while(i+1<t1+1 && M[++i]<M[h1]);
			while(j-1>h1-1 && M[--j]>M[h1]);
			if(i>=j) break;
			// swap M[i] and M[j]
			int tmp=M[j];
            M[j]=M[i];
            M[i]=tmp;
		}
        // swap(M[h],M[j]);			
		int tmp=M[j];
        M[j]=M[h1];
        M[h1]=tmp;
	}
    int h3=h1, t3=j; int h4=j+1, t4=t1;
	
	pivot=h3;
	if(t3>h3){
		i=h3; j=t3+1;
		while(1){
			while(i+1<t3+1 && M[++i]<M[h3]);
			while(j-1>h3-1 && M[--j]>M[h3]);
			if(i>=j) break;
			// swap M[i] and M[j]
			int tmp=M[j];
            M[j]=M[i];
            M[i]=tmp;
		}
        // swap(M[h],M[j]);			
		int tmp=M[j];
        M[j]=M[h3];
        M[h3]=tmp;
	}
    int h7=h3, t7=j; int h8=j+1, t8=t3;

	pivot=h4;
	if(t4>h4){
		i=h4; j=t4+1;
		while(1){
			while(i+1<t4+1 && M[++i]<M[h4]);
			while(j-1>h4-1 && M[--j]>M[h4]);
			if(i>=j) break;
			// swap M[i] and M[j]
			int tmp=M[j];
            M[j]=M[i];
            M[i]=tmp;
		}
        // swap(M[h],M[j]);			
		int tmp=M[j];
        M[j]=M[h4];
        M[h4]=tmp;
	}
    int h9=h4, t9=j; int h10=j+1, t10=t4;
	
	pivot=h2;
	if(t2>h2){
		i=h2; j=t2+1;
		while(1){
			while(i+1<t2+1 && M[++i]<M[h2]);
			while(j-1>h2-1 && M[--j]>M[h2]);
			if(i>=j) break;
			// swap M[i] and M[j]
			int tmp=M[j];
            M[j]=M[i];
            M[i]=tmp;
		}
        // swap(M[h],M[j]);			
		int tmp=M[j];
        M[j]=M[h2];
        M[h2]=tmp;
	}
    int h5=h2, t5=j; int h6=j+1, t6=t2;

	pivot=h5;
	if(t5>h5){
		i=h5; j=t5+1;
		while(1){
			while(i+1<t5+1 && M[++i]<M[h5]);
			while(j-1>h5-1 && M[--j]>M[h5]);
			if(i>=j) break;
			// swap M[i] and M[j]
			int tmp=M[j];
            M[j]=M[i];
            M[i]=tmp;
		}
        // swap(M[h],M[j]);			
		int tmp=M[j];
        M[j]=M[h5];
        M[h5]=tmp;
	}
    int h11=h5, t11=j; int h12=j+1, t12=t5;
	
	pivot=h6;
	if(t6>h6){
		i=h6; j=t6+1;
		while(1){
			while(i+1<t6+1 && M[++i]<M[h6]);
			while(j-1>h6-1 && M[--j]>M[h6]);
			if(i>=j) break;
			// swap M[i] and M[j]
			int tmp=M[j];
            M[j]=M[i];
            M[i]=tmp;
		}
        // swap(M[h],M[j]);			
		int tmp=M[j];
        M[j]=M[h6];
        M[h6]=tmp;
	}
    int h13=h6, t13=j; int h14=j+1, t14=t6;
	
	bubble_sort(h7, t7);
	bubble_sort(h8, t8);
	bubble_sort(h9, t9);
	bubble_sort(h10, t10);
	bubble_sort(h11, t11);
	bubble_sort(h12, t12);
	bubble_sort(h13, t13);
	bubble_sort(h14, t14);

	//bubble_sort(0,n-1);
	sem_post(&test);
	pthread_exit(NULL);
}