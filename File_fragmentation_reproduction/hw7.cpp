#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <sys/time.h>
using namespace std;
#define size 1024*64
#define file_n 1500
#define size_l 1024*1024*48

int main(){
	struct timeval start, End;
	gettimeofday(&start, 0);

	cout<<"enter path\n";
	string location;
	getline(cin, location);
	location+="/";
	char* buf= new char[size];
	char* buffer=new char[size_l];
	string name;
	stringstream s;

	for(int i=0;i<file_n;i++){
		s<<i;
		s>>name;
		ofstream outfile((location+name+".txt").c_str(), ios::out);
		outfile.write(buf, size*sizeof(char));
		outfile.close();
		s.clear();
	}
	for (int i=0;i<file_n;i+=2){
		s<<i;
		s>>name;
		remove((location+name+".txt").c_str());
		s.clear();
	}
	
	ofstream outfile((location+"largefile.txt").c_str(), ios::out);
	outfile.write(buffer, size_l*sizeof(char));
	outfile.close();
	
	system(("filefrag -v "+location+"largefile.txt").c_str());
	gettimeofday(&End, 0);
	int sec=End.tv_sec-start.tv_sec;
	int usec=End.tv_usec-start.tv_usec;
	cout<<"time: "<<sec+(usec/1000000.0)<<" sec\n";
	
	for (int i=1;i<file_n;i+=2){
		s<<i;
		s>>name;
		remove((location+name+".txt").c_str());
		s.clear();
	}
	
	delete [] buf;
	delete [] buffer;
	return 0;
}
