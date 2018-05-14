#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h> // share memory
#include <sys/wait.h>
#include <sys/time.h>
#define endl '\n'
using namespace std;

int dim, pids[16];
void process(int i, int j,int shm_id_m,int shm_id_a,unsigned int *matrix,unsigned int *ans){
    pid_t childpid;
    childpid=fork();
    if(childpid<0){
        cout<<"error fork process!"<<endl;
        exit(1);
    }
    else if(childpid==0){ // child
        matrix=(unsigned int *)shmat(shm_id_m, NULL, 0);
        ans=(unsigned int *)shmat(shm_id_a, NULL, 0);
		int start=(dim/i)*j;
		int end;
		if(j+1==i){ // note that if not split clean
			end=dim;
		}
		else{
			end=start+(dim/i);
		}
		/*----------------matrix multiplication----------------*/
		for(int row=start;row<end;row++){
			for(int col=0;col<dim;col++){
				unsigned int tmp=0;
				for(int t=0;t<dim;t++){
					tmp+=matrix[row*dim+t]*matrix[t*dim+col];
				}
				ans[row*dim+col]=tmp;
			}
		}
		/*----------------matrix multiplication----------------*/
		shmdt(matrix);
		shmdt(ans);
        exit(0);
    }
    else if(childpid>0){ // parent
        pids[j]=childpid;
        if(j+1==i){ // when done all processes, wait all child pid
            for(int k=0;k<=j;k++){
                waitpid(pids[k],NULL,0);
                pids[k]=0;
            }
        }
    }
}
int main(){
    dim=0;
    unsigned int checksum=0;
    cout<<"Input the matrix dimension: "<<endl;
    cin>>dim;

	unsigned int *ans=NULL;
	unsigned int *matrix=NULL;
	int shm_id_m=shmget(0, dim*dim*sizeof(unsigned int), IPC_CREAT|0600);
	int shm_id_a=shmget(0, dim*dim*sizeof(unsigned int), IPC_CREAT|0600);

	matrix=(unsigned int *)shmat(shm_id_m,NULL,0 );
	ans=(unsigned int *)shmat(shm_id_a, NULL, 0);
    for(int a=0;a<dim*dim;a++){
		matrix[a]=a;
    }
    for(int i=1;i<17;i++){
		checksum=0;// reset checksum
        struct timeval start, end;
        gettimeofday(&start, 0);
		matrix=(unsigned int *)shmat(shm_id_m,NULL,0 );
		ans=(unsigned int *)shmat(shm_id_a, NULL, 0);
		/* fork i's processes */
        for(int j=0;j<i;j++){
            process(i,j,shm_id_m,shm_id_a,matrix,ans);
        }
        for(int k=0;k<dim*dim;k++){ // calculate checksum
			checksum+=ans[k];
        }
        gettimeofday(&end, 0);
        int sec = end.tv_sec-start.tv_sec;
        int usec= end.tv_usec-start.tv_usec;
        cout<<"Multiplying matrices using "<<i<<" process "<<endl;
		//cout<<"Elapsed "<<sec*1000+(usec/1000.0)<<" ms, Checksum: "<<checksum<<endl;
		cout<<"Elapsed "<<sec+(usec/1000000.0)<<" sec, Checksum: "<<checksum<<endl;
		ans=NULL;
		shmdt(matrix);
		shmdt(ans);
    }
    shmdt(matrix);
    shmdt(ans);
    shmctl(shm_id_m, IPC_RMID, NULL);
    shmctl(shm_id_a, IPC_RMID, NULL);
    return 0;
}
