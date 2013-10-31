#ifndef LIB123_H
#define LIB123_H
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <math.h>
#include <algorithm>
#include <string> 
#include "LSHhumming.h"
#include "time.h"
#include "STypes.h"
#include "SMelody.h"
#include "SUtil.h"
#include "SDSP.h"
#include "SModel.h"
#include "STester.h"
#include <windows.h>
#include <process.h>    /* _beginthread, _endthread */
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>



using namespace std;


typedef struct 
{ 
	char wavename[300];
	map<string , vector<float>> indexSongName;
	PRNearNeighborStructT IndexHuming;
	map<unsigned long , pair<string,short>> IndexLSH;
	map<unsigned long , pair<string,pair<short,short>>> IndexLSHNote;
	int stepFactor;
	IntT RetainNum;
	IntT LSHFilterNum;
	float stepRatio;
}ParamInfo, *pParamInfo;

_declspec(dllexport) void songread(string IDAndNameFile,map <string ,string> &songIDAndName);
_declspec(dllexport) int indexRead(ParamInfo *param,map <string ,string> &songIDAndName);
_declspec(dllexport) int WavToSongFive(char *wavename,ParamInfo *param,vector<string>& songFive);
#endif