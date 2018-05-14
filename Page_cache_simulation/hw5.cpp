#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <string>
#define endl '\n'
using namespace std;

int main() {
	FILE *P;
	P = fopen("trace.txt", "r");
	unsigned int hitCount = 0, missCount = 0;
	vector<string> data;
	string pageNum;
	list<string> frame;
	map<string, list<string>::iterator> MAP;

	for (int choose = 0; choose < 2; choose++) {
		if (choose == 0) { // FIFO
			cout << "FIFO---" << endl << "size      miss      hit            page fault ratio" << endl;
		}
		else { // LRU
			cout <<  "LRU---" << endl << "size      miss      hit            page fault ratio" << endl;
		}

		for (int frameSize = 64; frameSize <= 512; frameSize *= 2) {
			// total 8 loops, so read the file and store into data in the 1st loop
			if (choose == 0 && frameSize == 64) {
				char s[100];
				while (fgets(s, 100, P) != NULL) {
					for (int i = 3; i < 8; i++) {
						pageNum += s[i];
					}
					data.push_back(pageNum);

					auto it = MAP.find(pageNum); bool hit = (it != MAP.end());
					if (hit) {
						hitCount++;
					}
					else {
						missCount++;
						// add page to list & map
						frame.push_back(pageNum);
						auto listIt = frame.end();
						MAP[pageNum] = --listIt;
						// kick out the victim page
						if (frame.size() > frameSize) {
							string victim = frame.front();
							frame.pop_front();
							MAP.erase(victim);
						}
					}
					pageNum = "";
				}
			}
			else {
				for (int i = 0; i < data.size(); i++) {
					pageNum = data[i];
					auto it = MAP.find(pageNum); bool hit = (it != MAP.end());
					if (hit) {
						hitCount++;
						if (choose == 1) { // LRU
							// update list
							frame.erase(it->second);
							frame.push_back(pageNum);
							auto listIt = frame.end();
							// update map
							it->second = --listIt;
						}
						else {
							// FIFO, nothing to do
						}
					}
					else {
						missCount++;
						// add page to list & map
						frame.push_back(pageNum);
						auto listIt = frame.end();
						MAP[pageNum] = --listIt;
						// kick out the victim page
						if (frame.size() > frameSize) {
							string victim = frame.front();
							frame.pop_front();
							MAP.erase(victim);
						}
					}
				}
			}
			cout << left << setw(10) << frameSize << setw(10) << missCount << setw(15) << hitCount << fixed << setprecision(9) << double(missCount) / double(missCount + hitCount) << endl;
			frame.clear(); MAP.clear();
			hitCount = 0; missCount = 0;
		}
	}
	return 0;
}
