#include "headers.h"
//#include <vector>

PPointT readPointHumming(FILE *fileHandle,IntT pointsDimension);
void readDataSetFromFileHumming(char *filename,IntT nPoints ,PPointT *dataSetPoints,IntT pointsDimension);
PPointT * readDataSetFromFileHumming(char *filename,IntT nPoints ,IntT pointsDimension);
IntT LSHDataQueryToResult(PPointT *dataSet,PPointT *sampleQueries,IntT dimension,Int32T nPointsData,
					 Int32T nPointsQuery,IntT nSampleQueries,float thresholdR,MemVarT memoryUpperBound,
					 IntT *IndexArray);

//初始化LSH数据结构
IntT LSHDataStruct(PPointT *dataSet,IntT dimension,Int32T nPointsData,IntT nSampleQueries,
				   float thresholdR,MemVarT memoryUpperBound,PRNearNeighborStructT &IndexHuming);

IntT LSHStructToResult(PPointT *sampleQueries,Int32T nPointsQuery, IntT IndexArraySize, 
					   IntT * &IndexArray,PRNearNeighborStructT &IndexHuming,IntT * NumArray,IntT dimension);
float dist(PPointT p1, PPointT p2, IntT dimension);
void sort(float *arr,IntT * ResultLable,int n,int SortNum);
IntT AllResultToRetainMostNearResult(IntT * RetainIndex,PPointT sampleQueries,IntT RetainNum ,
									 PPointT *result,IntT dimension,IntT sizeResult,double * RetainDis);
IntT LSHStructToResultOnePointRetainSeveral(PPointT *sampleQueries,Int32T nPointsQuery,IntT IndexArraySize, 
											IntT * &IndexArray,PRNearNeighborStructT &IndexHuming,IntT * NumArray,
											IntT RetainNum ,IntT dimension,IntT LSHFilterNum,IntT * &IndexFilterArray,
											IntT &sizeFilter,double * &IndexArrayDis);