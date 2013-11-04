#ifndef INDEX_H
#define INDEX_H
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>
#include <algorithm>
#include <string> 
#include "pitch.h"
#include "DTW.h"
#include "define.h"
#include "LSHhumming.h"
using namespace std;

PPointT readPointVector(vector<float> &VectorTone);	//读入LSH点到PPoint中，统计平方和
int LSHFilter(char *wavename,map<string ,int> &LSHFilterMap,vector<IntT> &CandidatesFilter,
			  map<unsigned long , pair<string,short>> &IndexLSH);
void VectorSmoothToHalf(vector <float> &queryPitch);	//两帧化为一帧

//从LSH向量索引中读数据集，读入dataSet中，读入每个LSH点时记录序号和LSH点的平方和
PPointT * readDataSetFromVector(vector<vector<float>> &LSHVector);

//读入pv文件，将二维的（音符，持续时间）序列转化为一维序列
int readIndexPitch8MinutesNewPv(string dataIndex,map<string , vector<float>> &indexSongName);

void smooth(vector <float> &queryPitch);	//用五点中值法平滑，窗长为5，窗移为1
float MiddleFive(float a, float b, float c,float d,float e);	//五点中值

int charToVector(char * wavename,vector <float> &queryPitch);//读基频char到vector转换
int readIndexPitch(string dataIndex,map<string , vector<vector<double>>> &indexSongName);//读索引基频，每一行是一个基频
int readindex(string dataIndex,map<string , vector<vector<double>>> &indexSongName);//读索引基频，勿认为的基频文件

//从一维音高序列抽取LSH点
int IndexPitchToLSHVector(map<string , vector<float>> &indexSongName, 
	int StepFactor, int LSHsize, int LSHshift, int maxFrame, 
	vector<vector<float>> &LSHVector, map<unsigned long , pair<string,short>> &IndexLSH);

//从一维音高序列抽取NLSH点
int IndexPitchToLSHVectorNote(map<string , vector<float>> &indexSongName, 
	int noteMaxFrame, int NLSHsize, int maxFrame, 
	vector<vector<float>> &LSHVector, map<unsigned long , pair<string,pair<short,short>>> &IndexLSH);

int QueryPitchToLSHVectorLinearStretchingShortToMore(vector <float> &queryPitch,vector <vector<vector<float>>> &LSHQueryVectorLinearStretching,
													 float FloorLevel, float UpperLimit,int stepFactor,float stepRatio,float StretchStep,int recur);

int LSHresult(char *wavename,int sizeQuery,int stepFactor,float stretch,vector<vector<IntT>> &IndexCandidatesStretch,
			  map<unsigned long ,pair<string,short>> &IndexLSH,float StretchStep);

int IndexSignToQueryAndDataVectorHummingBeginQueryLengthFixedAndNoClearMatchLeast(vector <float> &DisCandidates,vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
																				  map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,vector <float> queryPitch,
																				   vector<float>  &queryX, vector< vector<float>  >&dataY,
																				  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
																				  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
																				  vector<float>  &CandidatesDataYDis);
int IndexSignToQueryAndDataVectorHummingMatchLeastALL(vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
													  map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,vector <float> queryPitch,
													  vector<float>  &queryX,vector<  vector<float>  >&dataY,
													  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
													  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
													  map <string , vector<pair<short, double>> > &SongMapPositionAll,int offsetbegin, int offsetLength);

int QueryPitchToLSHVectorLinearStretchingShortToMoreNote(vector<pair<short, short>>& posPairvector, vector <float> &queryPitch,vector <vector <vector<float>>> &LSHQueryVectorLinearStretching,
														 float FloorLevel, float UpperLimit,int stepFactor,float stepRatio,float StretchStep);
int IndexSignToQueryAndDataVectorHummingMatchLeastALLNote(vector<pair<short, short>>& posPair,vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
														  map<unsigned long , pair<string,pair<short,short>>> &IndexLSH,int stepFactor,vector <float> queryPitch,
														  vector<float>  &queryX, vector< vector<float> > &dataY,
														  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
														  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
														  map <string , vector<pair<short, double>> >  &SongMapPositionAll,int offsetbegin, int offsetLength);
int LSHresultRate(char *wavename,int sizeQuery,int stepFactor,float stretch,vector<vector<IntT>> &IndexCandidatesStretch,
				  map<unsigned long , pair<string,short>> &IndexLSH,map<unsigned long , pair<string,pair<short,short>>> &IndexLSHNote,
				  float StretchStep,vector<vector<float>> &IndexCandidatesDis);

//抽取一维音高序列的第一个NLSH点
int QueryPitchToLSHVectorLinearStretchingShortToMoreNoteFirst(vector<pair<short, short>>& posPairvector, 
	vector <float> &queryPitch,vector <vector <vector<float>>> &LSHQueryVectorLinearStretching,
	int noteMinFrame,int noteMaxFrame,int NLSHsize);

#endif