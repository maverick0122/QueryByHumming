#ifndef LSHVECTORTOFILE_H
#define LSHVECTORTOFILE_H

#include<fstream>
#include<iostream>
#include<vector>
#include<map>
#include<cstring>
#include<string>
using namespace std;

//将LSH点写入文件
int LSHVectorToFile(vector<vector<float>> LSHVector, string filename);
//将LSH索引写入文件
int IndexLSHToFile(map<unsigned long , pair<string,short>> IndexLSH, string filename);
//将NLSH索引写入文件
int IndexLSHNoteToFile(map<unsigned long , pair<string,pair<short,short>>> &IndexLSH, string filename);

#endif