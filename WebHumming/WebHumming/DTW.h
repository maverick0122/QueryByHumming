#ifndef DTW_H
#define DTW_H
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <math.h>
#include <algorithm>
#include <string> 
#include "index.h"
#include "songname.h"
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
#include "LSHVectorToFile.h"
#include "LSHCandidateCorrect.h"
#define MyMinTwo(x,y) ((x)<(y)?(x):(y))
using namespace std;


//typedef struct 
//{ 
//	char wavename[300];
//	map<string , vector<float>> indexSongName;
//	PRNearNeighborStructT IndexHuming;
//	map<unsigned long , pair<string,short>> IndexLSH;
//	map<unsigned long , pair<string,pair<short,short>>> IndexLSHNote;
//	int stepFactor;
//	IntT RetainNum;
//	IntT LSHFilterNum;
//	float stepRatio;
//}ParamInfo, *pParamInfo;

void NoZero(vector<float>  &x);	//去零

//均值
void MinusMean(vector<float> &x);	//减均值
void MinusMeanWithThd( vector<float>  &x, int thd);	//减均值操作。加入thd参数，最多计算前thd个元素的均值
float MinusMeanSmooth(vector<float> &x);	//减均值操作，减均值后>12就-12，<-12就+12，返回均值
float Mean(vector<float>::iterator Ybegin, vector<float>::iterator Yend);
float MeanLSH(vector<float> &x);	//计算均值
void MeanNoteLSH(vector<float> &x, float mean);	//减均值
void Mean8MinutesInt(vector< vector<double> > &x);
float MeanPlus( vector<float>  &x, float plus);
void Mean(vector< vector<float> > &x);	//均值化
void MeanOld(vector< vector<double> > &x);
void MeanInt(vector< vector<double> > &x);

void Dimention20LSHToToneInteger(vector<double> &x);
double DTWBeginAndRA(vector<vector<double>> &D);
double DTWBeginNewLSH(vector<vector<double>> &D);
void ZeroToForward(vector< vector<double> > &x);
void OneFileToMultiFile(string fileName,int ThreadNum);
int realPitchToToneShengda(vector <float> &queryPitch);	//音高序列转换为半音音符序列
double PitchDistance(vector<vector<double>> &a, vector<vector<double>> &b);
double StringMatchToDisMapRALSHNewPairVariance(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse,int pairNum);
double StringMatchToDisMapRALSHNew(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);//固定长度的RA算法
double StretchRAThree(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
int WavToResult8Minutes(char *wavename,map<string , vector<vector<double>>> &indexSongName );
int readIndexPitch8Minutes(string dataIndex,map<string , vector<vector<double>>> &indexSongName);
double StringMatchToDisMyRA(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);//自己的RA
void StringToStringMean(vector< vector<double> > &queryX, vector< vector<double> > &dataY,double stretch);
double StringMatchToDisAndRA(vector< vector<double> > &queryX, vector< vector<double> > &dataY);//线性伸缩返回库中对应的长度
int StringMatchToSizeY(vector< vector<double> > &queryX, vector< vector<double> > &dataY);//线性伸缩返回库中对应的长度
double StretchFiveLength(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
double DTWBeginNew(vector<vector<double>> &D);//开头开始匹配
int WavToResultTowToOne(char *wavename,map<string , vector<vector<double>>> &indexSongName );
double StretchMyRA(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
double StringMatchToDisMap(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);//RA算法
double StretchRA(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
double DTWBeginQueryAndSongSeven(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D);//递归求距离的总函数
double DTWdisRecur(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D,double disMid,double num,double bound);//递归求最短距离
void MeanTowToOne(vector< vector<double> > &x);//两个点化为一个点
int ReadToneToResultCre(char *wavename,map<string , vector<vector<double>>> &indexSongName );//倒谱法读音符求
int WavCepstrumToResult(char *wavename,map<string , vector<vector<double>>> &indexSongName );//倒谱法求
float StringMatch( vector<float>  &queryX,  vector<float>  &dataY);//vector匹配求距离
double StringMatchUltimate(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int &ultimateNum);//vector匹配求距离
void Var(vector< vector<double> > &x);
bool CompareLastName(const pair<int,string> &p1,const pair<int,string> &p2);
double LinearToDisUltimate(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int &ultimateNum);
int WavToResult8MinutesLinearAndRA (char *wavename,map<string , vector<vector<double>>> &indexSongName );
float StringMatchToDisMapRALSHNewPairVariancePositionVariance(vector< vector<float> > &queryX, vector< vector<float> > &dataY,int recurse,
										   int pairNum,double MidPercentage,int &ultimateNum);
float LinearToDis( vector<float>  &queryX,  vector<float>  &dataY);//线性伸缩算法
double StringMatchToDisQueryConst(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);
double StringMatchToDis(vector< vector<double> > &queryX, vector< vector<double> > &dataY);//线性伸缩求距离
void StringToString( vector<float>  &queryX,  vector<float>  &dataY,float stretch);	//线性伸缩
double DTWBeginQueryAndSongSevenNo(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D);//音符上下波动求，7个DTW，不用递归
int realPitchToTone(vector <double> &queryPitch);//频率到音符
int WavToResultFive(char *wavename,map<string , vector<vector<double>>> &indexSongName );
int WavToResult8MinutesLinearAndDTW(char *wavename,map<string , vector<vector<double>>> &indexSongName );
int PitchToTone(vector <vector <double>> &queryPitch);//频率对应的点到音符
double DTWbegin(vector<vector<double>> &D);//开头开始匹配

double DTWBeginRecurseLinear(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D);
int WavToResultStretchAndDTW(char *wavename,map<string , vector<vector<double>>> &indexSongName );
void Var8Minutes(vector< vector<double> > &x);
int ToneToResultStretch(char *wavename,map<string , vector<vector<double>>> &indexSongName );
double StringMatchToDisMyRANewPair(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);
double DTWtotalOrig(vector<vector<double>> &D);

double Scale(vector< vector<double> > &x);

int WavToResult8MinutesInt(char *wavename,map<string , vector<vector<double>>> &indexSongName );
double StringMatchToDisMapMidMatch(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);
double StretchThree(vector< vector<double> > &queryX, vector< vector<double> > &dataY);//线性伸缩用5个音符波动求
double StringMatchToDis(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
double DTWBeginQueryAndSongFive(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D);//DTW用5个矩阵（5次DTW求）
double DTWtotalPlusPe(vector<vector<double>> &D);
int WavToResultStretch(char *wavename,map<string , vector<vector<double>>> &indexSongName );//读wav线性伸缩求
double DTWtotal(vector<vector<double>> &D);//任意部分开始匹配
double DTW(vector<vector<double>> &D);//原始匹配
double StretchRAOne(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
int DistanceMatrix(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D);//求距离矩阵
double MyDistance(vector<double> &a, vector<double> &b);//求距离
double MyMin(double a, double b, double c);	//求最小值
void readinstance(char *wavename,vector <vector <double>> &queryPitch);//读音符
void discre(vector <vector <double>> &x);//求音符差
void Zerodiscre(vector <vector <double>> &x);//不去零求音符差
int WavToResult(char *wavename,map<string , vector<vector<double>>> &indexSongName );//从WAV中匹配
int DatawavToPitch(char *wavename);//转化库WAV到基频文件，批处理程序
int ToneTorealPitch(vector <vector <double>> &queryPitch);//音符到基频的转化
int ReadToneToResult(char *wavename,map<string , vector<vector<double>>> &indexSongName );//从文件中匹配
int WavToResult8MinutesLSH (char *wavename,map<string , vector<vector<double>>> &indexSongName,PRNearNeighborStructT &IndexHuming,
							map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,IntT RetainNum);
void ZeroToForwardThreshold(vector< vector<double> > &x , int BeginFrame);
void ZeroToForwardThresholdAndLongZero(vector< vector<double> > &x , int BeginFrame);
void ZeroToForwardThresholdAndLongZeroToHalfBefor(vector< vector<float> > &x , int BeginFrame);
int realPitchToThreeTone(vector <vector <double>> &queryPitch,vector <vector <double>> &queryPitchTow,vector <vector <double>> &queryPitchThree);
int realPitchToAnotherTowTone(vector <vector <double>> &queryPitch,vector <vector <double>> &queryPitchTow,vector <vector <double>> &queryPitchThree);
int WavToResult8MinutesThreeMatrix (char *wavename,map<string , vector<vector<double>>> &indexSongName );

vector<pair<int,int>> DuplicateSegmentBegin(vector<float> & tone);	//从音符序列tone任意位置匹配开头，若匹配长度大于100帧，记录该位置和持续帧数并返回
float LinearToDisIter( vector<float>::iterator  Xbegin,  vector<float>::iterator  Xend,
					   vector<float>::iterator  Ybegin,  vector<float>::iterator  Yend);

double RAPositionVarianceOptimal( vector<double>  &queryX,  vector<double>  &dataY,
								 int recurse,int pairNum,double MidPercentage,int &ultimateNum);

//复制
void PRNearNeighborStructTCopy(PRNearNeighborStructT des,PRNearNeighborStructT src);	//RNN数据结构复制
void FloatCopyToVector(vector <float> &des, float *src, int len);	//将float数组复制到vector中

#endif