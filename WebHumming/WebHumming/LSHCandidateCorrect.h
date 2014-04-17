#ifndef LSHCANDIDATECORRECT_H
#define LSHCANDIDATECORRECT_H
#include<fstream>
#include<iostream>
#include<vector>
#include<map>
#include<cstring>
#include<string>
using namespace std;

int NLSHCandidateCorrect(char*wavename,map<unsigned long , pair<string,pair<short,short>>> &IndexLSHNote,vector<int> &IndexCandidatesStretch,vector<int> &CandidatesNumStretch);
int LSHCandidateCorrect(char*wavename,map<unsigned long , pair<string,short>> &IndexLSH,vector<int> &IndexCandidatesStretch,vector<int> &CandidatesNumStretch);
int BALSnLSHCandidateCorrect(char*wavename,vector <string> &SongNameMapToDataY);
int BALSLSHCandidateCorrect(char*wavename,vector <string> &SongNameMapToDataY);
#endif