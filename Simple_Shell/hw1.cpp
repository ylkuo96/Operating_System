#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // exec() family
#include <sys/types.h>
#include <sys/wait.h>
#include <sstream>
#include "fcntl.h" // dup2

#define endl '\n'
using namespace std;

bool run_in_bg;
char *program_argv[1000];

char *input;
char *output;
bool in, out;

void fun(int signal){
    pid_t pid;
    int status;
    while(waitpid(-1,&status,WNOHANG)>0);
}

int process(char *a,char *b[]){
    pid_t childpid;
    childpid=fork();

    if(childpid<0){
        cout<<"error fork process"<<endl;
        exit(1);
    }
    else if(childpid==0){ // child process
		// input redirection '<'
		if (in){
			int fd0 = open(input, O_RDONLY);
			dup2(fd0, STDIN_FILENO); // STDIN_FILENO : 0
			close(fd0); // restore stdin
		}
		// output redirection '>'
		else if (out){
			int fd1 = creat(output , 0644) ;
			dup2(fd1, STDOUT_FILENO); // STDOUT_FILENO : 1
			close(fd1); // restore stdout
		}
        execvp(a,b);
        exit(0);
    }
    else if(childpid>0) { // parent process
        if(!run_in_bg){
            wait(NULL);
        }
        else{ // command with &, so parent process don't wait child process
            cout<<"child: "<<childpid<<" is running in background "<<endl;
            signal(SIGCHLD,fun);
        }
    }
}

int main(){
    while(true){
        string command; // program arg1 arg2 arg3,...
        string command_splt[1000]; // [program] [arg1] [arg2] [arg3],...
        stringstream ss;
        string tmp;
        int count=0; // number of program arg1 arg2 ,...
        run_in_bg=false;

        cout<<"> ";
        getline(cin,command);
        int l=command.length();

        // ex: sleep 10s& or sleep 10s &
        if(command[l-1]=='&'){ // &
            run_in_bg=true;
            command[l-1]='\0';
            if(command[l-2]==' '){
                command[l-2]='\0';
            }
        }
        ss<<command;
        while(ss>>tmp){
            command_splt[count]=tmp;
            count++;
        }
		// exit the shell
        if(command_splt[0]=="exit"){
            return 0;
        }
        // string convert to char* because of the function: execvp()
        for(int i=0;i<count;i++){
            program_argv[i]=const_cast<char*>(command_splt[i].c_str());
        }
        program_argv[count]=NULL; // put NULL at the end of the argv char* list

		// I/O redirection
		in=false, out=false;
		for(int i=0;i<l;i++){
			if(command[i]=='<') in=true;
			else if(command[i]=='>') out=true;
		}
		for(int i=0;i<count;i++){
			if(*program_argv[i]=='<'){
				char *tmp=program_argv[i+1];
				//cout<<*tmp<<endl;
				input=tmp; // point to the second instr.
				break;
			}
		}
		for(int i=0;i<count;i++){
			if(*program_argv[i]=='>'){
				char *tmp=program_argv[i+1];
				//cout<<*tmp<<endl;
				output=tmp; // point to the second instr.
				break;
			}
		}

		int out_cnt=0;
        bool flag_i=false;
		bool flag_o=false;
		if(in){
			for(int i=0;i<count;i++){
				if(command_splt[i]=="<"){
					flag_i = true;
					break;
				}
				else if(flag_i==false) out_cnt++;
			}
			char *revised[out_cnt+1]; // put NULL in the end
			for(int i=0;i<out_cnt;i++){
				revised[i]=const_cast<char*>(command_splt[i].c_str());
			}
			revised[out_cnt]=NULL;
			// process
			process(program_argv[0],revised);
		}
		else if(out){
			for(int i=0;i<count;i++){
				if(command_splt[i]==">"){
					flag_o = true;
					break;
				}
				else if(flag_o==false) out_cnt++;
			}
			char *revised[out_cnt+1]; // put NULL in the end
			for(int i=0;i<out_cnt;i++){
				revised[i]=const_cast<char*>(command_splt[i].c_str());
			}
			revised[out_cnt]=NULL;
			// process
			process(program_argv[0],revised);
		}
		else{
			// process
			process(program_argv[0],program_argv);
		}

        // reset program and program_argv
        for(int i=0;i<count;i++){
            program_argv[i]=NULL;
        }
    }
    return 0;
}
