#ifndef LSHVECTORTOFILE_H
#define LSHVECTORTOFILE_H

#include<fstream>
#include<iostream>
#include<vector>
#include<map>
#include<cstring>
#include<string>
using namespace std;

//将LSH点或NLSH点写入文件
int LSHVectorToFile(vector<vector<float>> &LSHVector, string filename);
//将LSH索引写入文件，后一个输出文件用于统计每个索引文件的数据数
int IndexLSHToFile(map<unsigned long , pair<string,short>> &IndexLSH, string filename, string cfilename);
//将NLSH索引写入文件，后一个输出文件用于统计每个索引文件的数据数
int IndexLSHNoteToFile(map<unsigned long , pair<string,pair<short,short>>> &IndexLSH, string filename, string cfilename);
//将LS后的LSH点写入文件
int LSHVectorLSToFile(vector<vector<vector<float>>> &LSHVectorLS, string filename);
//将LS后的LSH索引写入文件，后一个输出文件用于统计每个查询文件的数据数
int IndexLSHLSToFile(vector<vector<vector<float>>> &LSHVectorLS, string wavname, string filename, string cfilename);

#endif