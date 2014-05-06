#include "DTW.h"
#include "emd.h"
#include "lib.h"

//DBN查询结果文件，每行一个查询结果，为候选序号和对应距离，按距离从小到大排序
ifstream queryResultFile;
//查询信息文件，因为这个三个文件为追加打开，重写前需删除
string query_vector_fname = "QueryLSHLSVector.txt";
string query_index_fname = "QueryLSHLSIndex.txt";
string query_counter_fname = "QueryLSHLSCounter.txt";

const int inf=100000;
const int Dpenalty=0.0;
const int emdLength=23000; //匹配5.5秒长度的EMD，哼唱音高序列的帧数上限
const double penalty=0;
const double disConst=4.0;
const double disConstString=4.0;//RA的固定常数  原始是16
const double disConstLSHRA=9.0;
map<int,vector<int>> samePoint;
map<string,int> ListSong;
map<string,vector <float>> ResultDis1;
map<string,vector <string>> ResultLable1;
map<string,vector <float>> ResultDis2;
map<string,vector <string>> ResultLable2;
map<string,vector <float>> ResultDis3;
map<string,vector <string>> ResultLable3;
CRITICAL_SECTION g_cs;


float distemd(feature_t *F1, feature_t *F2)
{
	return abs( *F1 - *F2); 
}

double MyMin(double a, double b, double c)
{
	double min;
	if (a<b)
	{
		min=a;
	}
	else
	{
		min=b;
	}
	if (c<min)
	{
		min=c;
	}
	return min;
}

bool sortRule(const pair<float,int>& s1, const pair<float,int>& s2) 
{
  return s1.first < s2.first;
}

double MyDistance(double &a, double &b)
{
	double D=0;
	D+=abs(a-b);

	return D;
}

double PitchDistance(vector<vector<double>> &a, vector<vector<double>> &b)
{
	int i,j,m,n,k;
	m = MyMinTwo(a.size(),b.size());
	if (m>0)
	{
		n=a[0].size();
	}
	else
		n=0;
	double D=0;
	double disPitch=0;
	for (i=0;i<m;i++)
	{
		disPitch=0;
		for (j=0;j<n;j++)
		{
			disPitch+=MyMinTwo(pow(a[i][j]-b[i][j],2),disConstLSHRA);
		}
		D+=disPitch;
	}
	return D;
}


//去除数组中的零值
void NoZero( vector<float>  &x)
{
	vector <float>::iterator iter=x.begin();
	for (;iter!=x.end();)
	{
		if (*iter==0)
			iter=x.erase(iter);
		else
			iter++;
	}
}


void ZeroToForwardThreshold(vector< vector<double> > &x , int BeginFrame)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//判决什么时候开始唱

	vector <vector <double>>::iterator iter=x.begin();
	for (i=0;i<BeginFrame;i++)
	{
		x.erase(iter);
		iter=x.begin();
	}

	m=x.size();
	if	(m>0)
	{
		if (x[0][0] !=0 )
		{
			Threshold=1;
		}

	}
	for (i=0;i<m-6 && Threshold==0;i++)
	{
		
		x[i].erase(x[i].begin());
		if (x[i+1][0] !=0)
		{
			Threshold=1;
		}
	}

	iter=x.begin();
	for (;iter!=x.end();)
	{
		if (iter->empty())
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;
	}
	m=x.size();

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				;
			}
			else
			{
				x[i][j]=x[i-1][j];
			}
		}
	}
}


void ZeroToForwardThresholdAndLongZero(vector< vector<double> > &x , int BeginFrame)	//去除中间以及结尾的静音部分
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//判决什么时候开始唱

	vector <vector <double>>::iterator iter=x.begin();
	for (i=0;i<BeginFrame;i++)
	{

		x.erase(iter);
		iter=x.begin();
	}
	m=x.size();
	if	(m>0)
	{
		if (x[0][0] !=0 )
		{
			Threshold=1;
		}

	}
	for (i=0;i<m-6 && Threshold==0;i++)
	{

		x[i].erase(x[i].begin());
		if (x[i+1][0] !=0)
		{
			Threshold=1;
		}
	}
	iter=x.begin();
	for (;iter!=x.end();)
	{
		if (iter->empty())
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;

	}
	m=x.size();
	int ZeroSequenceNum=0;
	int LastZero=-10;//判断上一次零的序号
	int ZeroBegin=-10;
	bool ExistSilence=FALSE;
	bool NoSilence=FALSE;

	while (!NoSilence)//去除中间以及结尾的静音部分
	{
		m=x.size();
		ExistSilence=FALSE;
		for (i=0;i<m-1 && ExistSilence==FALSE;i++)
		{
			if (i==m-2)
			{
				NoSilence=TRUE;
			}
			if (x[i][0] ==0 /*|| x[i+1][0] ==0*/)
			{
				if (LastZero+1==i)//如果上一次是零
				{
					LastZero=i;
					ZeroSequenceNum++;
				}
				else//如果上次不是零，则更改序号，并且ZeroSequenceNum归零
				{
					LastZero=i;
					ZeroBegin=i;//从这里开始是静音
					ZeroSequenceNum=0;
				}
			}
			if (ZeroSequenceNum>25)
			{
				
				ExistSilence=TRUE;
				for (j=ZeroBegin;j<m-1 && x[j][0] ==0 && j<ZeroBegin+ZeroSequenceNum-15;j++)//只删除15个静音
				{

					x[j].erase(x[j].begin());
				}
				LastZero=-10;
				ZeroBegin=-10;
				ZeroSequenceNum=0;
			}
			if (ExistSilence==TRUE)
			{
				iter=x.begin();
				for (;iter!=x.end();)
				{
					if (iter->empty())
					{
						x.erase(iter);
						iter=x.begin();
					}
					else
						iter++;

				}
			}

		}
	}
	
	m=x.size();

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				;
			}
			else
			{
				x[i][j]=x[i-1][j];
			}
		}
	}
}




void ZeroToForwardThresholdAndLongZeroToHalfBefor( vector<float>  &x , int BeginFrame)//对于中间以及结尾部分零按比例变为前面后面音符
{
	float mean=0;
	int m=x.size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//判决什么时候开始唱

	vector <float>::iterator iter=x.begin();
	for (i=0;i<BeginFrame;i++)
	{

		x.erase(iter);
		iter=x.begin();
	}
	m=x.size();
	if	(m>0)
	{
		if (x[0] !=0 )
		{
			Threshold=1;
		}

	}
	for (i=0;i<m-6 && Threshold==0;i++)
	{
		if (x[i+1] !=0/* && x[i+2][0] !=0*/ )
		{
			Threshold=1;
		}
	}
	iter=x.begin();
	for (;iter!=x.end();)
	{
		if (*iter==0)
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;

	}
	m=x.size();
	int ZeroSequenceNum=0;
	int LastZero=-10;//判断上一次零的序号
	int ZeroBegin=-10;
	bool ExistSilence=FALSE;
	bool NoSilence=FALSE;

	while (!NoSilence)//去除中间以及结尾的静音部分
	{
		m=x.size();
		ExistSilence=FALSE;
		for (i=0;i<m-1 && ExistSilence==FALSE;i++)
		{
			if (i==m-2)
			{
				NoSilence=TRUE;
			}
			if (x[i] ==0 /*|| x[i+1][0] ==0*/)
			{
				if (LastZero+1==i)//如果上一次是零
				{
					LastZero=i;
					ZeroSequenceNum++;
				}
				else//如果上次不是零，则更改序号，并且ZeroSequenceNum归零
				{
					LastZero=i;
					ZeroBegin=i;//从这里开始是静音
					ZeroSequenceNum=0;
				}
			}
			if (ZeroSequenceNum>25)//连续25帧为零的情况
			{

				ExistSilence=TRUE;
				//for (j=ZeroBegin;j<m-1 && (x[j][0] ==0 || x[j+1][0] ==0);j++)
				for (j=ZeroBegin;j<m-1 && x[j] ==0 && j<ZeroBegin+ZeroSequenceNum-10;j++)//只删除10个静音
				{

					x[j]=0;
				}
				LastZero=-10;
				ZeroBegin=-10;
				ZeroSequenceNum=0;
			}
			if (ExistSilence==TRUE)
			{
				iter=x.begin();
				for (;iter!=x.end();)
				{
					if (*iter==0)
					{
						x.erase(iter);
						iter=x.begin();
					}
					else
						iter++;
				}
			}
		}
	}
	ZeroSequenceNum=0;
	LastZero=-10;//判断上一次零的序号
	ZeroBegin=-10;
    ExistSilence=FALSE;
	NoSilence=FALSE;
	bool ZeroExist=FALSE;
	while (!NoSilence)//静音部分按比例化为前面帧或者后面帧
	{
		m=x.size();
		ExistSilence=FALSE;
		for (i=0;i<m-1 && ExistSilence==FALSE;i++)
		{
			if (i==m-2)
			{
				NoSilence=TRUE;
			}
			if (x[i] ==0 /*|| x[i+1][0] ==0*/)
			{
				if (LastZero+1==i)//如果上一次是零
				{
					LastZero=i;
					ZeroSequenceNum++;
				}
				else//如果上次不是零，则更改序号，并且ZeroSequenceNum归零
				{
					LastZero=i;
					ZeroBegin=i;//从这里开始是静音
					ZeroSequenceNum=0;//新的静音开始
				}
			}
			if (x[i] ==0 && x[i+1] !=0)
			{
				if (i>ZeroSequenceNum)
				{
					for (int k=i-ZeroSequenceNum;k<=i-ZeroSequenceNum/2;k++)
					{
						x[k]=x[k-1];
					}
					for (int k=i;k>i-ZeroSequenceNum/2;k--)
					{
						x[k]=x[k+1];
					}
				}
				ExistSilence=TRUE;
			}

		}
	}

	m=x.size();
	ZeroSequenceNum=0;
	LastZero=-10;//判断上一次零的序号
	ZeroBegin=-10;
	ExistSilence=FALSE;
	NoSilence=FALSE;

	m=x.size();

	for (i=1;i<m;i++)
	{
	
		if(x[i]!=0)
		{			
			;
		}
		else
		{
			x[i]=x[i-1];
		}

	}
}

void ZeroToForward(vector< vector<double> > &x )
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//判决什么时候开始唱

	for (i=0;i<m-6 && Threshold==0;i++)
	{
		
		x[i].erase(x[i].begin());
		int numZero=0;
		for (j=i;j<i+12 && j<m-6;j++)
		{
			if (x[j+1][0] ==0)
			{
				numZero++;
			}
		}

		if (x[i+1][0] !=0 && x[i+2][0] !=0)
		{
			Threshold=1;
		}
	}
	vector <vector <double>>::iterator iter=x.begin();
	for (;iter!=x.end();)
	{
		if (iter->empty())
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;

	}
	m=x.size();

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				;
			}
			else
			{
				x[i][j]=x[i-1][j];
			}
		}
	}
}



void MeanTowToOne(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/m;
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				;
			}
		}
	}  //均值部分
	for (i=0;i<n;i++)
	{
		for (j=1;j<m;j++,j++)
		{
			x[j][i]=(x[j][i]+x[j-1][i])/2;
			x[j-1][i]=0;
		}
	}
}


double Scale(vector< vector<double> > &x)
{
	int i,j,a0=0,a1=0,a2=0,a3=0,a4=0,a5=0,a6=0,a7=0,a8=0,a9=0;
	int m=x.size();
	int n=x[0].size();
	for (i=0;i<n;i++)
	{
		for (j=4;j<m-4;j++)
		{
			if (x[j][i]-int(x[j][i])>0.9)
			{
				a9++;
			}
			if (x[j][i]-int(x[j][i])>0.8)
			{
				a8++;
			}
			if (x[j][i]-int(x[j][i])>0.7 && x[j][i]-int(x[j][i])<0.9)
			{
				a7++;
			}
			if (x[j][i]-int(x[j][i])>0.6 && x[j][i]-int(x[j][i])<0.8)
			{
				a6++;
			}
			if (x[j][i]-int(x[j][i])>0.5 && x[j][i]-int(x[j][i])<0.7)
			{
				a5++;
			}
			if (x[j][i]-int(x[j][i])>0.4 && x[j][i]-int(x[j][i])<0.6)
			{
				a4++;
			}
			if (x[j][i]-int(x[j][i])>0.3 && x[j][i]-int(x[j][i])<0.5)
			{
				a3++;
			}
			if (x[j][i]-int(x[j][i])>0.2 && x[j][i]-int(x[j][i])<0.4)
			{
				a2++;
			}
			if (x[j][i]-int(x[j][i])>0.1 && x[j][i]-int(x[j][i])<0.3)
			{
				a1++;
			}
			if (x[j][i]-int(x[j][i])<0.2)
			{
				a0++;
			}

		}
	}
	vector <int>dis;
	dis.push_back(a0);
	dis.push_back(a1);
	dis.push_back(a2);
	dis.push_back(a3);
	dis.push_back(a4);
	dis.push_back(a5);
	dis.push_back(a6);
	dis.push_back(a7);
	dis.push_back(a8);
	dis.push_back(a9);
	vector <int> ::iterator iter;

	iter=max_element(dis.begin(),dis.end());
	int pos=iter-dis.begin();
	double scale_my=(pos+1)*0.1;
	return scale_my;

}

void MeanInt(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int Nozero=0;
	int vec=0;
	int i,j;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=8;j<m-8;j++)
		{
			mean+=x[j][i]/(m-16);
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				if (x[i][j]>10)
				{
					x[i][j]=x[i][j]-12;
				}
				if (x[i][j]<-10)
				{
					x[i][j]=x[i][j]+12;
				}
			}
		}
	}  //均值部分
	double scale=Scale(x);
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=scale;
				if (x[i][j]-(int)x[i][j]>0.7 || x[i][j]-(int)x[i][j]<0.3)
				{
					x[i][j]=int(x[i][j]+0.5);
				}
				
				cout<<i<<","<<x[i][j]<<" ";
			}
		}
	}  //均值部分
}


float MeanPlus( vector<float>  &x, float plus)
{
	float mean=0;
	int m=x.size();
	int Nozero=0;
	int i,j;
	mean=0;
	for (j=0;j<m;j++)
	{
		mean+=x[j]/m;
	}
	mean+=plus;
	for (i=0;i<m;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
			if (x[i]>12)
			{
				x[i]=x[i]-12;
			}
			if (x[i]<-12)
			{
				x[i]=x[i]+12;
			}
		}
	}  //均值部分
	return mean;
}

//减均值操作，减均值后>12就-12，<-12就+12
float MinusMeanSmooth(vector<float> &x)
{
	float mean=0;
	int m=x.size();
	int i,j;

	for (j=0;j<m;j++)
	{
	    mean+=x[j]/m;
	}
	for (i=0;i<m;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
			if (x[i]>12)
			{
				x[i]=x[i]-12;
			}
			if (x[i]<-12)
			{
				x[i]=x[i]+12;
			}
		}
	}  //均值部分
	return mean;
}


float Mean( vector<float>::iterator  Ybegin,  vector<float>::iterator  Yend)
{
	float mean=0;
	int m=Yend-Ybegin;
	int Nozero=0;
	int i,j;
	mean=0;
	for (j=0;j<m;j++)
	{
		mean+=*(Ybegin+j)/m;
	}
	for (i=0;i<m;i++)
	{
		if(*(Ybegin+i)!=0)
		{			
			*(Ybegin+i)-=mean;
			if (*(Ybegin+i)>12)
			{
				*(Ybegin+i)=*(Ybegin+i)-12;
			}
			if (*(Ybegin+i)<-12)
			{
				*(Ybegin+i)=*(Ybegin+i)+12;
			}
		}
	}  //均值部分
	return mean;
}



void Mean( float  *x ,int length)
{
	double mean=0;
	int Nozero=0;
	int i,j;
	mean=0;
	for (j=0;j<length;j++)
	{
		mean+=x[j]/length;
	}
	for (i=0;i<length;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
			if (x[i]>12)
			{
				x[i]=x[i]-12;
			}
			if (x[i]<-12)
			{
				x[i]=x[i]+12;
			}
		}
	}  //均值部分
}


void MeanFirst(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=0;
	if (m>0)
	{
		n=x[0].size();
	}
	int Nozero=0;
	int vec=0;
	int i,j;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		mean=0;
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/m;
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				if (x[i][j]>10)
				{
					x[i][j]=x[i][j]-12;
				}
				if (x[i][j]<-10)
				{
					x[i][j]=x[i][j]+12;
				}
			}
		}
	}  //均值部分
}

void MeanBefor8AndAfter8(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int Nozero=0;
	int vec=0;
	int i,j;

	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=8;j<m-8;j++)
		{
			mean+=x[j][i]/(m-16);
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				cout<<i<<","<<x[i][j]<<" ";
				//outf<<i<<","<<x[i][j]<<" ";
				if (x[i][j]>10)
				{
					x[i][j]=x[i][j]-12;
				}
				if (x[i][j]<-10)
				{
					x[i][j]=x[i][j]+12;
				}
			}
		}
	}  //均值部分

}

//减均值
void MinusMean(vector<float> &x)
{
	float mean=0;
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		mean+=x[i]/m;
	}
	for (int i=0;i<m;i++)
	{
		x[i]-=mean;
	}
}

void Dimention20LSHToToneInteger(vector<double> &x)
{
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		x[i]=(int)(x[i]+0.5);
	}

}

void MeanOld(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int Nozero=0;
	int vec=0;
	int i,j;
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				vec=1;
			}
		}
		if (vec==1)
		{
			Nozero++;
		}
		vec=0;
	}
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/Nozero;
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				cout<<i<<","<<x[i][j]<<" ";
			}
		}
	}  //均值部分
}


void MeanVar(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int i,j;
	double variance=0;
	vector<double> XVariance;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/m;
		}
		XMean.push_back(mean);
		mean=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				;
			}
		}
	}  //均值部分
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance)/m;
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]/=XVariance[j];
				;
			}
		}
	}  //方差部分
}


void Var(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int i,j;
	double variance=0;
	vector<double> XVariance;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance/m);
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{	
				if (XVariance[j]>8)
				{
				}
				x[i][j]/=2;
				;
			}
		}
	}  //方差部分
}

//减均值操作。最多计算前thd帧的均值
void MinusMeanWithThd(vector<float>  &x, int thd)
{
	double mean=0;
	int m=x.size();
	int i,j;
	int min = MyMinTwo(m,thd);

	for (j=0;j<min;j++)
	{
		mean+=x[j]/min;
	}
	for (i=0;i<m;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
		}
	}
}

void Mean8MinutesInt(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int min=MyMinTwo(m,251);
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			mean+=x[j][i]/min;
		}
		mean=int(mean+0.5);
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
			}
		}
	}  //均值部分
}

void Var8Minutes(vector< vector<double> > &x)
{
	double variance=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int min=MyMinTwo(m,251);
	vector<double> XVariance;
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance/min);
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]/=XVariance[j];
				;
			}
		}
	}  //方差部分
}

void Mean8MinutesVar(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	double variance=0;
	vector<double> XVariance;
	int min=MyMinTwo(m,251);
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			mean+=x[j][i]/min;
		}
		XMean.push_back(mean);
		mean=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
			}
		}
	}  //均值部分
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance)/min;
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]/=XVariance[j];
			}
		}
	}  //方差部分
}

int DistanceMatrix(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D)
{
	int i,j,m,n;
	m=queryX.size();
	n=dataY.size();
	for (i=0;i<m;i++)
	{
		for (j=0;j<n && j<m*3;j++)
		{
			//D[i][j]=MyDistance(queryX[i],dataY[j]);
		}
	}
	return 0;
}

double DTW(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	D[1][1]+=D[0][0];
	D[2][1]+=D[0][0]+penalty;
	for (j=2;j<n-(m-1)/2;j++)
	{
		D[1][j]+=min(D[0][j-1],D[0][j-2]+penalty);
	}
	for (j=2;j<n-(m-2)/2;j++)
	{
		D[2][j]+=MyMin(D[0][j-1]+penalty,D[1][j-1],D[1][j-2]+penalty);

	}
	for (i=3;i<m;i++)
	{
		for (j=(i+1)/2;j<n-(m-i)/2;j++)
		{
			D[i][j]+=MyMin(D[i-2][j-1]+penalty,D[i-1][j-1],D[i-1][j-2]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[i-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}
double DTWtotalOrig(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	for (i=1;i<m;i++)
	{
		for (j=1;j<n;j++)
		{
			D[i][j]+=MyMin(D[i][j-1]+penalty,D[i-1][j-1],D[i-1][j]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}

double DTWtotalFive(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n;j++)
		{
			D[i][j]+=MyMin(MyMin(D[i][j-1]+penalty,D[i-1][j-1],D[i-1][j]+penalty),D[i-1][j-2]+penalty,D[i-2][j-1]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}

double DTWtotal(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=2;j<m;j++)
	{
		D[j][0]=inf;
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n;j++)
		{
			D[i][j]+=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}


double DTWBeginQueryAndSongSevenNo(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D)
{
	double distanceM1;
	double distanceM2;
	double distanceM3;
	double distanceM4;
	double distanceM5;
	double distanceMin;
	double distanceM6;
	double distanceM7;
	int m=queryX.size();
	int n=queryX[0].size();
	int i,j;
	vector<vector<double>>query(m,vector<double>(n,0));
	DistanceMatrix(queryX,dataY,D);
	distanceM1=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]+2;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM2=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]+1;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM3=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]-2;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM4=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]-1;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM5=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]-3;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM6=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]+3;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM7=DTWBeginNew(D);
	distanceMin=MyMin(MyMin(distanceM1,distanceM2,distanceM3),distanceM4,distanceM5);
	distanceMin=MyMin(distanceMin,distanceM6,distanceM7);
	return distanceMin;
}


double DTWdisRecur(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D,double disMid,double num,double bound)
{                  
	int m=queryX.size();
	int n=queryX[0].size();
	int i,j;
	vector<vector<double>> queryPlus(m,vector<double>(n,0));
	vector<vector<double>> queryMinus(m,vector<double>(n,0));
	double disMinus=0,disPlus=0,distanceMin=0;
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			queryMinus[i][j]=queryX[i][j]-num;//num是上下偏移值
		}
	}
	DistanceMatrix(queryMinus,dataY,D);
	disMinus=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			queryPlus[i][j]=queryX[i][j]+num;
		}
	}
	DistanceMatrix(queryPlus,dataY,D);
	disPlus=DTWBeginNew(D);
	distanceMin=MyMin(disMid,disMinus,disPlus);
	if (num<=bound)//bound代表上下偏移数目
	{
		return distanceMin;
	}
	else if (distanceMin==disMid)
	{
		return DTWdisRecur(queryX,dataY,D,distanceMin,num/2,bound);
	}
	else if (distanceMin==disMinus)
	{
		return DTWdisRecur(queryMinus,dataY,D,distanceMin,num/2,bound);
	}
	else if (distanceMin==disPlus)
	{
		return DTWdisRecur(queryPlus,dataY,D,distanceMin,num/2,bound);
	}
	else
		return distanceMin;
}

double DTWBeginQueryAndSongSeven(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D)
{
	double distanceMin;
	int m=queryX.size();
	int n=queryX[0].size();
	int i,j;
	DistanceMatrix(queryX,dataY,D);
	distanceMin=DTWBeginNew(D);
	return DTWdisRecur(queryX,dataY,D,distanceMin,1,1);//这里参数是1和1表示仅仅上下平移一次
}

double DTWBeginNew(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=1;j<n;j++)
	{
		D[0][j]=0;
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty)+MyMinTwo(D[1][1],disConst);
	for (j=2;j<m;j++)
	{
		D[j][1]=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty)+MyMinTwo(D[j][1],disConst);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty)+MyMinTwo(D[1][j],disConst);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.7;j++)
		{
			D[i][j]=MyMin(D[i-1][j-1]+penalty,D[i-1][j-2],D[i-2][j-1]+penalty)-Dpenalty+MyMinTwo(D[i][j],disConst);
		}
	}
	for (j=1;j<D[i-1].size() && j<m*1.7;j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}

double DTWBeginNewLSH(vector<vector<double>> &D)//用的是归一化的距离
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=1;j<n;j++)
	{
		D[0][j]=inf;
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty)+MyMinTwo(D[1][1],disConst);
	for (j=2;j<m;j++)
	{
		D[j][1]=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty)+MyMinTwo(D[j][1],disConst);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty)+MyMinTwo(D[1][j],disConst);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.7;j++)
		{
			D[i][j]=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-Dpenalty+MyMinTwo(D[i][j],disConst);
		}
	}
	for (j=1;j<D[i-1].size() && j<m*1.7;j++)
	{
		DL.push_back(D[m-1][j]/m);//用的是归一化的距离,如果query长度不一样
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	if	(n>m+1)
	{
		return D[m-1][n-1]/m;
    }
	else
		return *disIter;
	return D[m-1][n-1]/m;//仅仅返回最后那个，长度已知的情况下
}


double DTWBeginAndRA(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=1;j<n;j++)
	{
		D[0][j]=inf;
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty)+MyMinTwo(D[1][1],disConst);
	for (j=2;j<m;j++)
	{
		D[j][1]=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty)+MyMinTwo(D[j][1],disConst);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty)+MyMinTwo(D[1][j],disConst);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.7;j++)
		{
			D[i][j]=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-Dpenalty+MyMinTwo(D[i][j],disConst);
		}
	}
	return D[m-1][n-1];//仅仅返回最后那个
}


double DTWBeginThreeOld(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=2;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=2;j<n;j++)
	{
		D[0][j]=inf;
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.4;j++)
		{
			D[i][j]+=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-Dpenalty;
		}
	}
	for (j=1;j<D[i-1].size() && j<m*1.4;j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}


double DTWtotalPlusPe(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n;j++)
		{
			D[i][j]+=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-0.3;
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}

double DTWbegin(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	for (i=1;i<m;i++)
	{
		for (j=1;j<n && j<m*2;j++)
		{
			D[i][j]+=MyMin(D[i][j-1]+penalty,D[i-1][j-1],D[i-1][j]+penalty);
		}
	}
	for (j=1;j<D[i-1].size() && j<m*2;j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}

int PitchToTone(vector <vector <double>> &queryPitch)
{
	int n=queryPitch.size();
	double pitchnum=0;
	int k=FREQ,sam=win;
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i][0]!=0)
		{
			pitchnum=queryPitch[i][0];
			pitchnum=k*1000/pitchnum;
			pitchnum=69+12*log(pitchnum/440)/log(2.0);
			cout<<i<<","<<pitchnum<<" ";
			queryPitch[i][0]=pitchnum;
		}
	}
	return 0;
}

//音高序列转换为半音音符序列
int realPitchToToneShengda(vector <float> &queryPitch)
{
	int n = queryPitch.size();
	float pitchnum = 0;
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i]!=0)
		{
			pitchnum = queryPitch[i];
            pitchnum = (12.0f*(pitchnum-log(440.0f)/log(2.0f))+69.0f);
			queryPitch[i] = pitchnum;
		}
	}
	return 0;
}


int realPitchToTone(vector <float> &queryPitch)
{
	int n=queryPitch.size();
	float pitchnum=0;
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i]!=0)
		{

			pitchnum=queryPitch[i];
			pitchnum=69+12*log(pitchnum/440)/log(2.0);
			queryPitch[i]=pitchnum;
		}
	}
	return 0;
}

int realPitchToThreeTone(vector <vector <double>> &queryPitch,vector <vector <double>> &queryPitchTow,vector <vector <double>> &queryPitchThree)
{
	int n=queryPitch.size();
	double pitchnum=0;
	for (int i=0;i!=n;i++)
	{
		queryPitchTow.push_back(queryPitch[i]);
		queryPitchThree.push_back(queryPitch[i]);
		if (queryPitch[i][0]!=0)
		{

			pitchnum=queryPitch[i][0];
			pitchnum=69+12*log(pitchnum/440)/log(2.0);
			int pitch=queryPitch[i][0];
			if ((pitch >=82.4) && (pitch <=1046.5))
			{
				;
			}
			queryPitchTow[i][0]=69+12*log(queryPitch[i][0]*2/440)/log(2.0);
			queryPitchThree[i][0]=69+12*log(queryPitch[i][0]/2/440)/log(2.0);
			queryPitch[i][0]=pitchnum;
		}
	}
	return 0;
}

int realPitchToAnotherTowTone(vector <vector <double>> &queryPitch,vector <vector <double>> &queryPitchTow,vector <vector <double>> &queryPitchThree)
{
	int n=queryPitch.size();
	double pitchnum=0;
	for (int i=0;i!=n;i++)
	{
		queryPitchTow.push_back(queryPitch[i]);
		queryPitchThree.push_back(queryPitch[i]);
		if (queryPitch[i][0]!=0)
		{

			pitchnum=queryPitch[i][0];
			pitchnum=440*pow(2.0,(pitchnum-69)/12);//还原为原始基频
			int pitch=queryPitch[i][0];
			if ((pitch >=82.4) && (pitch <=1046.5))
			{
				;
			}
			queryPitchTow[i][0]=69+12*log(pitchnum*1.2/440)/log(2.0);//加倍
			queryPitchThree[i][0]=69+12*log(pitchnum/1.2/440)/log(2.0);//减半
		}
	}
	return 0;
}


int ToneTorealPitch(vector <vector <double>> &queryPitch)
{
	int n=queryPitch.size();
	double pitchnum=0;
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i][0]!=0)
		{

			pitchnum=queryPitch[i][0];
			pitchnum=pow(2.0,(pitchnum-69)/12)*440;
			queryPitch[i][0]=pitchnum;
		}
	}
	return 0;
}

void readinstance(char *wavename,vector <vector <double>> &queryPitch)
{
	ifstream indexFile(wavename);
	vector <double> pitchNum;
	string songPitchNum;
	while (indexFile>>songPitchNum)
	{
		pitchNum.clear();
		pitchNum.push_back(atof(songPitchNum.c_str()));
		queryPitch.push_back(pitchNum);

	}
	indexFile.close();
}

void discre(vector <vector <double>> &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				x[i-1][j]=x[i][j]-x[i-1][j];
			}
		}

	}
	for (i=0;i<n;i++)
	{
		x[m-1][i]=0;
	}
}


void Zerodiscre(vector <vector <double>> &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0&&x[i-1][j]!=0)
			{			
				x[i-1][j]=x[i][j]-x[i-1][j];
			}
			else
			{
				x[i-1][j]=0;
			}
		}
	}
	for (i=0;i<n;i++)
	{
		x[m-1][i]=0;
	}
}

//线性伸缩
//输入：queryX，原串，stretch，伸缩因子
//输出：dataY，伸缩后的串
void StringToString( vector<float>  &queryX,  vector<float>  &dataY,float stretch)
{
	int i,j,m,n,k;
	m = queryX.size();
	float ratio = 0;
	for (i=0;i<(m-1)*stretch;i++)
	{
		float pitch_query=0;
		k = i/stretch;
		ratio=i/stretch-k;
		if (k<m-1)
		{
			pitch_query=(queryX[k]*(1-ratio)+queryX[k+1]*ratio);
			dataY.push_back(pitch_query);
		}
	}
}

void StringToStringSame( vector<float>  &queryX,  vector<float>  &dataY,float stretch)
{
	int i,j,m,n,k;
	m=queryX.size();
	float ratio=0;
	for (i=0;i<(m-1)*stretch;i++)
	{
		float pitch_query=0;
		k=i/stretch;
		ratio=i/stretch-k;
		if (k<m-1)
		{
			dataY.push_back(queryX[k]);
		}
	}
}

float LinearToDis( vector<float>  &queryX,  vector<float>  &dataY)
{
	int i,j,m,n=0,k;
	m=queryX.size();
	if	(m>0 && dataY.size()>0)
	{
	   float stretch=((double)dataY.size())/queryX.size();
	   float ratio=0;
	   float Dis=0;
	   vector<float>  queryStretchX;
	   int totalm=(m-1)*stretch;
	   for (i=0;i<totalm;i++)
	   {
		   double pitch_query=0;
		   k=i/stretch;
		   ratio=i/stretch-k;
		   if (k<m-1)
		   {

			   pitch_query=(queryX[k]*(1-ratio)+queryX[k+1]*ratio);

			   queryStretchX.push_back(pitch_query);
		   }
	   }
	   Dis=StringMatch(queryStretchX,dataY);
	   return Dis;
	}
	else
		return 0;
}

//queryX伸缩后的音符序列 DataY对应的候选音符序列 length=12  将候选序列进行左右缩短操作 比较与哼唱段距离 去距离最小值
float CalculateOptimalEdge( vector<float>  &queryX,  vector<float>  &dataY,int &left,int &right,int length,float ratio)
{
	int n=queryX.size();
	int m=dataY.size();
	int bestLeft=0;
	int bestRight=0;
	float bestDis=10000;
	float Cdis=0;
	int step=4;
	vector<float>::iterator  Ybegin=dataY.begin();
	vector<float>::iterator  Yend=dataY.end();
	for (int i=0;i< length;i+=step)
	{
		for (int j=0;j<length;j+=step)
		{
			Mean(Ybegin+i,Yend-j);////减均值操作，减均值后>12就-12，<-12就+12

			Cdis=LinearToDisIter(queryX.begin(),queryX.end(),Ybegin+i,Yend-j);//计算候选与哼唱段的最短距离
			if (bestDis>Cdis)
			{//取得最短距离 及候选片段左右滑动的帧数
				bestDis=Cdis;
				bestLeft=i;
				bestRight=j;
			}
		}
	}
	left=bestLeft;
	right=bestRight;
	if (bestLeft!=0)
	{
		dataY.erase(dataY.begin(),dataY.begin()+bestLeft);//左缩短序列
	}
	if (bestRight!=0)
	{
		dataY.erase(dataY.end()-bestRight,dataY.end());//右缩短序列
	}
	Mean(dataY.begin(),dataY.end());//减均值操作
	return bestDis;//返回最短距离
}
//哼唱序列及候选序列  比较两者距离 取最短距离值返回
float LinearToDisIter( vector<float>::iterator  Xbegin,  vector<float>::iterator  Xend,
					   vector<float>::iterator  Ybegin,  vector<float>::iterator  Yend)
{
	int i,j,m,n=0,k;
	m=Xend-Xbegin;
	n=Yend-Ybegin;
	float dis=0;
	if	(m>0 && n>0)
	{
		float stretch=((double)n)/m;
		float ratio=0;
		float Dis=0;
		int numY=0;
		int totalm=(m-1)*stretch;
		for (i=0;i<totalm;i++)
		{
			float pitch_query=0;
			k=i/stretch;
			ratio=i/stretch-k;
			if (k<m-1)
			{

				numY++;
				pitch_query=(*(Xbegin+k)*(1-ratio)+*(Xbegin+k+1)*ratio);
				Dis+=MyMinTwo(abs(pitch_query-*(Ybegin+i)),disConstString);

			}
		}

		if(numY!=0)
		{
			Dis/=numY;
		}
		return Dis;
	}
	else
		return 0;
}


double LinearToDisUltimate(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int &ultimateNum)
{
	int i,j,m,n=0,k;
	m=queryX.size();
	if	(m>0 && dataY.size()>0)
	{
		n=queryX[0].size();
		double stretch=((double)dataY.size())/queryX.size();
		double ratio=0;
		double Dis=0;
		vector< vector<double> > queryStretchX;
		int totalm=(m-1)*stretch;
		for (i=0;i<totalm;i++)
		{
			vector<double> pitch_query;
			k=i/stretch;
			ratio=i/stretch-k;
			if (k<m-1)
			{
				for (j=0;j<n;j++)
				{
					pitch_query.push_back(queryX[k][j]*(1-ratio)+queryX[k+1][j]*ratio);
				}
				queryStretchX.push_back(pitch_query);
			}
		}
		Dis=StringMatchUltimate(queryStretchX,dataY,ultimateNum);
		return Dis;
	}
	else
		return 0;
}



void StringToStringNoMean(vector< vector<double> > &queryX, vector< vector<double> > &dataY,double stretch)
{
	int i,m,n,k;
	m=queryX.size();
	n=queryX[0].size();
	for (i=0;i<m*stretch;i++)
	{
		k=i/stretch;
		if (k<m)
		{
			dataY.push_back(queryX[k]);
		}
	}
}

float StringMatch( vector<float>  &queryX,  vector<float>  &dataY)
{
	int i,j,m,n;
	m=queryX.size();
	n=dataY.size();
	float dis=0;
	int num=MyMinTwo(m,n);
	for (i=0;i< num;i++)
	{
		dis+=MyMinTwo(abs(queryX[i]-dataY[i]),disConstString);
	}
	if(num!=0)
	{
		dis/=num;
	}
	return dis;
}


double StringMatchUltimate(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int &ultimateNum)
{
	int i,j,m,n;
	m=queryX.size();
	n=dataY.size();
	double dis=0;
	int num=MyMinTwo(m,n);
	for (i=0;i< num;i++)
	{
		//dis+=MyMinTwo(MyDistance(queryX[i],dataY[i]),disConstString);
	}
	ultimateNum+=num;
	return dis;
}

double StringMatchToDis(vector< vector<double> > &queryX, vector< vector<double> > &dataY)
{
	int n=dataY.size();
	double stretch=0.75;
	double distanceM;
	vector <double> distanceStretch;
	for (;stretch<1.3;)
	{
		vector<vector<double>> queryStretch;
		stretch+=0.05;
		distanceStretch.push_back(distanceM);
	}
	stable_sort(distanceStretch.begin(),distanceStretch.end());
	distanceM=distanceStretch[0];
	return distanceM;
}


double StringMatchToDisMapRALSHNewPairVariancePositionVariance( vector<float>  &queryX,  vector<float>  &dataY,
															   int recurse,int pairNum,double MidPercentage,int &ultimateNum)
{
	int i;
	int sizeX=queryX.size();
	int sizeY=dataY.size();
	int MidPos=sizeX/2;
	int BeginMatchPos=sizeX*MidPercentage*1/3;
	double MovePoints=(sizeX-BeginMatchPos*2)/(double)pairNum;
	if (MovePoints<1)
	{
		MovePoints=1;
	}
	int MidDataYSize=dataY.size()/2;
	int MinDisPos=0;//最小距离的位置
	double distanceM;
	map <double,int>disMap;
	vector <double> distanceStretch;
	if (sizeX<=8 || sizeY<=8)
	{
		distanceM=LinearToDis(queryX,dataY);
		ultimateNum++;
		return distanceM;
	}
	vector<float> DataY_L(dataY.begin(),dataY.begin()+MidDataYSize);
	vector<float>  DataY_R(dataY.begin()+MidDataYSize,dataY.end());

	for (i=0;i<=pairNum;i++)
	{
		vector<float>  queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+i*MovePoints);
		vector<float>  queryX_R(queryX.begin()+BeginMatchPos+i*MovePoints,queryX.end());

		distanceM=LinearToDis(queryX_L,DataY_L)+LinearToDis(queryX_R,DataY_R);
		disMap.insert(make_pair(distanceM,i));
		distanceStretch.push_back(distanceM);

		if (queryX.begin()+BeginMatchPos+i*MovePoints==queryX.end()-2)
		{
			i+=pairNum;
		}
	}
	stable_sort(distanceStretch.begin(),distanceStretch.end());
	distanceM=distanceStretch[0];
	if (recurse==0)
	{
		ultimateNum+=2;
		return distanceM;
	}
	else
	{
		if (disMap.count(distanceM))
		{
			MinDisPos=disMap[distanceM];
			recurse--;
			MidPercentage=MidPercentage*1.1;
			pairNum=pairNum-2;
			if (pairNum<3)
			{
				pairNum=3;
			}
			 vector<float>  queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+MinDisPos*MovePoints);
			vector<float>  queryX_R(queryX.begin()+BeginMatchPos+MinDisPos*MovePoints,queryX.end());
			distanceM=StringMatchToDisMapRALSHNewPairVariancePositionVariance(queryX_L,DataY_L,recurse,pairNum,MidPercentage,ultimateNum)+
				StringMatchToDisMapRALSHNewPairVariancePositionVariance(queryX_R,DataY_R,recurse,pairNum,MidPercentage,ultimateNum);

			return distanceM;
		}
	}
	return distanceM;
}

//KTRA计算最小距离
float RAPositionVarianceOptimal( vector<float>  &queryX,  vector<float>  &dataY,
															   int recurse,int pairNum,float MidPercentage,int &ultimateNum)//3，5,1
{
	int i;
	int sizeX=queryX.size();
	int sizeY=dataY.size();
	int MidPos=sizeX/2;
	int BeginMatchPos=sizeX*MidPercentage*1/3;
	float MovePoints=(sizeX-BeginMatchPos*2)/(double)pairNum;
	if (MovePoints<1)
	{
		MovePoints=1;
	}
	int MidDataYSize=dataY.size()/2;
	int MinDisPos=0;//最小距离的位置
	double distanceM;
	map <float,int>disMap;
	vector <float> distanceStretch;
	if (sizeX<=8 || sizeY<=8)
	{
		distanceM=LinearToDis(queryX,dataY);//返回最小距离值
		ultimateNum++;
		return distanceM;
	}
	vector<float> DataY_L(dataY.begin(),dataY.begin()+MidDataYSize);
	vector<float>  DataY_R(dataY.begin()+MidDataYSize,dataY.end());

	for (i=0;i<=pairNum;i++)
	{
		distanceM=LinearToDisIter(dataY.begin(),dataY.begin()+MidDataYSize,queryX.begin(),queryX.begin()+BeginMatchPos+i*MovePoints)+
			LinearToDisIter(dataY.begin()+MidDataYSize,dataY.end(),queryX.begin()+BeginMatchPos+i*MovePoints,queryX.end());//将候选与哼唱段分成两段，查询段循环分割 进行RA计算
		disMap.insert(make_pair(distanceM,i));//存储最短距离及相应循环次数序号
		distanceStretch.push_back(distanceM);//存储最短距离
		if (queryX.begin()+BeginMatchPos+i*MovePoints==queryX.end()-2)
		{//判断是否到序列末尾，退出循环
			i+=pairNum;
		}
	}
	stable_sort(distanceStretch.begin(),distanceStretch.end());//距离由小到大排序
	distanceM=distanceStretch[0];//去最小距离值
	if (recurse==0)//循环次数为零 跳出
	{
		ultimateNum+=2;
		return distanceM;
	}
	else
	{

		if (disMap.count(distanceM))
		{
			MinDisPos=disMap[distanceM];//得到最小距离循环次数i
			recurse--;
			MidPercentage=MidPercentage*1.1;
			pairNum=pairNum-2;
			if (pairNum<3)
			{
				pairNum=3;
			}
			vector<float>  queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+MinDisPos*MovePoints);
			vector<float>  queryX_R(queryX.begin()+BeginMatchPos+MinDisPos*MovePoints,queryX.end());
			distanceM=RAPositionVarianceOptimal(queryX_L,DataY_L,recurse,pairNum,MidPercentage,ultimateNum)+
				RAPositionVarianceOptimal(queryX_R,DataY_R,recurse,pairNum,MidPercentage,ultimateNum);//将查询段与候选段分段进行RA计算
			return distanceM;
		}
	}
	return distanceM;
}

void StringTosignature(vector<float>  &dataY,  signature_t  &Y)
{
	int n=dataY.size();
	int m=emdLength;
	n=MyMinTwo(n,m);
	int num=1;
	for (int i=0;i<n-1 ;i++)
	{
		if (dataY[i]!=dataY[i+1])
		{
			num++;
		}
	}
	if (num<(MAX_SIG_SIZE-1))
	{
		Y.n=num;
	}
	else
		Y.n=MAX_SIG_SIZE-1;

	int currentNote=0;
	if (n>0)
	{
		Y.Features[0]=dataY[0];
		Y.Weights[0]=1;
	}
	for (int i=0;i<n-1 && currentNote<MAX_SIG_SIZE-1;i++)
	{
		if (dataY[i]==dataY[i+1])
		{
			Y.Weights[currentNote]+=1;
		}
		else
		{
			++currentNote;
			Y.Features[currentNote]=dataY[i+1];
			Y.Weights[currentNote]=1;
		}
	}
}

bool CompareLastName(const pair<int,string> &p1,const pair<int,string> &p2)
{
	return p1.first>p2.first;
}

vector<pair<int,int>> DuplicateSegmentBegin(vector<float> &tone)	//从音符序列tone任意位置匹配开头，若匹配长度大于100帧，记录该位置和持续帧数并返回
{
	vector<pair<int,int>> dupl;
	pair<int,int> segment;
	int sizeA=tone.size();
	double temp=0;
	int dupNum=0;
	for (int i=1;i<sizeA;i++)
	{
		temp=tone[i];
		dupNum=0;
		if (temp==tone[0])
		{
			for (int j=i;j<sizeA;j++)
			{
				temp=tone[j];
				if (temp==tone[j-i])
				{
					dupNum++;
				}
				else
					j+=sizeA;
			}
		}
		if (dupNum>100)
		{
			segment.first=i;
			segment.second=i+dupNum;
			dupl.push_back(segment);
		}
	}
	return dupl;
}


void OneFileToMultiFile(string fileName,int ThreadNum)
{
	ifstream wavList(fileName.c_str());
	string oneLine;
	char buffer[20];
	int totalWav=0;
	int fileCnum=0;
	for (int i=0;i<ThreadNum;i++)
	{
		string cmd("del ");
		cmd+=fileName;
		cmd+=itoa(i,buffer,10);
		system(cmd.c_str());
	}
	while (wavList>>oneLine)
	{
		totalWav++;
		fileCnum=totalWav%ThreadNum;
		string fileN=fileName+itoa(fileCnum,buffer,10);
		ofstream fileSplit(fileN.c_str(),ofstream::app);
		fileSplit<<oneLine<<endl;
		fileSplit.close();
	}
	wavList.close();
}


void MultiFileDel(string fileName,int ThreadNum)
{
	ifstream wavList(fileName.c_str());
	string oneLine;
	char buffer[20];
	for (int i=0;i<ThreadNum;i++)
	{
		string cmd("del ");
		cmd+=fileName;
		cmd+=itoa(i,buffer,10);
		system(cmd.c_str());
	}

}

//减去指定均值
void MeanNoteLSH(vector<float> &x, float mean)
{
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		x[i]-=mean;
	} 
}

//计算均值
float MeanLSH(vector<float> &x)
{
	float mean=0;
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		mean+=x[i]/m;
	}
	return mean;

}

//将float数组复制到vector中
void FloatCopyToVector(vector <float> &des, float *src, int len)
{
	for (int i=0; i<len; i++)
	{
		des.push_back(src[i]);	
	}
}


//查询函数：
//输入：wavename,查询文件路径，param,参数信息，
//isWriteLSHQueryToFile,是否将LS后的LSH查询数据及索引写入文件
//isUseDBNResult，是否用DBN查询结果
//输出：songFive，查询结果
int WavToSongFive(char *wavename, ParamInfo *param, vector<string>& songFive, bool isWriteLSHQueryToFile, bool isUseDBNResult)
{
	string wavOut("wavPitch.txt");	//存放wav音高序列结果的文件
	ofstream wavPitch(wavOut.c_str(),ofstream::app);

	//从参数结构体param中提取查询用到的参数
	ParamInfo *pci = param;	//LSH参数结构体
	ParamInfo *pciNote = param+1;	//NLSH参数结构体

	map<string,vector<float>> indexSongName(pci->indexSongName);	//索引pv文件路径和对应的一维音高序列
	PRNearNeighborStructT IndexHuming = pci->IndexHuming;	//LSH点的RNN索引
	PRNearNeighborStructT IndexHumingNote = pciNote->IndexHuming;	//NLSH点的RNN索引
	map<unsigned long,pair<string,short>> IndexLSH(pci->IndexLSH);	//LSH索引，记录LSH点的序号，带路径的文件名，起始位置
	map<unsigned long,pair<string,pair<short,short>>> IndexLSHNote(pciNote->IndexLSHNote);	//NLSH索引，记录NLSH点的序号，带路径的文件名，起始位置，持续帧数
	int stepFactor = pci->stepFactor;	//选择LSH点的间隔数（间隔几个点抽取一个点）
	IntT RetainNum = pci->RetainNum;		//LSH每个点仅保留的点数
	IntT RetainNumNote = pciNote->RetainNum;	//NLSH每个点仅保留的点数
	IntT LSHFilterNum = pci->LSHFilterNum;	//LSH滤波保留的点数
	IntT LSHFilterNumNote = pciNote->LSHFilterNum;	//NLSH滤波保留的点数
	float stepRatio = pci->stepRatio;	//query的LSH变换帧移  什么是变换帧移？？
	songFive.clear();	//查询结果清零

	//统计时间相关变量
	clock_t firstTime,lastTime,firstTimeTemp,lastTimeTemp,firstTimeTempLSH1,
		lastTimeTempLSH1,firstTimeTempLSH2,lastTimeTempLSH2;
	double OneSongLSHTime = 0;
	static double totalLSHTime = 0;
	double OneSongLSTime = 0;
	static double totalLSTime = 0;
	double OneSonglshnoteTime = 0;
	static double totalLSHNoteTime = 0;
	double OneSonglshPitchTime = 0;
	static double totalLSHOnlyNoteTime = 0;
	static double totalLSHOnlyPitchTime = 0;
	static double totalLSHRetrievalPostProcessPitchTime = 0;
	static double totalLSHRetrievalPostProcessPitchTimeNote = 0;
	static double totalLSHPitchTime = 0;
	static double totalLSHPitchFirstTime = 0;
	static double totalLSHNoteFirstTime = 0;
	static double totalLSHoneTime = 0;
	static double totalLSHtwoTime = 0;
	static double totalLSHthreeTime = 0;
	double OneSongEMDTime = 0;
	static double totalEMDTime = 0;
	double OneSongRATime = 0;
	static double totalRATime = 0;
	static int total = 0;

	//盛大开源代码：特征提取函数所需参数
	float *pFeaBuf = NULL;	//音高序列
	int nFeaLen = 0;	//音高序列长度
	SNote *QueryNotes = NULL;	//音符序列
	int nNoteLen = 0;	//音符序列长度
	float ratio = 0.5;	//后处理重采样音高序列的比例参数

	vector <float> queryPitch;	//pFeaBuf直接转换的音高序列
	vector <float> queryPitchNote;	//QueryNotes转换的一维音符序列
	map<float ,string> songDis;//记录查询段与候选段最小距离 候选段歌曲名
	
	feature_t * NoteEmd = NULL;	//记录减均值后的音符，以音符为单位
	float * NoteDuration = NULL;	//记录每个音符持续时间

	signature_t query;	//记录音符长度，减均值后的音符，每个音符持续时间
	static int shengdaPitch = 0;	//记录特征提取次数
	vector <float> Dis;//经过所有比对后的距离 
	bool returnN = false;//是否进行LSH检索
	static int enhance = 0;

	//盛大开源代码：特征提取函数，提取wav中的基于帧和音高的旋律特征
	//输入：filename：wav文件路径
	//ratio：后处理重采样音高序列的比例参数
	//输出：pFeaBuf：音高序列，nFeaLen：音高序列长度
	//Query：音符序列，nNoteLen：音符序列长度
	//返回值：0：正常，ERROR_CODE_TOO_SHORT_INPUT：音高序列长度小于20，此时pFeaBuf为空
	cout<<wavename<<endl;
	int reNum = SMelodyFeatureExtraction(wavename,pFeaBuf,nFeaLen,QueryNotes,nNoteLen,ratio);

	if (reNum != ERROR_CODE_TOO_SHORT_INPUT)	//提取的音高序列长度不小于20
	{
		shengdaPitch++;	//记录特征提取次数
		FloatCopyToVector(queryPitch,pFeaBuf,nFeaLen);	//原始音高序列（未切分之前的音高）拷贝到vector中

		int emdl = MyMinTwo(nFeaLen,emdLength);	//音高序列时长，emdLength为哼唱音高序列的时长上限
		int lengC = 0;	//记录音符序列总时长
		int lengNote = 0;	//记录实际音符序列长度

		realPitchToToneShengda(queryPitch);	//音高序列转换为半音音符序列

		float meanNote = MinusMeanSmooth(queryPitch);	//减均值操作，减均值后>12就-12，<-12就+12，返回均值  
		wavPitch<<wavename<<":"<<endl;
		if(!wavPitch)
		{
			cout<<"fail open wavPitch"<<endl;	
		}
		for(int k=0;k<queryPitch.size();k++)
		{
			
			wavPitch<<queryPitch[k]<<" ";
		}
		wavPitch<<endl;
		NoteEmd = (feature_t *)malloc(nNoteLen*sizeof(feature_t));	//记录减均值后的音符，以音符为单位
		NoteDuration = (float *)malloc(nNoteLen*sizeof(float));	//记录每个音符持续时间

		//从QueryNotes中提取减均值后的音符，每个音符持续时间
		for (int i=0;i<nNoteLen;i++)
		{
			lengNote++;	//记录实际音符序列长度

			if (QueryNotes[i].fNoteValue != 0)	//音符值不为零
				NoteEmd[i] = QueryNotes[i].fNoteValue - meanNote;	//记录减均值的音符
			else	//音符值为零
				NoteEmd[i] = QueryNotes[i].fNoteValue;	//直接记录，不减均值

			//转换为一维音符序列
			for (int j=0;j<QueryNotes[i].fNoteDuration;j++)	
				queryPitchNote.push_back(NoteEmd[i]);

			//记录每个音符持续时间
			if (lengC+QueryNotes[i].fNoteDuration <= emdLength)	//音符序列时长未超过音高序列时长，直接记录
				NoteDuration[i] = QueryNotes[i].fNoteDuration;
			else	//音符序列时长超过音高序列时长，调整最后一个音符持续时间
				NoteDuration[i] = QueryNotes[i].fNoteDuration-(emdLength-lengC);

			lengC += QueryNotes[i].fNoteDuration;

			if (lengC >= emdl) break;	//音符序列时长超过音高序列时长，跳出
		}
		query.n = lengNote;	//记录音符长度
		query.Features = NoteEmd;	//记录减均值后的音符，以音符为单位
		query.Weights = NoteDuration;	//记录每个音符持续时间
	}
	ofstream shengdaTimes("wav.result",ofstream::app);	//追加打开文件
	shengdaTimes<<"盛大提取次数："<<shengdaPitch<< endl;	//输出是第几次提取
	shengdaTimes.close();

	//释放pFeaBuf和QueryNotes空间
	if(NULL != pFeaBuf)
	{
		delete[] pFeaBuf;
		pFeaBuf=NULL;
	}
	if(NULL!=QueryNotes)
	{
		delete[] QueryNotes;
		QueryNotes=NULL;
	}

	signature_t candiY;

	candiY.Features = (feature_t *) malloc((MAX_SIG_SIZE-1)*sizeof(feature_t));
	candiY.Weights = (float *) malloc((MAX_SIG_SIZE-1)*sizeof(float));
	
	//LS(线性伸缩)参数
	float FloorLevelInitial=0.6;	//伸缩因子初始值
	float FloorLevel = FloorLevelInitial;	//伸缩因子当前值
	float UpperLimit=1.7;	//伸缩因子上限
	float StretchStep=0.1;	//伸缩因子步长
	int MatchBeginPos=6;	//代表query和dataY相差点数必须在这个1/MatchBeginPos范围之内才进行匹配

	static int CandidatesDTWAll=0;
	float ratioAll=2.6;
	static int filter0ne=0;
	static int filterTwo=0;
	static int filterThree=0;
	vector <string> tempList;
	vector <float > tempDis1;
	vector <float > tempDis2;
	vector <float > tempDis3;

	bool isPrintLSHQueryVectorLS = 0;

	vector<int> allStretchCandidates;//记录伸缩后的所有候选点
	vector<int> allCorrectCan;//记录所有正确候选
	ofstream filename("LSHCandidateCorrect.txt",ofstream::app);
	static int exitCandidateNum=0;

	for (int recur=0;recur<3;recur++)
	{
		if (recur==0)
		{
			filter0ne++;
		}
		else if (recur==1)
		{
			filterTwo++;
		}
		else if (recur==2)
		{
			filterThree++;
		}
		vector <vector<vector<float>>> LSHQueryVectorLinearStretching;	//记录不同伸缩因子下抽取的LSH点集
		vector <vector<vector<float>>> LSHQueryVectorLinearStretchingNote;	//记录不同最长帧下抽取的NLSH点集
		vector<pair<short, short>> posPair;	//记录NLSH点的起始位置和持续帧数
		

		if (recur==0)
		{

			//设置LS参数 没有伸缩

			FloorLevelInitial = 1;
			FloorLevel = FloorLevelInitial;
			StretchStep = 0.1;
			UpperLimit = 1;

			//抽取一维音高序列的NLSH点
			//输入：queryPitchNote，查询的一维音符序列
			//noteMinFrame，音符最短持续帧数，不足则去除
			//noteMaxFrame，音符最长持续帧数，超过则切分
			//NLSHsize,NLSH点维数

			//输出：posPairvector，记录LSH点的起始位置和抽取LSH点音符长度
			//LSHQueryVectorLinearStretchingNote，记录NLSH点，三维的

			QueryPitchToLSHVectorLinearStretchingShortToMoreNoteFirst(posPair,queryPitchNote,
				LSHQueryVectorLinearStretchingNote,
				param[1].noteMinFrame,param[1].noteMaxFrame,param[1].LSHsize);
		}
		else if (recur==1)
		{

			//设置LS参数 从0.8到1.4 步长为0.2 进行4次伸缩

			FloorLevelInitial=0.8;
			FloorLevel=FloorLevelInitial;
			StretchStep=0.2;
			UpperLimit=1.4;
		}
		else if (recur==2)
		{

			//设置LS参数 进行12次伸缩

			FloorLevelInitial=0.6;
			FloorLevel=FloorLevelInitial;
			StretchStep=0.1;
			UpperLimit=1.7;

			//改变音符最长帧，再抽取NLSH点（没有做LS）
			for(int recur_t=0; recur_t<1; recur_t++)

			{//循环一次

				int noteMaxFrame_t;
				if (recur_t==0)
				{
					noteMaxFrame_t = param[1].noteMaxFrame+2;
				}
				else
				{
					noteMaxFrame_t = param[1].noteMaxFrame-2;
				}
				
				//抽取一维音高序列的NLSH点
				//输入：queryPitch，查询的一维音符序列
				//noteMinFrame，音符最短持续帧数，不足则去除

				//noteMaxFrame，音符最长持续帧数，超过则切分 noteMaxFrame_t=12
				//NLSHsize,NLSH点维数
				//输出：posPairvector，记录LSH点的起始位置和持续长度
				//LSHQueryVectorLinearStretching，记录LSH点

				QueryPitchToLSHVectorLinearStretchingShortToMoreNoteFirst(posPair,queryPitchNote,
					LSHQueryVectorLinearStretchingNote,
					param[1].noteMinFrame,noteMaxFrame_t,param[1].LSHsize);
			}
		}

		//线性伸缩并抽取LSH点
		//输入：queryPitch，一维音高向量
		//FloorLevel，初始伸缩因子，UpperLimit，伸缩因子上限，StretchStep，伸缩步长
		//stepFactor，抽取间隔，stepRatio，抽取窗移，LSHsize，抽取窗长，recur，上层函数的循环序号
		//输出：LSHQueryVectorLinearStretching，所有伸缩因子下的LSH点集
		QueryPitchToLSHVectorLinearStretchingShortToMore(queryPitch,LSHQueryVectorLinearStretching, 
			FloorLevel, UpperLimit,stepFactor,stepRatio, StretchStep,recur,param[0].LSHsize);
		
		if(!isPrintLSHQueryVectorLS)
		{
			if(isWriteLSHQueryToFile)
			{
				//将LS后的LSH点写入文件
				//输入：LSHVectorLS，LS后的LSH点，每个点为一个音高序列，vector[i][j][k]表示第i个伸缩因子下的第j个采样点的第k个数据
				//filename，输出文件路径
				LSHVectorLSToFile(LSHQueryVectorLinearStretching,query_vector_fname);
				//将LS后的LSH索引写入文件
				//输入：LSHVectorLS，LS后的LSH点，每个点为一个音高序列，vector[i][j][k]表示第i个伸缩因子下的第j个采样点的第k个数据
				//filename，输出文件路径，输出文件中，每行为上述LSH点对应的索引（目前使用LSH点所属的文件名）
				//后一个输出文件用于统计每个查询文件的数据数
				IndexLSHLSToFile(LSHQueryVectorLinearStretching,wavename,query_index_fname,query_counter_fname);
				cout<<"打印 "<<wavename<<" 的QueryLSHLS 数据和索引 完毕"<<endl;
			}
			isPrintLSHQueryVectorLS = 1;
		}

		//取消音符的LSH需要修改三个地方
		int LinearCoe = 0;
		int LinearCoeTotal = LSHQueryVectorLinearStretching.size();	//线性伸缩的LSH点集数目
		vector<vector<IntT>> IndexCandidatesStretch;
		vector<vector<float>> IndexCandidatesDis;
		vector<vector<IntT>> CandidatesNumStretch;

		vector<IntT> CandidatesFilter;
		firstTime=clock();
		map<int,vector<int>> :: iterator samePointIte;

		firstTimeTempLSH1 = clock();
		int edge=6;
		float LSratio=0.4;
		int qRecurse=1;
		if (recur==10)
		{
			edge=1;
		}
		else 
			edge=6;

		if (recur!=1) //此为音符的检索
		{//循环不是1
			//下面为音符的LSH搜索
#if 1
			if(!isUseDBNResult)
			{
				LinearCoe=0;
				PPointT *QueriesArray=NULL;	//NLSH点集

				if (LSHQueryVectorLinearStretchingNote[LinearCoe].size()>0)	//若当前的NLSH点集不为空
				{
					//从LSH向量索引中读数据集，读入PPointT*中，读入每个LSH点时记录序号和LSH点的平方和
					//输入：LSHVector：LSH点集
					QueriesArray = readDataSetFromVector(LSHQueryVectorLinearStretchingNote[LinearCoe]);	//得到当前NLSH点集

					Int32T nPointsQuery = LSHQueryVectorLinearStretchingNote[LinearCoe].size();	//当前NLSH点集大小

					IntT dimension = 6;	//NLSH点维数
					if (nPointsQuery>0)
					{
						dimension = LSHQueryVectorLinearStretchingNote[LinearCoe][0].size();	//NLSH点真实维数
					}

					IntT LSHFilterReturnNum = 0;
					IntT *IndexArray = NULL;
					int IndexArraySize = 1000000;	//最大查找候选数目

					IndexArray = (IntT *)MALLOC(IndexArraySize *sizeof(IntT));
					double *IndexArrayDis = (double *)MALLOC(IndexArraySize *sizeof(double));
					IntT *IndexFilterArray = NULL;

					IndexFilterArray = (IntT *)MALLOC(IndexArraySize/2 *sizeof(IntT));//这是什么？
					IntT * NumArray = (IntT *)MALLOC(nPointsQuery *sizeof(IntT));	//每个点返回的数目

					//得到NLSH结果，即对每个LSH点进行RNN查询，返回RetainNumNote个候选，最后返回所有点的候选数目
					//输入：QueriesArray,当前NLSH点集，nPointsQuery，当前点集大小
					//IndexArraySize，最大查找候选数
					//IndexHumingNote，NLSH点的RNN索引，RetainNum，NLSH每个点仅保留的点数
					//dimension，NLSH点维数，LSHFilterNum，NLSH滤波保留的点数
					//输出：
					//IndexArray，候选序号，NumArray，每个点返回的保留数目，IndexArrayDis，所有候选RA距离
					//IndexFilterArray，所有候选序号，sizeFilter，
					IntT ResultSize = LSHStructToResultOnePointRetainSeveral(QueriesArray,nPointsQuery,IndexArraySize, 
						IndexArray,IndexHumingNote,NumArray,RetainNumNote , dimension,LSHFilterNumNote,IndexFilterArray,
						LSHFilterReturnNum,IndexArrayDis);
				
					vector<IntT> IndexCandidates;
					vector <float> DisCandidates;
					vector<IntT> CandidatesNum;	//每个点返回的数目存入Vector
					int curreIndex=0;
					//返回的每个点的所有候选序号均存入到CandidatesFilter中
					for (int i=0;i<LSHFilterReturnNum;i++)
					{
						CandidatesFilter.push_back(IndexFilterArray[i]);
					}

					bool insertY=false;
					for (int i=0;i<nPointsQuery;i++)
					{//遍历当前NLSH点集
						int numS=0;
						int siz=NumArray[i];//每个LSH点返回的候选点数
						for (int j=0;j<siz;j++)
						{
							if (samePoint.count(IndexArray[j+curreIndex]))//查看候选序号存在，返回0或1，测试结果显示，samepoint没有数值，基本无用
							{
								numS=0;

								samePointIte=samePoint.find(IndexArray[j+curreIndex]);//得到候选序号对应的候选点的迭代器或指针
								numS=samePointIte->second.size();//候选点的大小
								IndexCandidates.push_back(IndexArray[j+curreIndex]);//存入候选序号
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);//存入候选点的距离
								for (int k=0;k<samePointIte->second.size();k++)
								{//去除部分候选点  规则？？
									insertY=true;
									for (int l=0;l<siz;l++)
									{
										if (samePointIte->second[k]==IndexArray[l+curreIndex])
										{
											insertY=false;
										}
									}
									if (insertY==true)
									{
										IndexCandidates.push_back(samePointIte->second[k]);
										DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
									}
									else
										numS--;

								}
								NumArray[i]+=numS;
							
							}
						
							else//查看候选序号不存在，存入候选序号及距离
							{
								IndexCandidates.push_back(IndexArray[j+curreIndex]);
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
							
							}

						}

						curreIndex+=siz;//进行下一个点
						CandidatesNum.push_back(NumArray[i]);//存入当前LSH点候选点数
					}
					IndexCandidatesStretch.push_back(IndexCandidates);//存入点集的候选点序号
					CandidatesNumStretch.push_back(CandidatesNum);//存入此次LSH点候选点数
					IndexCandidatesDis.push_back(DisCandidates);
					for (int i=0;i<nPointsQuery;i++)//释放空间
					{
						free(QueriesArray[i]->coordinates);
						free(QueriesArray[i]);
					}
					free(IndexArray);
					free(IndexFilterArray);
					free(NumArray);
					free(QueriesArray);
					free(IndexArrayDis);
				}
				else//若当前的NLSH点集为空
				{
					vector <float> DisCandidates;
					vector<IntT> IndexCandidates;
					vector<IntT> CandidatesNum;//每个点返回的数目存入Vector
					IndexCandidatesStretch.push_back(IndexCandidates);//存入候选点序号
					CandidatesNumStretch.push_back(CandidatesNum);//存入此次LSH点候选点数
					IndexCandidatesDis.push_back(DisCandidates);
				}

				lastTime=clock();
				OneSongLSHTime=(double)(lastTime-firstTimeTempLSH1)/CLOCKS_PER_SEC;
				totalLSHOnlyNoteTime+=OneSongLSHTime;
				OneSongLSHTime=(double)(lastTime-firstTime)/CLOCKS_PER_SEC;
				totalLSHTime+=OneSongLSHTime;
			}
#endif

			//下面为统计LSH准确率
			FloorLevel=FloorLevelInitial;
			int sizeLSHNote=IndexCandidatesStretch.size();
			NLSHCandidateCorrect(wavename,IndexLSHNote,IndexCandidatesStretch[sizeLSHNote-1],CandidatesNumStretch[sizeLSHNote-1]);
			//LSHresultRate(wavename,queryPitch.size(),stepFactor,FloorLevel,IndexCandidatesStretch,IndexLSH,IndexLSHNote,StretchStep,IndexCandidatesDis);
			//LSHresult(wavename,queryPitch.size(),stepFactor,FloorLevel,IndexCandidatesStretch,IndexLSH,StretchStep);

			//下面为LSH滤波
			map<string ,int> LSHFilterMap;//对候选歌曲名计数，<候选歌曲名，对应候选次数>
			LSHFilter(wavename,LSHFilterMap,CandidatesFilter,IndexLSH);//得到LSHFilterMap

#if 1

			firstTime=clock();
		
			for (int q=0;q<qRecurse;q++)//LS 线性缩放 再次循环一次 去除伪候选
			{

				int sizeLSH=IndexCandidatesStretch.size();//获得NLSH点集大小
				vector<float>  queryStretchPitch;
				vector< vector<float> >  CandidatesDataY;
				vector<float>  CandidatesDataYDis;
				vector <string> SongNameMapToDataY;
				int CandidatesSizeInDWT=0;//返回的要精确匹配的数目
				map <string , short > SongMapPosition;

#if 0  //这里是frame的后处理，不编译

				for	(FloorLevel=FloorLevelInitial,LinearCoe=0;FloorLevel<UpperLimit+StretchStep && LinearCoe<sizeLSH-1 &&LinearCoe< LinearCoeTotal;FloorLevel+=StretchStep,LinearCoe++)
				{
					map <string , vector<pair<short, double>> > SongMapPositionAll;
					StretchStep=0.1;
					int StepFactorCurrent=int(stepFactor*stepRatio*FloorLevel/**FloorLevel*/);

					if (q==0)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,edge,edge*2); //此为全部匹配（不是从头开始唱）
					}
					else if (q==3)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,6,0); //此为全部匹配（不是从头开始唱）
					}
					else if (q==4)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,-6,0); //此为全部匹配（不是从头开始唱）
					}
					else if (q==1)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,4,8); //此为全部匹配（不是从头开始唱）
					}
					else if (q==2)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,-4,-8); //此为全部匹配（不是从头开始唱）
					}
				}	
#endif	

#if 1 //这里为音符的扩展
				sizeLSH=IndexCandidatesStretch.size();
				map <string , vector<pair<short, double>> > SongMapPositionAll;
				map <string , vector<pair<short, double>> >::iterator iter;
				vector <string> AllCandidateSong;
				////MatchBeginPos代表query和dataY相差点数必须在这个1/MatchBeginPos范围之内才进行匹配
				//CandidatesSizeInDWT返回要精确匹配的数目
				//posPair NLSH点的起始位置和持续长度
				//IndexCandidatesStretch[sizeLSH-1],CandidatesNumStretch[sizeLSH-1] 候选序号及候选点数
				//FloorLevel初始伸缩值         IndexLSHNote NLSH索引 NLSH点的序号，带路径的文件名，起始位置，持续帧数
				//queryPitchNote,wav由音高序列转换的一维音符序列，queryStretchPitch，伸缩后的序列，edge起始位置偏移量
				//输出 SongNameMapToDataY 候选歌曲名，CandidatesDataY NLSH点对应的候选音高序列 ，SongMapPositionAll 存入候选歌曲信息，包括候选歌曲名，哼唱段对应的的偏移位置及其实际长度。 
				//indexSongName 索引pv文件路径和对应的一维音高序列
				IndexSignToQueryAndDataVectorHummingMatchLeastALLNote( posPair,IndexCandidatesStretch[sizeLSH-1],CandidatesNumStretch[sizeLSH-1],FloorLevel,
				IndexLSHNote,5,queryPitchNote,queryStretchPitch,CandidatesDataY,
				indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
				SongMapPositionAll,edge,edge*2);
				//正确率统计
				for(iter=SongMapPositionAll.begin();iter!=SongMapPositionAll.end();++iter)
				{
					AllCandidateSong.push_back(iter->first);
				}
				BALSnLSHCandidateCorrect(wavename,AllCandidateSong);

#endif	 

				lastTime=clock();
				OneSongLSHTime=(double)(lastTime-firstTimeTempLSH1)/CLOCKS_PER_SEC;
				totalLSHRetrievalPostProcessPitchTimeNote+=OneSongLSHTime;
				int sizeCandidates=SongNameMapToDataY.size();//所有候选歌曲名数量
				MinusMeanSmooth(queryPitchNote);////减均值操作，减均值后>12就-12，<-12就+12
				MinusMeanSmooth(queryStretchPitch);//减均值操作，减均值后>12就-12，<-12就+12

				int numLSH=0;//候选出现次数
				int thresholdMatch=0;//候选出现次数门限

				firstTimeTemp=clock();
				vector <pair<float,int>> DisLS;
				vector <float> DisLSOriginal;
				for (int i=0;i<sizeCandidates;i++)
				{//遍历所有候选歌曲
					if (LSHFilterMap.count(SongNameMapToDataY[i]))
					{//存在候选歌曲
						numLSH=LSHFilterMap[SongNameMapToDataY[i]];//得到候选歌曲出现的次数
						if (numLSH>thresholdMatch)//出现次数大于0
						{
							MinusMeanSmooth(CandidatesDataY[i]);
							vector <float> candidat;
							float pitch_query=0;
							int left=0;
							int right=0;
							//int Cedge=edge*(0.7+((float)i)/sizeCandidates);
							//queryStretchPitch伸缩后的序列 CandidatesDataY对应的音高序列
							//将候选序列进行左右缩短操作 比较与哼唱段距离 取距离最小值返回BALS
							float lsDis=CalculateOptimalEdge(queryStretchPitch,CandidatesDataY[i],left,right,edge*2,((float)i)/sizeCandidates);
							DisLSOriginal.push_back(lsDis);

							DisLS.push_back(make_pair(lsDis,i)); //将最小距离，与候选序号存入，得到所有候选的最小距离
						}
					}

				}
				stable_sort(DisLS.begin(),DisLS.end(),sortRule);//稳定排序，从小到大排序

				set<int> LSResult;
				int LSLeft=DisLS.size()*LSratio;//LSratio LS伸缩比例 =0.4
				if (LSLeft<20)
				{
					LSLeft=DisLS.size();
				}
				for (int i=0;i<LSLeft;i++)
				{

					LSResult.insert(DisLS[i].second);//存入前LSLeft个距离最短歌曲的候选序号

				}
				lastTimeTemp=clock();
				OneSongLSTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalLSTime+=OneSongLSTime;
				firstTimeTemp=clock();
				int disEmdNum=-1;
				lastTimeTemp=clock();
				OneSongEMDTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalEMDTime+=OneSongEMDTime;
				firstTimeTemp=clock();

				for (int i=0;i<sizeCandidates;i++)
				{//遍历所有候选歌曲
					numLSH=LSHFilterMap[SongNameMapToDataY[i]];//得到候选歌曲出现的次数
					if (numLSH>thresholdMatch)
					{
						disEmdNum++;//得到候选歌曲出现次数大于0的歌曲数目
					}

					if (query.n==0 || (LSHFilterMap.count(SongNameMapToDataY[i]) && LSResult.count(i)))
					{

						if (numLSH>thresholdMatch)
						{
							MinusMeanSmooth(CandidatesDataY[i]);
							int QuerySize=queryStretchPitch.size();
							int DataYSize=CandidatesDataY[i].size();
							int sizeQandD=CandidatesDataY[i].size();
							int ultimateNum=0;
							float distanceM=10000000;
							float distanceF=10000000;
							float distanceM1=10000000;
							float distanceM2=10000000;
							float distanceM3=10000000;

							if (0/*recur==10*/)
							{
								MeanPlus(CandidatesDataY[i],0);
								distanceM1=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
							}
							else
							{
								for (int k=0;k<5;k++)
								{//循环五次KTRA算法
									ultimateNum=0;
									float plus=(float(k-2))/2;
									MeanPlus(CandidatesDataY[i],plus);//减均值处理，取均值=mean+plus
									////KTRA计算最小距离
									distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
									distanceM1=MyMinTwo(distanceM1,distanceF);//取最小距离值
								}
							}
						
							tempDis1.push_back(distanceM1);//取最小距离值
							for (int k=0;k<0;k++)
							{//没有循环
								ultimateNum=0;
								float plus=(float(k-2.5))/2;
								MeanPlus(CandidatesDataY[i],plus);
								distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
								distanceM2=MyMinTwo(distanceM2,distanceF);
							}
							if (QuerySize<2)
							{
								QuerySize=2;
							}
							distanceM2=((float)DataYSize)/QuerySize;
							distanceM2=abs(distanceM2-1);
							tempDis2.push_back(distanceM2);
							ultimateNum=0;
							MeanPlus(CandidatesDataY[i],0);
							
							distanceM3=((float)DataYSize)/QuerySize;
							if (distanceM3>1)
							{
								distanceM3=distanceM3-1;
							}
							else
								distanceM3=(1-distanceM3)*2;
							tempDis3.push_back(distanceM3);
							
							distanceM=distanceM1*0.8+distanceM2*0.2;//按比例分配

							Dis.push_back(distanceM);//存入最短RA距离
							string songName=SongNameMapToDataY[i];

							if (songDis.count(distanceM))
							{//若已经存在进行增加0.00001，以表示区别
								*(Dis.end()-1)+=0.00001;
								songDis.insert(make_pair(distanceM+0.00001,songName)); 
							}
							else
								songDis.insert(make_pair(distanceM,songName)); //记录查询段与候选段最短距离 候选段歌曲名
							songName.erase(0,6);
							int posSong=songName.rfind("pv");
							songName.erase(posSong-1,3);//得到去掉路径及.pv的完整歌曲名
							tempList.push_back(songName);
						}
					}
				}

				lastTimeTemp=clock();
				OneSongRATime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalRATime+=OneSongRATime;
				CandidatesDTWAll+=Dis.size();
				string nameSong(wavename);

				string nameSongResult;
				stable_sort(Dis.begin(),Dis.end());//稳定排序 从小到大
				set <string> SongFindAlready;
				float ratio=0.0;
				string songNameFive;

				for (int i=0;i!=songDis.size()&&i<=1000 ;i++)
				{//最多循环1000次
					songNameFive=songDis.find(Dis[i])->second;
					if (SongFindAlready.count(songNameFive))
					{
						;
					}
					else
					{

						SongFindAlready.insert(songNameFive);//存入到已找到的歌曲列表中
					}
					if (SongFindAlready.size()==2)
					{//候选歌曲数为2  进行置信度计算

						if (Dis[0]/Dis[i]<=ratio)
						{//置信度计算  ratio=0.0;  即只能等于0  很难达到
							returnN=true;
							recur+=3;
						}
						i+=10000;//跳出循环
					}
				}
				if (returnN==true)
				{		
					nameSongResult.assign(nameSong,0,nameSong.size()-4);
					ResultDis1.insert(make_pair(nameSongResult,tempDis1));
					ResultDis2.insert(make_pair(nameSongResult,tempDis2));
					ResultDis3.insert(make_pair(nameSongResult,tempDis3));
					ResultLable1.insert(make_pair(nameSongResult,tempList));
				}
			}
		} //基于音符的检索的结尾，得到基于音符检索的候选歌曲，及距离

		lastTimeTempLSH1=clock();
		OneSonglshnoteTime=(double)(lastTimeTempLSH1-firstTimeTempLSH1)/CLOCKS_PER_SEC;
		totalLSHNoteTime+=OneSonglshnoteTime;

		if (recur==0 || recur==3)
		{
			lastTimeTempLSH1=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH1-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHNoteFirstTime+=OneSonglshPitchTime;
		}

		if (returnN==false)
		{//清空候选序列
			firstTimeTempLSH2=clock();
			IndexCandidatesStretch.clear();
			IndexCandidatesDis.clear();
			CandidatesNumStretch.clear();
			CandidatesFilter.clear();

#if 1 //这里为帧的LSH检索

			LinearCoe=0;
			FloorLevel=FloorLevelInitial;//当recur=0时 为1
			int LSHSizeDefine;
			LSHSizeDefine=LSHQueryVectorLinearStretching.size();

			int candidateSize;
			queryResultFile >> candidateSize;	//第一行为每个查询的候选数，即每行列数

			//samePointStream<<LSHSizeDefine<<endl;	//输出候选点
			
			
			
			for (;FloorLevel<UpperLimit+StretchStep && LinearCoe< LinearCoeTotal;FloorLevel+=StretchStep)
			{
				//遍历不同伸缩因子伸缩后的LSH点集 当recur=0时 只循环一次
				PPointT *QueriesArray=NULL;//要free
				
				if ((recur==1 || recur==2 )&& FloorLevel<=1.01 && FloorLevel>=0.99)
				{
					continue;
				}
				if (recur==2 && ((FloorLevel<=0.81 && FloorLevel>=0.79)||(FloorLevel<=1.21 && FloorLevel>=1.19)||(FloorLevel<=1.41 && FloorLevel>=1.39)))
				{
					continue;
				}

				//LSHQueryVectorLinearStretching，LS后的LSH点，每个点为一个音高序列，vector[i][j][k]表示第i个伸缩因子下的第j个采样点的第k个数据
				if (LSHQueryVectorLinearStretching[LinearCoe].size()>0)	//当前伸缩因子下伸缩后得到的点集
				{
					QueriesArray=readDataSetFromVector(LSHQueryVectorLinearStretching[LinearCoe]);	//读入LSH点集
					Int32T nPointsQuery=LSHQueryVectorLinearStretching[LinearCoe].size();	//得到点集大小
					IntT dimension=20;
					if (nPointsQuery>0)
					{
						dimension=LSHQueryVectorLinearStretching[LinearCoe][0].size();	//得到LSH的真实维数
					}
					IntT LSHFilterReturnNum=0;
					IntT *IndexArray=NULL;
					int IndexArraySize=500000;//最大查找候选数目

					IndexArray=(IntT *)MALLOC(IndexArraySize *sizeof(IntT));
					double *IndexArrayDis=(double *)MALLOC(IndexArraySize *sizeof(double));
					IntT *IndexFilterArray=NULL;
					IndexFilterArray=(IntT *)MALLOC(IndexArraySize/2 *sizeof(IntT));
					IntT * NumArray=(IntT *)MALLOC(nPointsQuery *sizeof(IntT));//每个点返回的候选数目

					if(!isUseDBNResult)
					{
						//得到LSH结果，即对每个LSH点进行RNN查询，返回RetainNum个候选，最后返回所有点的候选数目
						//输入：QueriesArray,当前LSH点集，nPointsQuery，当前点集大小
						//IndexArraySize，最大查找候选数
						//IndexHuming，LSH点的RNN索引，RetainNum，LSH每个点仅保留的点数
						//dimension，LSH点维数，LSHFilterNum，LSH滤波保留的点数
						//输出：
						//IndexArray，候选序号，NumArray，每个点返回的保留数目  IndexArrayDis，所有候选对应的距离
						//IndexFilterArray，所有候选序号，sizeFilter，候选大小
						IntT ResultSize=LSHStructToResultOnePointRetainSeveral(QueriesArray,nPointsQuery,IndexArraySize, 
							IndexArray,IndexHuming,NumArray,RetainNum,dimension,LSHFilterNum,IndexFilterArray,LSHFilterReturnNum,IndexArrayDis);//得到结果，每个点返回RetainNum个候选
					}
					else
					{
						if(queryResultFile.eof())	//到文件尾，错误
						{
							printf("DBN查询结果文件query_result.txt数据不足！\n");
							break;
						}
						for(int i=0; i<nPointsQuery*candidateSize; i++)	//读入当前查询点集的候选和距离
						{
							queryResultFile >> IndexArray[i] >> IndexArrayDis[i];
						}
						for(int i=0; i<nPointsQuery; i++)
						{
							NumArray[i] = candidateSize;
						}
					}

					vector<IntT> IndexCandidates;
					vector <float> DisCandidates;
					vector<IntT> CandidatesNum;//每个点返回的数目存入Vector
					int curreIndex=0;
					
					for (int i=0;i<LSHFilterReturnNum;i++)
					{
						//返回的每个点的所有候选序号均存入到CandidatesFilter中
						CandidatesFilter.push_back(IndexFilterArray[i]);
					}
					bool insertY=false;
					for (int i=0;i<nPointsQuery;i++)
					{//遍历当前LSH点集
						int numS=0;
						int siz=NumArray[i];//每个点保留的候选点数
						for (int j=0;j<siz;j++)
						{
							if (samePoint.count(IndexArray[j+curreIndex]))
							{
								//查看候选序号存在，返回0或1，测试结果显示，samepoint没有数值，基本无用
								numS=0;

								samePointIte=samePoint.find(IndexArray[j+curreIndex]);
								numS=samePointIte->second.size();
								IndexCandidates.push_back(IndexArray[j+curreIndex]);
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
								for (int k=0;k<samePointIte->second.size();k++)
								{
									insertY=true;
									for (int l=0;l<siz;l++)
									{
										if (samePointIte->second[k]==IndexArray[l+curreIndex])
										{
											insertY=false;
										}
									}
									if (insertY==true)
									{
										IndexCandidates.push_back(samePointIte->second[k]);
										DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
									}
									else
										numS--;
								}
								NumArray[i]+=numS;
							}
							else
							{//查看候选序号不存在，存入候选序号及距离
								IndexCandidates.push_back(IndexArray[j+curreIndex]);//存入候选序号
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);//存入候选对应的距离
							}
						}

						curreIndex+=siz;
						CandidatesNum.push_back(NumArray[i]);//每个点保留的候选数目
					}
					//准确率统计

					LSHCandidateCorrect(wavename,IndexLSH,IndexCandidates,CandidatesNum,allStretchCandidates,allCorrectCan);
				
					IndexCandidatesStretch.push_back(IndexCandidates);//存入当前点集的所有候选序号
					CandidatesNumStretch.push_back(CandidatesNum);//每个点保留的候选数目
					IndexCandidatesDis.push_back(DisCandidates);//存入候选对应的距离

					for (int i=0;i<nPointsQuery;i++)
					{//释放空间
						free(QueriesArray[i]->coordinates);
						free(QueriesArray[i]);
					}
					free(IndexArray);
					free(IndexFilterArray);
					free(NumArray);
					free(QueriesArray);
					free(IndexArrayDis);
				}
				else//伸缩后不存在点集
				{//同样存入 实则为空 不用进行检索哦
					vector <float> DisCandidates;
					vector<IntT> IndexCandidates;
					vector<IntT> CandidatesNum;//每个点返回的数目存入Vector
					IndexCandidatesStretch.push_back(IndexCandidates);//存入候选序号
					CandidatesNumStretch.push_back(CandidatesNum);//每个点保留的候选数目
					IndexCandidatesDis.push_back(DisCandidates);//存入候选对应的距离

				}
				LinearCoe++;

			}//遍历不同伸缩因子伸缩后的LSH点集 结束 得到候选点集  统计正确候选比率
			
#endif

			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
			totalLSHOnlyPitchTime+=OneSonglshPitchTime;


			for (int q=0;q<qRecurse;q++)
			{//再循环一次 去除伪候选
				int sizeLSH=IndexCandidatesStretch.size();//得到所有伸缩次数
				vector<float>  queryStretchPitch;//伸缩后的音高序列
				vector< vector<float> >  CandidatesDataY;//候选音高序列
				vector<float>  CandidatesDataYDis;//候选距离
				vector <string> SongNameMapToDataY;//候选歌曲名
				int CandidatesSizeInDWT=0;//返回的要精确匹配的数目
				map <string , short > SongMapPosition;

#if 1  //这里是frame的后处理
				for	(FloorLevel=FloorLevelInitial,LinearCoe=0;FloorLevel<UpperLimit+StretchStep && LinearCoe<sizeLSH &&LinearCoe< LinearCoeTotal;FloorLevel+=StretchStep)
				{
					map <string , vector<pair<short, double>> > SongMapPositionAll;
					map <string , vector<pair<short, double>> >::iterator iter;
					vector <string> AllCandidateSong;
					if ((recur==1 || recur==2 )&& FloorLevel<=1.01 && FloorLevel>=0.99)
					{
						continue;
					}
					if (recur==2 && ((FloorLevel<=0.81 && FloorLevel>=0.79)||(FloorLevel<=1.21 && FloorLevel>=1.19)||(FloorLevel<=1.41 && FloorLevel>=1.39)))
					{
						continue;
					}
					int StepFactorCurrent=int(stepFactor*stepRatio*FloorLevel/**FloorLevel*/);//伸缩后的帧移

					if (q==0)
					{
						int beforeSize=SongNameMapToDataY.size();
						//根据伸缩因子返回候选向量以及对应的歌曲名字
						////MatchBeginPos代表query和dataY相差点数必须在这个1/MatchBeginPos范围之内才进行匹配
						//CandidatesSizeInDWT返回要精确匹配的数目
						//IndexCandidatesStretch[sizeLSH-1],CandidatesNumStretch[sizeLSH-1] 候选序号及候选点数
						//FloorLevel初始伸缩值         IndexLSH //LSH索引，记录LSH点的序号，带路径的文件名，起始位置
						//queryPitch,wav由音高序列转换的一维音高序列，queryStretchPitch，伸缩后的序列
						//indexSongName 索引pv文件路径和对应的一维音高序列
						//输出 SongNameMapToDataY 候选歌曲名，dataY LSH点对应的候选音高序列 ，
						//SongMapPositionAll 存入候选歌曲信息，包括候选歌曲名，伸缩后对应的起始位置及伸缩因子。offsetbegin起始位置偏移量 
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,edge,edge*2); //此为全部匹配（不是从头开始唱）
						int afterSize=SongNameMapToDataY.size();

						//准确率统计
						for(int k=beforeSize;k<afterSize;k++)
						{
							AllCandidateSong.push_back(SongNameMapToDataY[k]);
						}
						
						BALSLSHCandidateCorrect(wavename,AllCandidateSong);
					}
					else if (q==3)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,6,0); //此为全部匹配（不是从头开始唱）
					}
					else if (q==4)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,-6,0); //此为全部匹配（不是从头开始唱）
					}
					else if (q==1)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,4,8); //此为全部匹配（不是从头开始唱）
					}
					else if (q==2)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,-4,-8); //此为全部匹配（不是从头开始唱）
					}
					LinearCoe++;
				}

				lastTimeTempLSH2=clock();
				OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
				totalLSHRetrievalPostProcessPitchTime+=OneSonglshPitchTime;

#endif	

#if 0 //这里为音符的扩展
				sizeLSH=IndexCandidatesStretch.size();
				map <string , vector<pair<short, double>> > SongMapPositionAll;
				IndexSignToQueryAndDataVectorHummingMatchLeastALLNote( posPair,IndexCandidatesStretch[sizeLSH-1],CandidatesNumStretch[sizeLSH-1],FloorLevel,
					IndexLSHNote,5,queryPitchNote,queryStretchPitch,CandidatesDataY,
					indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
					SongMapPositionAll,edge,edge*2);
#endif	 

				int sizeCandidates=SongNameMapToDataY.size();//获取候选歌曲大小
				MinusMeanSmooth(queryPitchNote);//减均值
				MinusMeanSmooth(queryStretchPitch);//减均值
				int numLSH=0;
				int thresholdMatch=0;

				firstTimeTemp=clock();
				vector <pair<float,int>> DisLS;
				vector <float> DisLSOriginal;
				for (int i=0;i<sizeCandidates;i++)
				{
					MinusMeanSmooth(CandidatesDataY[i]);
					vector <float> candidat;
					float pitch_query=0;
					int left=0;
					int right=0;
					//queryX伸缩后的音符序列 DataY对应的候选音符序列 length=12  调整候选边界 BALS 将候选序列进行左右缩短操作 比较与哼唱段距离 去距离最小值
					float lsDis=CalculateOptimalEdge(queryStretchPitch,CandidatesDataY[i],left,right,edge*2,((float)i)/sizeCandidates);
					DisLSOriginal.push_back(lsDis);//存入最小距离值
					DisLS.push_back(make_pair(lsDis,i)); //存入最小距离值及对应的候选音高序号
				}

				stable_sort(DisLS.begin(),DisLS.end(),sortRule);//稳定排序 由小到大

				set<int> LSResult;
				int LSLeft=DisLS.size()*LSratio;//LSratio=0.4  LS线性伸缩后留下的候选
				if (LSLeft<20)
				{
					LSLeft=DisLS.size();
				}
				for (int i=0;i<LSLeft;i++)
				{//取候选距离前LSLeft个最小候选
					LSResult.insert(DisLS[i].second);//存入对应的音高序号
				}

				lastTimeTemp=clock();
				OneSongLSTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalLSTime+=OneSongLSTime;
				firstTimeTemp=clock();
				int disEmdNum=-1;
				lastTimeTemp=clock();
				OneSongEMDTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalEMDTime+=OneSongEMDTime;
				firstTimeTemp=clock();

				for (int i=0;i<sizeCandidates;i++)
				{//遍历所有候选歌曲
					disEmdNum++;

					if ( LSResult.count(i))
					{//最短距离序列中存在候选歌曲对应的序号
						MinusMeanSmooth(CandidatesDataY[i]);//候选音高序列减均值
						int QuerySize=queryStretchPitch.size();//伸缩后的查询音高序列长度
						int DataYSize=CandidatesDataY[i].size();//候选音高序列长度
						int sizeQandD=CandidatesDataY[i].size();
						int ultimateNum=0;
						float distanceM=10000000;
						float distanceF=10000000;
						float distanceM1=10000000;
						float distanceM2=10000000;
						float distanceM3=10000000;

						if (0/*recur==10*/)
						{
							MeanPlus(CandidatesDataY[i],0);
							distanceM1=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
						}
						else
						{
							for (int k=0;k<5;k++)
							{
								ultimateNum=0;
								float plus=(float(k-2))/2;
								MeanPlus(CandidatesDataY[i],plus);//减均值处理，取均值=mean+plus
								////KTRA计算最小距离
								distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
								distanceM1=MyMinTwo(distanceM1,distanceF);//取最小距离值
							}
						}
						tempDis1.push_back(distanceM1);
						for (int k=0;k<0;k++)
						{
							ultimateNum=0;
							float plus=(float(k-2.5))/2;
							MeanPlus(CandidatesDataY[i],plus);
							distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
							distanceM2=MyMinTwo(distanceM2,distanceF);
						}
						if (QuerySize<2)
						{
							QuerySize=2;
						}
						distanceM2=((float)DataYSize)/QuerySize;//候选序列与查询序列伸缩比例
						distanceM2=abs(distanceM2-1);
						tempDis2.push_back(distanceM2);
						ultimateNum=0;
						MeanPlus(CandidatesDataY[i],0);

						distanceM3=((float)DataYSize)/QuerySize;
						if (distanceM3>1)
						{
							distanceM3=distanceM3-1;
						}
						else
							distanceM3=(1-distanceM3)*2;

						tempDis3.push_back(distanceM3);
						distanceM=distanceM1*0.8+distanceM2*0.2;//按比例计算最短距离

						Dis.push_back(distanceM);//存入最短距离
						string songName=SongNameMapToDataY[i];//得到候选歌曲名

						if (songDis.count(distanceM))
						{//若此最短距离已经存在 则增加0.00001，已示区别
							*(Dis.end()-1)+=0.00001;
							songDis.insert(make_pair(distanceM+0.00001,songName)); 
						}
						else
							songDis.insert(make_pair(distanceM,songName)); //存储最短RA距离及对应的歌曲名
						songName.erase(0,6);
						int posSong=songName.rfind("pv");
						songName.erase(posSong-1,3);
						tempList.push_back(songName);//得到去除路径及扩展名的歌曲名
					}
				}
				lastTimeTemp=clock();
				OneSongRATime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalRATime+=OneSongRATime;
				CandidatesDTWAll+=Dis.size();
				string nameSong(wavename);

				string nameSongResult;

				stable_sort(Dis.begin(),Dis.end());//对最短距离排序
				set <string> SongFindAlready;

				float ratio=110.00;
				if (recur==10)
				{
					ratio=1.59;
				}
				else if (recur==11)
				{
					ratio=1.47;
				}
				string songNameFive;
				float Thr[1000]={0.0};
				int num=0;
					
				for (int i=0;i!=songDis.size()&&i<=1000 ;i++)
				{//存入候选歌曲
					songNameFive=songDis.find(Dis[i])->second;//找到最短距离对应的歌曲名
					if (SongFindAlready.count(songNameFive))
					{//若找到的候选歌曲中已存在此歌曲 则不操作
						;
					}
					else
					{//不存在 则存入此候选歌曲
						num++;//存入的候选歌曲数量
						SongFindAlready.insert(songNameFive);
						if (SongFindAlready.size()==num && num<1000)
						{
							Thr[num-1]=Dis[i];
							float totalDis=0;
							for (int l=1;l<num;l++)
							{
								totalDis+=Thr[l];//所有距离和
							}
							if (SongFindAlready.size()==5)
							{//如果已找到的歌曲数量为5

								if ( totalDis/(Dis[0]*(num-1)) >=ratio)
								{//置信度计算 距离比率大于阈值110
									returnN=true;
									recur+=3;//表示跳出此查询循环，已找到目标歌曲
								}
								i+=10000;//跳出当前循环
							}
						}
					}
				}

				if (returnN==true || recur>=2)
				{		
					nameSongResult.assign(nameSong,0,nameSong.size()-4);
					ResultDis1.insert(make_pair(nameSongResult,tempDis1));
					ResultDis2.insert(make_pair(nameSongResult,tempDis2));
					ResultDis3.insert(make_pair(nameSongResult,tempDis3));
					ResultLable1.insert(make_pair(nameSongResult,tempList));
				}
			}

			if (recur==0 || recur==3)
			{
				lastTimeTempLSH2=clock();
				OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
				totalLSHPitchFirstTime+=OneSonglshPitchTime;
			}

			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
			totalLSHPitchTime+=OneSonglshPitchTime;
		}
		if (recur==0 || recur==3)
		{
			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHoneTime+=OneSonglshPitchTime;
		}
		else if (recur==1 || recur==4)
		{
			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHtwoTime+=OneSonglshPitchTime;
		}
		else if (recur==2 || recur==5)
		{
			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHthreeTime+=OneSonglshPitchTime;
		}
	}//for的recur循环结束
	vector<IntT>::iterator allStrCanBen;
	vector<IntT>::iterator allStrCanEnd;
	vector<IntT>::iterator allCorCanBen;
	vector<IntT>::iterator allCorCanEnd;
	allStrCanBen=allStretchCandidates.begin();
	allStrCanEnd=allStretchCandidates.end ();
	allCorCanBen=allCorrectCan.begin();
	allCorCanEnd=allCorrectCan.end();

	//正确率统计				
	int sumNum=0;
	int sumCorNum=0;
	float corCanRate=0;
	for(;allStrCanBen!=allStrCanEnd;++allStrCanBen)
	{
		
		sumNum+=*allStrCanBen;
	}
	for(;allCorCanBen!=allCorCanEnd;++allCorCanBen)
	{
		
		sumCorNum+=*allCorCanBen;
	}

	filename<<"所有候选："<<sumNum<<endl;
	filename<<"所有正确候选："<<sumCorNum<<endl;
	if(sumNum>0&&sumCorNum>0)
	{
		corCanRate=(float)sumCorNum/sumNum;
		exitCandidateNum++;
	}
	else
		corCanRate=0;
	filename<<"总正确率："<<corCanRate<<endl;
	filename<<"存在候选的数目："<<exitCandidateNum<<endl;
	filename.close();

	
	if(NULL!=NoteEmd){
		free (NoteEmd);
		NoteEmd=NULL;
		query.Features=NULL;
	}
	if(NULL!=NoteDuration){
		free (NoteDuration);
		NoteDuration=NULL;
		query.Weights=NULL;
	}
	if(NULL!=candiY.Features){
		free (candiY.Features);
		candiY.Features=NULL;
	}
	if(NULL!=candiY.Weights){
		free (candiY.Weights);
		candiY.Weights=NULL;
	}
	
	//if (total%500==0 || total >310)
	//{
	//	ofstream outTime("wav.result",ofstream::app);//LSH时间
	//	outTime<<" 全部歌曲RA时间："<<totalRATime<<"EMD时间："<<totalEMDTime<< "LS时间："<<totalLSTime<<endl;
	//	cout<<" 全部歌曲RA时间："<<totalRATime<<"EMD时间："<<totalEMDTime<< "LS时间："<<totalLSTime<<endl;
	//	outTime.close();
	//}

	stable_sort(Dis.begin(),Dis.end());
	string   songNameFive;
	ofstream outf("wav.result",ofstream::app);//识别结果保存文件
	ofstream outfCandidates("wavCandidates.result",ofstream::app);//候选结果保存文件

	static int num1=0;
	static int num2=0;
	static int num3=0;
	static int num4=0;
	static int num5=0;
	static int num10=0;
	static int num20=0;
	static int num50=0;
	static int findall=0;
	static float thresholdC=10;
	static float dis1=0;
	static float dis2=0;
	static string tempSt;
	
	static int rightNum[1101]={0};
	static int wrongNum[1101]={0};
	int findtop5=0;
	bool wrongA=TRUE;
	static map <string,int> resultMap;
	set <string> SongFindAlready;
	string firstSong;
	int findNum=0;	//控制输出的数目以及统计第几个找到的
	float ThrCal[1000]={0.0};
	int numCal=0;
	static float ThrRes[1000]={0.0};

	for (int i=0,k=0;i!=songDis.size()&&k<5 && i!= Dis.size() && k<500;i++)
	{
		if (songDis.count(Dis[i]))
		{
			songNameFive=songDis.find(Dis[i])->second;
		
			if (SongFindAlready.count(songNameFive))
			{
				;
			}
			else
			{
				k++;
			
				SongFindAlready.insert(songNameFive);
				int posSong=songNameFive.rfind("pv");
				songNameFive.erase(posSong-1,3);
				songNameFive.erase(0,6);
		
				songFive.push_back(songNameFive);//存入结果
			}
		}
	}
	while (songFive.size()<5)
	{
		songFive.push_back("NONE");
	}
	outf.close();
#endif	
	return 0;
}

//参数均为输出参数，输入为5000newpv355.txt 中的pv文件，建立索引（文件需在程序运行目录/5355P文件夹中）
//输出：param，参数信息
//songIDAndName，歌曲ID和歌名映射
int indexRead(ParamInfo *param,	map <string ,string> &songIDAndName)
{
	string fileName("wavall355.txt");	

	
	/*
	//多线程下分割任务
	int ThreadNum=1;
	if (ThreadNum>=100)
	{
		ThreadNum=100;//最多100个线程
	}
	OneFileToMultiFile(fileName,ThreadNum);	//根据线程数分割文件
	*/

	string dataIndex="5000newpv355.txt";	//pv列表文件，读入./5355P中的pv文件，建立索引
	vector<vector<float>> LSHVector;	//记录LSH点，每个点为一个音高序列

	//LSH点抽取参数
	int StepFactor = 3;	//选择LSH点的间隔数（间隔几个点抽取一个点）
	int LSHsize = 20;	//LSH窗长，一个LSH点的大小，20帧*3，2.4秒
	int LSHshift = StepFactor*5;	//LSH窗移,15帧，0.6秒
	int maxFrame = StepFactor*9*120*5;	//一维音高序列最大帧数

	//NLSH点抽取参数
	int noteMaxFrame = 10;	//一个音高的最长帧数（超过则切分）
	int NLSHsize = 10;	//NLSH窗长，一个LSH点的大小

	//更新帧和音符索引的参数列表
	strcpy(param[0].wavename,fileName.c_str());
	strcpy(param[1].wavename,fileName.c_str());
	param[0].RetainNum = 3;		//每个点仅保留的点数
	param[1].RetainNum = 5;	
	param[0].stepFactor = StepFactor;	//选择LSH点的间隔数
	param[1].stepFactor = StepFactor;
	param[0].LSHFilterNum = 10;	//LSH滤波保留的点数
	param[1].LSHFilterNum = 200;
	param[0].stepRatio = 1.5;	//查询的LSH点窗移
	param[1].stepRatio = 1.5;
	param[0].LSHsize = LSHsize;	//LSH点的维数
	param[1].LSHsize = NLSHsize;
	param[0].LSHshift = LSHshift;	//LSH点窗移
	param[1].LSHshift = 1;
	param[0].noteMinFrame = 3;	//音符最短帧数
	param[1].noteMinFrame = 3;
	param[0].noteMaxFrame = 10;	//音符最长帧数
	param[1].noteMaxFrame = 10;
	param[0].maxFrame = maxFrame;	//最大帧数
	param[1].maxFrame = maxFrame;

	//存储LSH点的中间变量
	PPointT *dataSet=NULL;	//存储LSH点，用于建立IndexHumming数据结构
	PPointT *dataSetNote=NULL;	//存储NLSH点，用于建立IndexHumming数据结构

	//读入pv文件，将二维的(音符，持续时间)序列转化为一维音符序列
	//输入：dataIndex: 索引音符列表文件
	//输出：indexSongName: 文件路径（5355P\\XXX.pv）和对应pv文件的一维音符序列
	readIndexPitch8MinutesNewPv(dataIndex, param[0].indexSongName);	
	
	//从一维音高序列抽取LSH点，去除了匹配开头的功能
	//输入：indexSongName，pv文件名和对应的一维音高序列
	//StepFactor，选择LSH点的间隔数，间隔几帧抽取一个点
	//LSHsize，LSH窗长，一个LSH点的大小
	//LSHshift，LSH窗移
	//maxFrame，一维音高序列最大帧数
	//输出：LSHVector，LSH点，每个点为一个音高序列，存储所有pv文件的LSH点
	//IndexLSH,	LSH索引，记录在LSHVector的序号，带路径的文件名，起始位置
	IndexPitchToLSHVector(param[0].indexSongName, StepFactor, LSHsize, LSHshift, maxFrame, 
		LSHVector, param[0].IndexLSH);
		
	//将LSH点写入文件
	//输入：LSHVector，LSH点，每个点为一个音高序列
	//filename，输出文件路径
	LSHVectorToFile(LSHVector,"LSHVector.txt");
	//将LSH索引写入文件
	//输入：IndexLSH，LSH索引，记录在LSHVector的序号，带路径的文件名，起始位置
	//filename，输出文件路径，后一个输出文件用于统计每个索引文件的数据数
	IndexLSHToFile(param[0].IndexLSH,"LSHIndex.txt","LSHCounter.txt");

	//将LSH点写入文件
	//输入：LSHVector，LSH点，每个点为一个音高序列
	//filename，输出文件路径
	LSHVectorToFile(LSHVector,"LSHVector.txt");
	//将LSH索引写入文件
	//输入：IndexLSH，LSH索引，记录在LSHVector的序号，带路径的文件名，起始位置
	//filename，输出文件路径，后一个输出文件用于统计每个索引文件的数据数
	IndexLSHToFile(param[0].IndexLSH,"LSHIndex.txt","LSHCounter.txt");

	//从LSH向量索引中读数据集，读入dataSet中，读入每个LSH点时记录序号和LSH点的平方和
	dataSet = readDataSetFromVector(LSHVector);

	//调用MIT的r-NN开源代码所需参数
	Int32T nPointsData = LSHVector.size();	//LSH点集大小
	IntT dimension = LSHsize;	//每个点维数，即LSH窗长
	IntT nSampleQueries = 100;	//测试样本点数
	float thresholdR = 18;		//门限
	MemVarT memoryUpperBound = 600000000;	//内存上限

	if (nPointsData>0)
	{
		dimension = LSHVector[0].size();	//记录真实维数
	}
	LSHVector.clear();	//清除该中间变量，数据已读入dataSet了

	//调用MIT的r-NN开源代码初始化LSH数据结构
	//输入：dataSet：LSH点集
	//dimension：每个LSH点维数
	//nPointsData：LSH点集大小
	//nSampleQueries：测试样本点数
	//thresholdR：门限
	//memoryUpperBound：内存上限
	//输出：IndexHuming，RNN索引
	LSHDataStruct(dataSet, dimension, nPointsData, nSampleQueries,
		thresholdR, memoryUpperBound, param[0].IndexHuming);
	
	//从一维音高序列抽取基于音符的LSH点
	//输入：indexSongName，pv文件名和对应的一维音高序列
	//noteMaxFrame，一个音高的最长帧数（超过则切分）
	//NLSHsize，LSH窗长，一个LSH点的大小
	//maxFrame，一维音高序列最大帧数
	//输出：LSHVector，LSH点，每个点为一个10维的音高序列
	//IndexLSH,	NLSH索引，记录在LSHVector的序号，带路径的文件名，起始位置，持续帧数
	IndexPitchToLSHVectorNote(param[0].indexSongName, noteMaxFrame, NLSHsize, maxFrame, 
		LSHVector, param[1].IndexLSHNote);
		
	//将LSH点写入文件
	//输入：LSHVector，LSH点，每个点为一个音高序列
	//filename，输出文件路径
	LSHVectorToFile(LSHVector,"NLSHVector.txt");
	//将NLSH索引写入文件
	//输入：IndexLSH，NLSH索引，记录在LSHVector的序号，带路径的文件名，起始位置，持续帧数
	//filename，输出文件路径，后一个输出文件用于统计每个索引文件的数据数
	IndexLSHNoteToFile(param[1].IndexLSHNote,"NLSHIndex.txt","NLSHCounter.txt");

	//将LSH点写入文件
	//输入：LSHVector，LSH点，每个点为一个音高序列
	//filename，输出文件路径
	LSHVectorToFile(LSHVector,"NLSHVector.txt");
	//将NLSH索引写入文件
	//输入：IndexLSH，NLSH索引，记录在LSHVector的序号，带路径的文件名，起始位置，持续帧数
	//filename，输出文件路径，后一个输出文件用于统计每个索引文件的数据数
	IndexLSHNoteToFile(param[1].IndexLSHNote,"NLSHIndex.txt","NLSHCounter.txt");

	//从NLSH向量索引中读数据集，读入dataSetNote中，读入每个LSH点时记录序号和LSH点的平方和
	dataSetNote=readDataSetFromVector(LSHVector);

	//调用MIT的r-NN开源代码所需参数
	nPointsData = LSHVector.size();	//NLSH点集大小
	dimension = NLSHsize;	//每个点维数，即NLSH窗长
	nSampleQueries = 100;	//测试样本点数
	thresholdR = 7;	//门限
	memoryUpperBound = 600000000;	//内存上限

	if (nPointsData>0)
	{
		dimension = LSHVector[0].size();	//记录真实维数
	}
	LSHVector.clear();	//清除该中间变量，数据已读入dataSet了
	samePoint.clear();

	//调用MIT的r-NN开源代码初始化LSH数据结构
	//输入：dataSetNote：NLSH点集
	//dimension：每个LSH点维数
	//nPointsData：LSH点集大小
	//nSampleQueries：测试样本点数
	//thresholdR：门限
	//memoryUpperBound：内存上限
	//输出：IndexHuming
	LSHDataStruct(dataSetNote, dimension, nPointsData, nSampleQueries,
		thresholdR, memoryUpperBound, param[1].IndexHuming);

	string IDAndNameFile = "name.txt";
	songread(IDAndNameFile,songIDAndName);	//从文件读入歌曲ID和歌名映射表,文件每行为：ID 歌名

	/*
	//用param初始化输入参数LSH索引
	FAILIF(NULL == (IndexHumingLocal = (PRNearNeighborStructT)MALLOC(sizeof(RNearNeighborStructT))));
	PRNearNeighborStructTCopy(IndexHumingLocal,param[0].IndexHuming);
	FAILIF(NULL == (IndexHumingLocalNote = (PRNearNeighborStructT)MALLOC(sizeof(RNearNeighborStructT))));
	PRNearNeighborStructTCopy(IndexHumingLocalNote,param[1].IndexHuming);
	*/
	return 1;
}

void PRNearNeighborStructTCopy(PRNearNeighborStructT des,PRNearNeighborStructT src)	//RNN数据结构复制
{
	des->dimension=src->dimension;
	des->hashedBuckets=src->hashedBuckets;
	des->hfTuplesLength=src->hfTuplesLength;
	des->lshFunctions=src->lshFunctions;
	des->sizeMarkedPoints=src->sizeMarkedPoints;
	FAILIF(NULL == (des->markedPoints = (BooleanT*)MALLOC(des->sizeMarkedPoints * sizeof(BooleanT))));
	for(IntT i = 0; i < des->sizeMarkedPoints; i++)
	{
		des->markedPoints[i] = FALSE;
	}
	FAILIF(NULL == (des->markedPointsIndeces = (Int32T*)MALLOC(des->sizeMarkedPoints * sizeof(Int32T))));
	des->nHFTuples=src->nHFTuples;
	des->nPoints=src->nPoints;
	des->parameterK=src->parameterK;
	des->parameterL=src->parameterL;
	des->parameterR=src->parameterR;
	des->parameterR2=src->parameterR2;
	des->parameterT=src->parameterT;
	des->parameterW=src->parameterW;
	des->points=src->points;
	des->pointsArraySize=src->pointsArraySize;
	FAILIF(NULL == (des->pointULSHVectors = (Uns32T**)MALLOC(des->nHFTuples * sizeof(Uns32T*))));
	for(IntT i = 0; i < des->nHFTuples; i++)
	{
		FAILIF(NULL == (des->pointULSHVectors[i] = (Uns32T*)MALLOC(des->hfTuplesLength * sizeof(Uns32T))));
	}
	FAILIF(NULL == (des->precomputedHashesOfULSHs = (Uns32T**)MALLOC(des->nHFTuples * sizeof(Uns32T*))));
	for(IntT i = 0; i < des->nHFTuples; i++)
	{
		FAILIF(NULL == (des->precomputedHashesOfULSHs[i] = (Uns32T*)MALLOC(N_PRECOMPUTED_HASHES_NEEDED * sizeof(Uns32T))));
	}
	FAILIF(NULL == (des->reducedPoint = (float*)MALLOC(des->dimension * sizeof(float))));
	des->reportingResult=src->reportingResult;
	des->useUfunctions=src->useUfunctions;
}


#include<windows.h>
#include<tlhelp32.h>
#include<iostream>
using namespace std;

int CreateNewProcess(LPCSTR pszExeName,PROCESS_INFORMATION &piProcInfoGPS)
{
	STARTUPINFO siStartupInfo;
	SECURITY_ATTRIBUTES saProcess, saThread;
	ZeroMemory( &siStartupInfo, sizeof(siStartupInfo) );
	siStartupInfo.cb = sizeof(siStartupInfo);
	saProcess.nLength = sizeof(saProcess);
	saProcess.lpSecurityDescriptor = NULL;
	saProcess.bInheritHandle = true;
	saThread.nLength = sizeof(saThread);
	saThread.lpSecurityDescriptor = NULL;
	saThread.bInheritHandle = true;
	return ::CreateProcess( NULL, (LPTSTR)pszExeName, &saProcess,
		&saThread, false,
		CREATE_NO_WINDOW, NULL, NULL,
		&siStartupInfo, &piProcInfoGPS );
}


void main()
{
	ParamInfo param[2];		//参数信息
	map <string ,string> songIDAndName;		//歌曲ID和歌名映射表
	vector<string> songFive;	//存储返回的检索结果

	
	//参数均为输出参数，输入为5000newpv355.txt 中的pv文件，建立索引（文件需在./5355P文件夹中）
	indexRead(param,songIDAndName);

	string fileName("query.txt");	//文件每行是一个哼唱wav的路径，即查询集
	ifstream pitchFile(fileName.c_str());

	string OutName("result.txt");	//存放结果的文件
	ofstream resultFile(OutName.c_str());

	string pitchname;	//wav路径
	queryResultFile.open("query_result.txt");	//打开DBN查询结果文件，每行一个查询结果，为候选序号和对应距离，按距离从小到大排序

	//查询信息文件，因为这个三个文件为追加打开，重写前需删除
	string cmd1 = "del "+query_vector_fname;
	string cmd2 = "del "+query_index_fname;
	string cmd3 = "del "+query_counter_fname;
	system(cmd1.c_str());
	system(cmd2.c_str());
	system(cmd3.c_str());

	double top1Accuracy = 0;	//top1正确率
	double top5Accuracy = 0;	//top5正确率
	double wavCount = 0;	//查询wav数

	while(getline(pitchFile,pitchname))	//读取每行的wav路径
	{
		char filename[300];		//哼唱wav文件名（含路径）
		strcpy(filename,pitchname.c_str());

		//查询函数：
		//输入：filename,查询文件路径，param,参数信息
		//isWriteLSHQueryToFile,是否将LS后的LSH查询数据及索引写入文件
		//isUseDBNResult，是否用DBN查询结果
		//输出：songFive，查询结果
		WavToSongFive(filename,param,songFive,true,false);

		//输出结果
		resultFile<<"query:"<<pitchname<<endl;

		//提取哼唱wav文件的文件名
		int pos = pitchname.find_last_of('\\');
		string queryPureName = pitchname.substr(pos+1,pitchname.length()-pos-5);	//取最后一个'\'和".wav"之间的文件名

		for (int j=0; j<5 && j<=songFive.size(); j++)
		{
			resultFile<<songFive[j]<<endl;
			if(queryPureName == songFive[j])
			{
				top5Accuracy++;
				if(j==0) top1Accuracy++;
			}
		}
		resultFile<<endl;

		wavCount++;
	}
	top1Accuracy /= wavCount;
	top5Accuracy /= wavCount;
	resultFile << "Top1 正确率：" << top1Accuracy*100 << " %" << endl;
	resultFile << "Top5 正确率：" << top5Accuracy*100 << " %" << endl;
	
	queryResultFile.close();

	pitchFile.close();	//关闭查询文件流
	resultFile.close();	//关闭结果文件流
}