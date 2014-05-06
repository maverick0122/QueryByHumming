#include "LSHhumming.h"
//#include <map>
//using namespace std;
//
//extern map<int,vector<int>> samePoint;


char sBufferH[600000];
PPointT * readDataSetFromFileHumming(char *filename,IntT nPoints ,IntT pointsDimension)
{
	FILE *f = fopen(filename, "rt");
	FAILIF(f == NULL);
	PPointT *dataSetPoints=NULL;

	//fscanf(f, "%d %d ", &nPoints, &pointsDimension);
	//FSCANF_DOUBLE(f, &thresholdR);
	//FSCANF_DOUBLE(f, &successProbability);
	//fscanf(f, "\n");
	FAILIF(NULL == (dataSetPoints = (PPointT*)MALLOC(nPoints * sizeof(PPointT))));
	for(IntT i = 0; i < nPoints; i++){
		dataSetPoints[i] = readPointHumming(f,pointsDimension);
		dataSetPoints[i]->index = i;
		printf("指针地址：%d ",dataSetPoints[i]);
	}
	return dataSetPoints;
}


PPointT readPointHumming(FILE *fileHandle,IntT pointsDimension){
	PPointT p;
	float sqrLength = 0;
	FAILIF(NULL == (p = (PPointT)MALLOC(sizeof(PointT))));
	FAILIF(NULL == (p->coordinates = (float*)MALLOC(pointsDimension * sizeof(float))));
	for(IntT d = 0; d < pointsDimension; d++){
		FSCANF_REAL(fileHandle, &(p->coordinates[d]));
		sqrLength += SQR(p->coordinates[d]);
	}
	fscanf(fileHandle, "%[^\n]", sBufferH);
	p->index = -1;
	p->sqrLength = sqrLength;
	for(IntT d = 0; d < pointsDimension; d++){
		printf("%f ",p->coordinates[d]);
	}
	printf("\n");
	return p;
}



//PPointT * readDataSetFromVector(vector<vector<double>> &LSHVector,IntT nPoints ,IntT pointsDimension)
//{
//	char *filename;
//	FILE *f = fopen(filename, "rt");
//	FAILIF(f == NULL);
//	PPointT *dataSetPoints=NULL;
//
//	//fscanf(f, "%d %d ", &nPoints, &pointsDimension);
//	//FSCANF_DOUBLE(f, &thresholdR);
//	//FSCANF_DOUBLE(f, &successProbability);
//	//fscanf(f, "\n");
//	FAILIF(NULL == (dataSetPoints = (PPointT*)MALLOC(nPoints * sizeof(PPointT))));
//	for(IntT i = 0; i < nPoints; i++){
//		dataSetPoints[i] = readPointHumming(f,pointsDimension);
//		dataSetPoints[i]->index = i;
//		printf("指针地址：%d ",dataSetPoints[i]);
//	}
//	return dataSetPoints;
//}


int main123()
{
	char *filenameData="data.txt";
	char *filenameQuery="query.txt";
	float thresholdR=5; 
	float successProbability=0.9;
	Int32T nPointsData=10;//数据点数目
	Int32T nPointsQuery=4;//询问点数目
	IntT dimension=24; 
	PPointT *dataSet=NULL; 
	IntT nSampleQueries=4; //测试询问点数目
	PPointT *sampleQueries=NULL;
	MemVarT memoryUpperBound=1000;
	PRNearNeighborStructT IndexHuming;
	dataSet=readDataSetFromFileHumming(filenameData,nPointsData ,dimension);
	sampleQueries=readDataSetFromFileHumming(filenameQuery,nPointsQuery ,dimension);
	IndexHuming=initSelfTunedRNearNeighborWithDataSet(thresholdR, successProbability, nPointsData, dimension, dataSet, 
		nSampleQueries, sampleQueries,  memoryUpperBound);//用数据直接初始化
	printf("\n初始化成功");



	RNNParametersT HummingParameter=computeOptimalParameters(thresholdR,successProbability,
		nPointsData,dimension,dataSet,nSampleQueries, sampleQueries,memoryUpperBound);//构造最优参数
	PRNearNeighborStructT IndexHumingTowSteps=initLSH_WithDataSet(HummingParameter,nPointsData,dataSet);//用最优参数构造LSH索引



	IntT resultSize=3;
	PPointT *result=(PPointT*)MALLOC(resultSize * sizeof(PPointT));
	IntT sizeR=getRNearNeighbors(IndexHuming,sampleQueries[0],result,resultSize);
	IntT sizeR2=getRNearNeighbors(IndexHumingTowSteps,sampleQueries[0],result,resultSize);
	printf("\n查询完毕");
	return 0;

}



IntT LSHDataQueryToResult(PPointT *dataSet,PPointT *sampleQueries,IntT dimension,Int32T nPointsData,
						 Int32T nPointsQuery,IntT nSampleQueries,float thresholdR,MemVarT memoryUpperBound,
						 IntT *IndexArray)
{
	char *filenameData="data.txt";
	char *filenameQuery="query.txt";
//	float thresholdR=5; 
	float successProbability=0.9;
//	Int32T nPointsData=10;//数据点数目
//	Int32T nPointsQuery=4;//询问点数目
//	IntT dimension=24; 
	//PPointT *dataSet=NULL; 
//	IntT nSampleQueries=4; //测试询问点数目
//	MemVarT memoryUpperBound=1000;


	PRNearNeighborStructT IndexHuming;
//	dataSet=readDataSetFromFileHumming(filenameData,nPointsData ,dimension);
//	sampleQueries=readDataSetFromFileHumming(filenameQuery,nPointsQuery ,dimension);


	IndexHuming=initSelfTunedRNearNeighborWithDataSet(thresholdR, successProbability, nPointsData, dimension, dataSet, 
		nSampleQueries, dataSet,  memoryUpperBound);//用数据直接初始化
	printf("\n初始化成功");



//	RNNParametersT HummingParameter=computeOptimalParameters(thresholdR,successProbability,
//		nPointsData,dimension,dataSet,nSampleQueries, sampleQueries,memoryUpperBound);//构造最优参数
//	PRNearNeighborStructT IndexHumingTowSteps=initLSH_WithDataSet(HummingParameter,nPointsData,dataSet);//用最优参数构造LSH索引



	IntT resultSize=3;
	PPointT *result=NULL;
	int IndexArraySize=10000;
	IndexArray=(IntT *)MALLOC(IndexArraySize *sizeof(IntT));
	IntT IndexMaxSize=0;
	IntT IndexNum=0;
	IntT sizeR;
	for (int i=0;i<nPointsQuery;i++)
	{
		result=(PPointT*)MALLOC(resultSize * sizeof(PPointT));
		sizeR=getRNearNeighbors(IndexHuming,sampleQueries[i],result,resultSize);
		for (int j=0;j<sizeR;j++)
		{
			if (IndexNum<IndexArraySize)
			{
				IndexArray[IndexNum]=result[j]->index;
			}
		}
		IndexMaxSize+=sizeR;
		free(result);
	}
	printf("\n查询完毕");
	return IndexMaxSize;

}

//调用MIT的r-NN开源代码初始化LSH数据结构
//输入：dataSet：LSH点集
//dimension：每个LSH点维数
//nPointsData：LSH点集大小
//nSampleQueries：测试样本点数
//thresholdR：门限
//memoryUpperBound：内存上限
//输出：IndexHuming，RNN索引
IntT LSHDataStruct(PPointT *dataSet,IntT dimension,Int32T nPointsData,IntT nSampleQueries,
	float thresholdR,MemVarT memoryUpperBound,PRNearNeighborStructT &IndexHuming)
{
	float successProbability=0.9;

	//调用MIT的r-NN开源代码初始化LSH数据结构
	IndexHuming = initSelfTunedRNearNeighborWithDataSet(thresholdR, successProbability, nPointsData, dimension, dataSet, 
		nSampleQueries, dataSet,  memoryUpperBound);	//用数据直接初始化
	printf("\n初始化成功\n");

	return 1;
}

IntT LSHStructToResult(PPointT *sampleQueries,Int32T nPointsQuery,IntT IndexArraySize, 
	   IntT * &IndexArray,PRNearNeighborStructT &IndexHuming,IntT * NumArray ,IntT dimension)
{
	IntT resultSize=3;
	PPointT *result=NULL;
	IntT IndexMaxSize=0;
	IntT IndexNum=0;
	IntT sizeR;
	for (int i=0;i<nPointsQuery;i++)
	{
		result=(PPointT*)MALLOC(resultSize * sizeof(PPointT));
		sizeR=getRNearNeighbors(IndexHuming,sampleQueries[i],result,resultSize);
		for (int j=0;j<sizeR;j++)
		{
			if (IndexNum<IndexArraySize)
			{
				IndexArray[IndexNum]=result[j]->index;
				IndexNum++;
			}
		}
		NumArray[i]=sizeR;
		IndexMaxSize+=sizeR;
		free(result);
	}
	return IndexMaxSize;
}



//得到NLSH结果，即对每个LSH点进行RNN查询，返回RetainNum个候选
//输入：sampleQueries，NLSH点集，nPointsQuery，NLSH点数
//IndexArraySize，最大查找候选数
//IndexHuming，NLSH点的RNN索引，RetainNum，NLSH每个点仅保留的点数
//dimension，NLSH点维数，LSHFilterNum，NLSH滤波保留的点数
//输出：
//IndexArray，NumArray，每个点返回的数目
//IndexFilterArray，sizeFilter，IndexArrayDis
IntT LSHStructToResultOnePointRetainSeveral(PPointT *sampleQueries,Int32T nPointsQuery,IntT IndexArraySize, 
		IntT * &IndexArray,PRNearNeighborStructT &IndexHuming,IntT * NumArray,
		IntT RetainNum ,IntT dimension,IntT LSHFilterNum,
		IntT * &IndexFilterArray,IntT &sizeFilter,double * &IndexArrayDis)
{
	IntT IndexMaxSize = 0;
	IntT IndexNum = 0;
	IntT IndexLSHFilterNum = 0;
	IntT sizeR;

	IntT sizeRetain = LSHFilterNum > RetainNum ? LSHFilterNum: RetainNum;	//实际每个点保留点数是RetainNum（5）和LSHFilterNum（200）的最大值


    IntT * RetainIndex = (IntT *)MALLOC(sizeRetain * sizeof(IntT));
	double * RetainDis = (double *)MALLOC(sizeRetain * sizeof(double));

	//对每个LSH点进行RNN查询
	for (int i=0;i<nPointsQuery;i++)
	{
		IntT resultSize = 10;	//RNN查询结果开辟的空间
		PPointT *result = (PPointT*)MALLOC(resultSize * sizeof(PPointT));	//RNN查询结果

		//调用MIT的RNN开源代码得到RNN结果
		//输入：IndexHuming，RNN索引，sampleQueries[i]，查询的LSH点
		//输出：result，查询结果，resultSize，查询结果开辟的空间（结果数大于resultSize时，resultSize*=2，即空间不足开辟两倍空间）
		//返回值：sizeR，查询结果数
		sizeR = getRNearNeighbors(IndexHuming,sampleQueries[i],result,resultSize);

		if (sizeR<=RetainNum)	//查询结果数比需要保留的点数少
		{
			if (sizeR>0)	//有查询结果

			{//获得各候选点与查询点的距离及对应序号到RetainDis及RetainIndex中

				AllResultToRetainMostNearResult(RetainIndex,sampleQueries[i], sizeR ,
					result,dimension,sizeR,RetainDis);
			}
			for (int j=0;j<sizeR;j++)
			{
				if (IndexNum<IndexArraySize)
				{
					IndexArray[IndexNum]=result[j]->index;//得到各点候选对应结果序号
					IndexFilterArray[IndexLSHFilterNum]=result[j]->index;//得到各点候选对应结果序号
					IndexArrayDis[IndexNum]=RetainDis[j];//得到结果距离值
					IndexNum++;
					IndexLSHFilterNum++;
				}
			}
			NumArray[i]=sizeR;//得到每个点查询结果返回的数目
			IndexMaxSize+=sizeR;//得到所有点的结果数目
			free(result);
		}
		else//查询结果数比需要保留的多
		{
			IntT sizeResort= LSHFilterNum < sizeR ? LSHFilterNum: sizeR;//实际每个点保留点数是sizeR和LSHFilterNum的最小值
			AllResultToRetainMostNearResult(RetainIndex,sampleQueries[i], sizeResort ,
				result,dimension,sizeR,RetainDis);//获得各候选点与查询点的距离（由小到大排序后）及对应序号到RetainDis及RetainIndex中
			for (int j=0;j<RetainNum;j++)
			{//得到前retainNum个距离及序号结果
				if (IndexNum<IndexArraySize)
				{
					IndexArray[IndexNum]=RetainIndex[j];
					IndexArrayDis[IndexNum]=RetainDis[j];
					IndexNum++;
				}
			}
			if (RetainDis[RetainNum-1]==RetainDis[RetainNum])//若最后一个候选结果与后一个候选结果相同，则后一个仍保留
			{
				IndexArray[IndexNum]=RetainIndex[RetainNum];
				IndexArrayDis[IndexNum]=RetainDis[RetainNum];
				IndexNum++;
				NumArray[i]=RetainNum+1;//需要返回的结果数加1
				IndexMaxSize+=(RetainNum+1);//得到所有点的结果数目加1
				if (sizeR>RetainNum+1 && RetainDis[RetainNum]==RetainDis[RetainNum+1])//若后面两个均与最后一个候选距离相同，则都保留
				{
					IndexArray[IndexNum]=RetainIndex[RetainNum+1];
					IndexArrayDis[IndexNum]=RetainDis[RetainNum+1];
					IndexNum++;
					NumArray[i]=RetainNum+2;//需要返回的结果数加2
					IndexMaxSize+=1;//得到所有点的结果数目再加1
				}
			}
			else//若最后一个候选结果与后一个候选结果不同，则保留现有的
			{
				NumArray[i]=RetainNum;
				IndexMaxSize+=RetainNum;
			}
			
			if (sizeR<=LSHFilterNum)//若查询结果小于滤波保留的点数，则过滤索引为查询结果序号
			{
				for (int j=0;j<sizeR;j++)
				{
					if (IndexNum<IndexArraySize)
					{
						IndexFilterArray[IndexLSHFilterNum]=result[j]->index;
						IndexLSHFilterNum++;
					}
				}
			}
			else//若查询结果大于滤波保留的点数，则过滤索引为最后保留的结果序号
			{
				for (int j=0;j<LSHFilterNum;j++)
				{
					if (IndexLSHFilterNum<IndexArraySize)
					{
						IndexFilterArray[IndexLSHFilterNum]=RetainIndex[j];
						IndexLSHFilterNum++;
					}
				}

			}
			free(result);
		}
	}
	free(RetainIndex);
	free(RetainDis);
	sizeFilter=IndexLSHFilterNum;//所有查询出的候选结果数目

	return IndexMaxSize;//返回所有点的查询保留结果数目
}

//获得各候选点的距离排序后及对应序号到RetainIndex及RetainDis中
IntT AllResultToRetainMostNearResult(IntT * RetainIndex,PPointT sampleQueries,IntT RetainNum ,
									 PPointT *result,IntT dimension,IntT sizeResult,double * RetainDis)//返回最近的RetainNum个点
{
	float * ResultDis=(float * )MALLOC(sizeResult * sizeof(float));
	IntT * ResultLable=(IntT * )MALLOC(sizeResult * sizeof(IntT));
	for (int i=0;i<sizeResult;i++)
	{
		ResultDis[i]=dist(sampleQueries,result[i],dimension);//返回查询集与各候选集的距离
		//ResultDis[i]=result[i]->sqrLength;
		ResultLable[i]=result[i]->index;//各候选对应序号
	}
	sort(ResultDis, ResultLable,sizeResult,RetainNum);//将各候选距离由小到大排序
	for (int i=0;i<RetainNum;i++)
	{
		RetainIndex[i]=ResultLable[i];
		RetainDis[i]=ResultDis[i];
	}
	free(ResultDis);
	free(ResultLable);

	return 0;
}

float dist(PPointT p1, PPointT p2, IntT dimension){ //返回两个点的距离
	float result = 0;

	for (int i = 0; i < dimension; i++){
		float temp = p1->coordinates[i] - p2->coordinates[i];
#ifdef USE_L1_DISTANCE
		result += ABS(temp);
#else
		result += SQR(temp);
#endif
	}
#ifdef USE_L1_DISTANCE
	return result;
#else
	return SQRT(result);
#endif
}


void sort(float *arr,IntT * ResultLable,int n ,int SortNum)//由小到大排序
{
	float temp;
	IntT tempLable;
	int i,j,k;
	for (i = 0; i < n - 1 && i<SortNum; i++)
	{
		k = i;
		for (j = i + 1; j < n; j++)
		{
			if (arr[k]>arr[j] )
			{
				k = j;
			}
		}

		if(k != i)
		{
			temp = arr[i];
			arr[i]= arr[k];
			arr[k] = temp;
			tempLable = ResultLable[i];
			ResultLable[i]= ResultLable[k];
			ResultLable[k] = tempLable;
		}
	}
}
