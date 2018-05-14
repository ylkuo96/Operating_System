#include <iostream>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <math.h>
#define unit pow(2, 20)
using namespace std;

bool _Num; // inode number
bool _Name; // file name
bool _Min; // min size
bool _Max; // max size

long long num; // inode number
string name; // file name
long double mini; // min size
long double maxx; // max size

void dir(const char *path)
{
	DIR *d;
	struct dirent *ptr;
	struct stat buf;
	// current path
	string current; 
	current.assign(path);
	if(current[current.size()-1]!='/')
	{
		current+='/';
	}
	
	if((d=opendir(path)) != NULL)
	{
		while((ptr=readdir(d)) != NULL)
		{
			bool check=true;
			if(strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
			{
				if(stat((current+ptr->d_name).c_str(), &buf) != -1)
				{
					// regular file or directory
					if(S_ISREG(buf.st_mode) || S_ISDIR(buf.st_mode))
					{
						if(_Num)
						{
							if(buf.st_ino != num)
							{
								check=false;
							}
						}
						if(_Name)
						{
							if(ptr->d_name != name)
							{
								check=false;
							}
						}
						if(_Min)
						{
							if((long double)buf.st_size/(long double)unit < mini)
							{
								check=false;
							}
						}
						if(_Max)
						{
							if((long double)buf.st_size/(long double)unit > maxx)
							{
								check=false;
							}
						}
						
						if(check)
						{
							long double Size=(long double)buf.st_size/(long double)unit;
							printf("%s%s %lu %LF MB\n", current.c_str(), ptr->d_name, buf.st_ino, Size);
						}
					}
					
					if(S_ISDIR(buf.st_mode))
					{
						const char *nxt=(current+ptr->d_name+'/').c_str();
						dir(nxt);
					}
				}
			}
		}
	}
	else
	{
		printf("error open directory %s\n", path);
	}
}

int main(int argc, char** argv)
{
	_Num=_Name=_Min=_Max=false;
	for(int i=2; i<argc; i++)
	{
		string command;
		command.assign(argv[i]);
		if(command=="-inode")
		{
			i++;
			num=atoi(argv[i]);
			_Num=true;
		}
		else if(command=="-name")
		{
			i++;
			name=argv[i];
			_Name=true;
		}	
		else if(command=="-size_min")
		{
			i++;
			mini=atof(argv[i]);
			_Min=true;
		}
		else if(command=="-size_max")
		{
			i++;
			maxx=atof(argv[i]);
			_Max=true;
		}
	}
	dir(argv[1]);
	return 0;
}
