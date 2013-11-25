#include "index.h"
extern map<int,vector<int>> samePoint;
extern map<string,int> ListSong;


int readindex(string dataIndex,map<string , vector<vector<double>>> &indexSongName)
{
	ifstream indexFile(dataIndex.c_str());
	vector <double> pitchNum;
	vector<vector<double>> database;
	ifstream songPitch;
	string songName,songPitchNum;
	string::size_type pos=0;
	while (indexFile>>songName)
	{
		songPitch.open(songName.c_str());
		while (getline(songPitch,songPitchNum))
		{
			if (songPitchNum.find("value")!=string::npos)
			{
				pitchNum.clear();
				pos=songPitchNum.find("value");
				songPitchNum.erase(0,pos+8);
				pitchNum.push_back(atof(songPitchNum.c_str()));
				database.push_back(pitchNum);

			}
        }
		indexSongName.insert(make_pair(songName,database));
		database.clear();
		songPitch.close();
		songPitch.clear();
	}
	indexFile.close();
	return 0;
}

int IndexSignToQueryAndDataVectorHummingMatchLeastALLNote(vector<pair<short, short>>& posPair,vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
													  map<unsigned long , pair<string,pair<short,short>>> &IndexLSH,int stepFactor,vector <float> queryPitch,
													  vector<float>  &queryX, vector< vector<float> > &dataY,
													  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
													  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
													  map <string , vector<pair<short, double>> >  &SongMapPositionAll,int offsetbegin, int offsetLength)//根据伸缩因子返回候选向量以及对应的歌曲名字
{  
	//匹配最少数目，即精确匹配的数目最少
	int posBegin=100;//小于1/posBegin的保留
	queryX.clear();
	StringToString(queryPitch,queryX,1);
	int sizePoints=CandidatesNumStretch.size();
	int sizeCandidates=0;
	int QueryLSHPos=0;
	int QueryLSHLength=0;
	float DataYLength=0;
	float ratioLength=0;
	int accuSizeQuery=0;
	int DifferPos=0;
	int sizeQuery=queryX.size();
	int queryLogicPos=0;
	pair<string, pair<short ,short>> SongNamePos;
	map<string , vector<float>> :: iterator SongTone;
	vector<float>  DataYOnePoint;
	vector <string> :: iterator SongIter;
	vector< vector<float> >  :: iterator SongPitchIter;
	map <string , vector<pair<short, double>> > :: iterator songPosIter;
	vector<pair<short, double>>  :: iterator PaiCIter;

	for (int i=0;i<sizePoints;i++)
	{
		int sizeCandidatesCurrent=sizeCandidates;
		int CurrentNum=CandidatesNumStretch[i];
		for (int j=sizeCandidatesCurrent;j<sizeCandidatesCurrent+CurrentNum;j++)
		{
			sizeCandidates++;

			if (IndexLSH.count(IndexCandidatesStretch[j]))
			{
				QueryLSHPos=posPair[i].first;
				QueryLSHLength=posPair[i].second;
				SongNamePos=IndexLSH.find(IndexCandidatesStretch[j])->second;
				DataYLength=SongNamePos.second.second;
				ratioLength=DataYLength/QueryLSHLength;
				queryLogicPos=DataYLength*QueryLSHPos/QueryLSHLength;
				DifferPos=SongNamePos.second.first-queryLogicPos-offsetbegin;
				accuSizeQuery=sizeQuery/ratioLength+offsetLength;
				if (indexSongName.count(SongNamePos.first) && ratioLength>0.5 && ratioLength<2)
				{
					DataYOnePoint.clear();
					SongTone=indexSongName.find(SongNamePos.first);
					int sizeSong=SongTone->second.size();
					if (DifferPos>=0  &&DifferPos+accuSizeQuery*3.0/4.0<sizeSong)//保证长度不是最末尾部分,最多错query一半
					{
						for (int k=DifferPos;k<DifferPos+accuSizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//将此点复制过去
						}
						if (SongMapPositionAll.count(SongNamePos.first))
						{
							int booW=1;
							songPosIter=SongMapPositionAll.find(SongNamePos.first);
							for (int i=0;i<songPosIter->second.size();i++)
							{
								if (abs(songPosIter->second[i].first-DifferPos)<sizeQuery/posBegin && 
									abs(songPosIter->second[i].second-accuSizeQuery)<6)
								{
									booW=0;
								}
							}
							if (booW==1)
							{
								dataY.push_back(DataYOnePoint);
								SongNameMapToDataY.push_back(SongNamePos.first);
								SongMapPositionAll[SongNamePos.first].push_back(make_pair(DifferPos,accuSizeQuery));
								CandidatesSizeInDWT++;
							}
						}
						else
						{
							vector<pair<short, double>> temp;
							temp.push_back(make_pair(DifferPos,accuSizeQuery));
							SongMapPositionAll.insert(make_pair(SongNamePos.first,accuSizeQuery));
							dataY.push_back(DataYOnePoint);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;
						}
					}
					else if (DifferPos<0 && DifferPos >-(sizeQuery/3) &&  DifferPos+sizeQuery<sizeSong)
					{
						DifferPos=0;
						for (int k=0;k<DifferPos+accuSizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//将此点复制过去
						}
						if (SongMapPositionAll.count(SongNamePos.first))
						{
							int booW=1;
							songPosIter=SongMapPositionAll.find(SongNamePos.first);
							for (int i=0;i<songPosIter->second.size();i++)
							{
								if (abs(songPosIter->second[i].first-DifferPos)<sizeQuery/posBegin && 
									abs(songPosIter->second[i].second-accuSizeQuery)<4)
								{
									booW=0;
								}
							}
							if (booW==1)//如果比列表中的位置靠前
							{
								dataY.push_back(DataYOnePoint);
								SongNameMapToDataY.push_back(SongNamePos.first);
								SongMapPositionAll[SongNamePos.first].push_back(make_pair(DifferPos,accuSizeQuery));
								CandidatesSizeInDWT++;
							}
						}
						else
						{
							vector<pair<short, double>> temp;
							temp.push_back(make_pair(DifferPos,accuSizeQuery));
							SongMapPositionAll.insert(make_pair(SongNamePos.first,temp));
							dataY.push_back(DataYOnePoint);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;
						}
					}
					else
						CandidatesNumStretch[i]--;//此矩阵中存储实际精确匹配的数目
				}
				else
					CandidatesNumStretch[i]--;
			}
			else
				CandidatesNumStretch[i]--;
		}
	}
	return 0;
}


int IndexSignToQueryAndDataVectorHummingMatchLeastALL(vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
													  map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,vector <float> queryPitch,
													  vector<float>  &queryX, vector< vector<float> > &dataY,
													  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
													  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
													  map <string , vector<pair<short, double>> >  &SongMapPositionAll,int offsetbegin, int offsetLength)//根据伸缩因子返回候选向量以及对应的歌曲名字
{  
	//匹配最少数目，即精确匹配的数目最少
	int posBegin=100;//小于1/posBegin的保留
	queryX.clear();
	StringToString(queryPitch,queryX,1);
	int sizePoints=CandidatesNumStretch.size();
	int sizeCandidates=0;
	int QueryLSHPos=0;
	int DifferPos=0;
	int sizeQuery=queryX.size();

	sizeQuery=(sizeQuery*stretchFactor+offsetLength);

	pair<string, short> SongNamePos;
	map<string , vector<float>> :: iterator SongTone;
	vector<float>  DataYOnePoint;
	vector <string> :: iterator SongIter;
	vector< vector<float> >  :: iterator SongPitchIter;
	map <string , vector<pair<short, double>> > :: iterator songPosIter;
	vector<pair<short, double>>  :: iterator PaiCIter;

	for (int i=0;i<sizePoints;i++)
	{
		int sizeCandidatesCurrent=sizeCandidates;
		int CurrentNum=CandidatesNumStretch[i];
		for (int j=sizeCandidatesCurrent;j<sizeCandidatesCurrent+CurrentNum;j++)
		{
			sizeCandidates++;

			if (IndexLSH.count(IndexCandidatesStretch[j]))
			{
				QueryLSHPos=i*stepFactor;
				SongNamePos=IndexLSH.find(IndexCandidatesStretch[j])->second;
				DifferPos=SongNamePos.second-QueryLSHPos-offsetbegin;
				if (indexSongName.count(SongNamePos.first))
				{
					DataYOnePoint.clear();
					SongTone=indexSongName.find(SongNamePos.first);
					int sizeSong=SongTone->second.size();
					if (DifferPos>=0  &&DifferPos+sizeQuery*3.0/4.0<sizeSong)//保证长度不是最末尾部分,最多错query一半
					{
						for (int k=DifferPos;k<DifferPos+sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//将此点复制过去

						}
						if (SongMapPositionAll.count(SongNamePos.first))
						{
							int booW=1;
							songPosIter=SongMapPositionAll.find(SongNamePos.first);
							for (int i=0;i<songPosIter->second.size();i++)
							{
								if (abs(songPosIter->second[i].first-DifferPos)<sizeQuery/posBegin && 
									abs(songPosIter->second[i].second-stretchFactor)<0.1)
								{
									booW=0;
								}
							}
							if (booW==1)
							{
								dataY.push_back(DataYOnePoint);
								SongNameMapToDataY.push_back(SongNamePos.first);
								SongMapPositionAll[SongNamePos.first].push_back(make_pair(DifferPos,stretchFactor));
								CandidatesSizeInDWT++;

							}
						}
						else
						{
							vector<pair<short, double>> temp;
							temp.push_back(make_pair(DifferPos,stretchFactor));
							SongMapPositionAll.insert(make_pair(SongNamePos.first,temp));
							dataY.push_back(DataYOnePoint);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;

						}
					}
					else if (DifferPos<0 && DifferPos >-(sizeQuery/3) &&  DifferPos+sizeQuery<sizeSong)
					{
						DifferPos=0;
						for (int k=0;k<DifferPos+sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//将此点复制过去

						}
						if (SongMapPositionAll.count(SongNamePos.first))
						{
							int booW=1;
							songPosIter=SongMapPositionAll.find(SongNamePos.first);
							for (int i=0;i<songPosIter->second.size();i++)
							{
								if (abs(songPosIter->second[i].first-DifferPos)<sizeQuery/posBegin && 
									abs(songPosIter->second[i].second-stretchFactor)<0.05)
								{
									booW=0;
								}
							}
							if (booW==1)//如果比列表中的位置靠前
							{
								dataY.push_back(DataYOnePoint);
								SongNameMapToDataY.push_back(SongNamePos.first);
								SongMapPositionAll[SongNamePos.first].push_back(make_pair(DifferPos,stretchFactor));
								CandidatesSizeInDWT++;

							}
						}
						else
						{
							vector<pair<short, double>> temp;
							temp.push_back(make_pair(DifferPos,stretchFactor));
							SongMapPositionAll.insert(make_pair(SongNamePos.first,temp));
							dataY.push_back(DataYOnePoint);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;
						}
					}
					else
						CandidatesNumStretch[i]--;//此矩阵中存储实际精确匹配的数目
				}
				else
					CandidatesNumStretch[i]--;
			}
			else
				CandidatesNumStretch[i]--;
		}
	}
	return 0;

}


//读入pv文件，将二维的(音符，持续时间)序列转化为一维音符序列
//输入：dataIndex: 索引音符列表文件
//输出：indexSongName: 文件路径（5355P\\XXX.pv）和对应pv文件的一维音符序列
int readIndexPitch8MinutesNewPv(string dataIndex, map<string , vector<float>> &indexSongName)
{
	int frames_per_sec = 25;	//每秒的帧数
	int max_totalMidi = 50000;	//最多读入的pv文件数
	double max_time_midi = 600;	//每个pv最多读入时长(s)
	int max_mean_thd = frames_per_sec*max_time_midi;	//减均值操作时算均值的最大帧数

	ifstream indexFile(dataIndex.c_str());	//输入pv列表文件流

	string songName;	//pv文件名
	int totalMidi=0;	//pv文件计数

	while (indexFile>>songName)	//读入一个pv文件名
	{
		totalMidi++;	//计数

		if (totalMidi == max_totalMidi)	//控制最多读入的pv文件数
		{
			break;
		}

		songName = "5355P\\" + songName;	//音符文件(*.pv)路径
		
		locale::global(locale(""));		//设定全局 locale 为环境设置的locale,解决std::ofstream.open()中文路径文件名问题
		
		ifstream songPitch;
		songPitch.open(songName.c_str());	//打开当前pv文件流

		locale::global(locale("C"));	//重新将全局locale设定为C locale

		double time_midi=0;	//记录每个pv有效音符时长
		string songPitchNum,songPitchBegin,songPitchDuration;	//接收（音符，起始时间，持续时间）的字符串
		float pitchNum=0;	//当前音符
		float pitchPre=0;	//前一个音符
		float duration=0;	//音符持续时间
		int pitchAmount=0;	//音符持续帧数

		vector<float> database;	//存储该pv文件的一维音高序列

		//读取一个pv文件，读入有效音符总时长不超过360s，6min
		//pv文件格式：音符 起始时间(s) 持续时间(s)
		while (time_midi < max_time_midi && 
			songPitch>>songPitchNum && songPitch>>songPitchBegin && songPitch>>songPitchDuration )
		{
			pitchNum = atof(songPitchNum.c_str());	//音符
			duration = atof(songPitchDuration.c_str());	//持续时间
			pitchAmount = duration * frames_per_sec;	//音符持续的连续帧数，25帧/s
			//cout<<pitchNum<<" "<<duration<<" "<<pitchAmount<<endl;

			if(duration<5 && pitchAmount>=1)	//有效音符长度小于5s，大于1帧，即40ms
			{
				
				if (pitchNum != pitchPre && pitchAmount<3)	//和前一个音符不同且长度小于3帧，不处理
				{
					;
				}
				else	//和前一个音符相同或此音符持续长度大于3帧
				{
					pitchPre = pitchNum;
					time_midi += duration;	//记录总时长
					for (int i=0;i<pitchAmount;i++)	//将二维音符转化为一维音高序列，存入database
						database.push_back(pitchNum);
				}
			}
		}
		
		if (totalMidi%10==0)	//每10首歌输出一次音高序列点数
		{
			cout<<"pv文件(序号，文件名)："<<totalMidi<<","<<songName<<endl;
			cout<<"该文件一维音高序列帧数:"<<database.size()<<endl;
		}

		NoZero(database);	//去除序列中的零值
		smooth(database);	//用五点中值法平滑，窗长为5，窗移为1
		MinusMeanWithThd(database,max_mean_thd);		//减均值操作。最多计算前max_mean_thd帧的均值

		indexSongName.insert(make_pair(songName,database));	//将pv文件名和一维音高序列插入输出变量

		database.clear();	//清空当前pv文件的一维音高序列
		songPitch.close();	//关闭当前pv文件
		songPitch.clear();	//清空文件流
	}
	indexFile.close();	//关闭pv列表文件

	return 0;
}

int charToVector(char * wavename,vector <float> &queryPitch)
{
	int  n=win;
	unsigned long DataLength=LONGPOINT;	//音频中的采样点总数
	int *pitchnum;
	double pitchNum=0;
	float energy=0;
	int BeginFrame[]={0};
	DataLength=pitchana(wavename,&pitchnum,BeginFrame);
	int leng_i=(DataLength-FLENGTH)/FSHIFTW+1;
	for (int i=0;i<leng_i;i++)
	{
		pitchNum=*(pitchnum+i);
		queryPitch.push_back(pitchNum);
	}
	free(pitchnum);

	return *BeginFrame;
}

//五点中值
float MiddleFive(float a, float b, float c,float d,float e)
{
	vector <float> num;
	num.push_back(a);
	num.push_back(b);
	num.push_back(c);
	num.push_back(d);
	num.push_back(e);
	vector <float>::iterator ite_begin=num.begin();
	vector <float>::iterator ite_end=num.end();
	stable_sort(ite_begin,ite_end);
	ite_begin=num.begin();

	return *(ite_begin+2);
}

//用五点中值法平滑，窗长为5，窗移为1
void smooth(vector <float> &queryPitch)
{
	vector <float> queryPitchDuplicate(queryPitch);
	int m=queryPitch.size();
	int i;
	for (i=2;i<m-2;i++)
	{
		queryPitch[i] = MiddleFive(queryPitchDuplicate[i-2],queryPitchDuplicate[i-1],
			queryPitchDuplicate[i],queryPitchDuplicate[i+1],queryPitchDuplicate[i+2]);
	}
}

void VectorSmoothToHalf(vector <float> &queryPitch)
{
	int m=queryPitch.size();
	int n=0;
	int i,j;
	vector <float> queryPitchHalf;
	for (i=0;i<m-1;i+=2)
	{
		queryPitchHalf.push_back(queryPitch[i]);
	}
	queryPitch.clear();
	n=queryPitchHalf.size();
	for (i=0;i<n;i++)
	{
		queryPitch.push_back(queryPitchHalf[i]);
	}

}

//从一维音高序列抽取基于音符的LSH点
//输入：indexSongName，pv文件名和对应的一维音高序列
//noteMaxFrame，一个音高的最长帧数（超过则切分）
//NLSHsize，LSH窗长，一个LSH点的大小
//maxFrame，一维音高序列最大帧数
//输出：LSHVector，LSH点，每个点为一个10维的音高序列
//IndexLSH,	LSH索引，记录在LSHVector的序号，每个LSH点的起始位置，持续帧数
int IndexPitchToLSHVectorNote(map<string , vector<float>> &indexSongName, 
	int noteMaxFrame, int NLSHsize, int maxFrame, 
	vector<vector<float>> &LSHVector, map<unsigned long , pair<string,pair<short,short>>> &IndexLSH)
{
	map<string , vector<float>>::iterator indexIte;	//遍历pv音高序列的迭代器
	unsigned long pointNum=0;	//抽取LSH点计数
	typedef map<unsigned long , pair<string,pair<short,short>>>::value_type IndexType;	//LSH索引格式

	//遍历所有pv文件
	for (indexIte = indexSongName.begin(); indexIte!=indexSongName.end(); indexIte++)
	{
		map<string , vector<float>>::value_type SongTone;

		string SongName(indexIte->first);	//当前pv文件名
		int sizeTone=indexIte->second.size();	//当前一维音高序列点数

		//遍历当前pv文件的一维音高序列
		for (int pos=0; pos<sizeTone && pos<maxFrame; )
		{
			float currentTone=1000;	//存当前音高
			short frameNow = 0;		//当前LSH点持续帧数
			int oneNoteDuration = 0;	//第一个音符持续的帧数
			float TheFirstNote = indexIte->second[pos];	//第一个音高
			
			vector <float> temp;	//当前LSH点用的帧
			vector <float> LSHVectorDimention;	//存储当前的LSH点

			int CuNoteNum=0;	//当前音高持续帧数

			//从pos位置开始抽取一个LSH点
			for (int i=pos; i<sizeTone; i++)
			{
				frameNow++;		//当前LSH点持续帧数
				temp.push_back(indexIte->second[i]);	//当前LSH点用的帧

				if (TheFirstNote==indexIte->second[i])	//记录第一个音高持续帧数
					oneNoteDuration++;
				else	//第一个音高结束，将TheFirstNote置为最大值防止oneNoteDuration继续累加
					TheFirstNote=1000;

				//当前LSH点维数达上限，且当前音高和前一个不同，当前LSH点抽取完毕
				if (LSHVectorDimention.size()==NLSHsize && currentTone !=indexIte->second[i])
				{
					break;
				}

				if (currentTone !=indexIte->second[i])	//当前音高和前一个不同
				{
					CuNoteNum=0;
					currentTone=indexIte->second[i];	//记录当前音高
					LSHVectorDimention.push_back(indexIte->second[i]);	//当前音高录入LSH点末尾
				}
				else	//和前一个音高相同
					CuNoteNum++;	//当前音高持续帧数计数

				//当前LSH点维数达上限，且当前音高持续帧数超过最长帧，当前LSH点抽取完毕
				if (LSHVectorDimention.size()==NLSHsize && CuNoteNum>=noteMaxFrame)
				{
					break;
				}
				if (CuNoteNum>=noteMaxFrame)	//当前音高持续帧数超过最长帧，切分
				{
					CuNoteNum=0;	//持续帧数计数清零
					currentTone=indexIte->second[i];
					LSHVectorDimention.push_back(indexIte->second[i]);	//录入LSH点末尾
				}
			}

			if (LSHVectorDimention.size()==NLSHsize)	//抽取了一个完整的LSH点
			{
				pointNum++;	//LSH点计数
				//加入LSH索引，序号，起始音高位置，持续帧数
				IndexLSH.insert(IndexType(pointNum,make_pair(SongName,make_pair(pos,frameNow))));
				float mean=MeanLSH(temp);	//求当前LSH点内所有帧的均值
				MeanNoteLSH(LSHVectorDimention,mean);	//减均值
				LSHVector.push_back(LSHVectorDimention);	//加入LSH点集
			}
			else	//当前pv文件抽取不到1个完整的LSH点，跳过
				break;

			//LSH窗移为第一个音符的持续帧数，且不超过最长帧
			if (oneNoteDuration>=noteMaxFrame)
				pos+=noteMaxFrame;
			else
				pos+=oneNoteDuration;
		}
	}
	ofstream outf("wav.result",ofstream::app);
	outf<<"NLSH总点数："<<pointNum<<endl;
	cout<<"NLSH总点数："<<pointNum<<endl;
	outf.close();

	return 0;
}

//从一维音高序列抽取LSH点
//输入：indexSongName，pv文件名和对应的一维音高序列
//StepFactor，选择LSH点的间隔数，间隔几帧抽取一个点
//LSHsize，LSH窗长，一个LSH点的大小
//LSHshift，LSH窗移
//maxFrame，一维音高序列最大帧数
//输出：LSHVector，LSH点，每个点为一个20维的音高序列
//IndexLSH,	LSH索引，记录在LSHVector的序号，带路径的文件名，起始音高位置
int IndexPitchToLSHVector(map<string , vector<float>> &indexSongName, 
	int StepFactor, int LSHsize, int LSHshift, int maxFrame, 
	vector<vector<float>> &LSHVector, map<unsigned long , pair<string,short>> &IndexLSH)
{
	map<string , vector<float>>::iterator indexIte;	//遍历pv音高序列的迭代器
	unsigned long pointNum=0;	//抽取LSH点计数
	typedef map<unsigned long , pair<string,short>>::value_type IndexType;	//LSH索引格式	

	//遍历所有pv文件
	for (indexIte=indexSongName.begin(); indexIte!=indexSongName.end(); indexIte++)
	{
		string SongName(indexIte->first);	//当前文件名
		int sizeTone=indexIte->second.size();	//一维音高序列点数

		//遍历当前文件的一维音高序列
		for (int pos=2; pos<sizeTone-StepFactor*(LSHsize+1) && pos<maxFrame; pos+=LSHshift)
		{
			pointNum++;
			double currentTone = indexIte->second[pos];	//当前音高

			IndexLSH.insert(IndexType(pointNum,make_pair(SongName,pos)));	//加入LSH索引，序号，带路径的文件名，起始音高位置
			
			vector <float> LSHVectorDimention;	//存储当前的LSH点

			//抽取一个LSH点
			for (int i=pos; i<pos+StepFactor*LSHsize; i+=StepFactor)
			{
				vector <double > temp;

				//取StepFactor个点的中值加入LSH向量
				for (int k=0;k<StepFactor;k++)
				{
					temp.push_back(indexIte->second[i+k]);
				}
				stable_sort(temp.begin(),temp.end());
				LSHVectorDimention.push_back(temp[StepFactor/2]);	
			}

			MinusMean(LSHVectorDimention);		//减均值
			LSHVector.push_back(LSHVectorDimention);	//加入LSH点集
		}
	}
	ofstream outf("wav.result",ofstream::app);
	outf<<"LSH总点数："<<pointNum<<endl;
	cout<<"LSH总点数："<<pointNum<<endl;
	outf.close();
	return 0;
}

//从LSH向量索引中读数据集，读入dataSet中，读入每个LSH点时记录序号和LSH点的平方和
//输入：LSHVector：LSH点集
PPointT * readDataSetFromVector(vector<vector<float>> &LSHVector )
{
	IntT nPoints = LSHVector.size();	//LSH点集大小，即pv文件数
	PPointT *dataSetPoints = NULL;		//返回的结果

	FAILIF(NULL == (dataSetPoints = (PPointT*)MALLOC(nPoints * sizeof(PPointT))));	//开辟空间

	for(IntT i = 0; i < nPoints; i++){
		//dataSetPoints[i]->coordinates为一个LSH点
		//dataSetPoints[i]->sqrlength为该点每个音高的平方和
		dataSetPoints[i] = readPointVector(LSHVector[i]);	
		dataSetPoints[i]->index = i;
	}

	return dataSetPoints;
}

//读入LSH点到PPoint中，统计平方和
PPointT readPointVector(vector<float> &VectorTone)
{
	PPointT p;
	float sqrLength = 0;	
	IntT pointsDimension = VectorTone.size();	//维数

	FAILIF(NULL == (p = (PPointT)MALLOC(sizeof(PointT))));
	FAILIF(NULL == (p->coordinates = (float*)MALLOC(pointsDimension * sizeof(float))));

	for(IntT d = 0; d < pointsDimension; d++)
	{
		p->coordinates[d]=VectorTone[d];
		sqrLength += SQR(p->coordinates[d]);
	}
	p->index = -1;
	p->sqrLength = sqrLength;	//记录平方和
	return p;
}


//抽取一维音高序列的NLSH点
//输入：queryPitch，查询的一维音符序列
//noteMinFrame，音符最短持续帧数，不足则去除
//noteMaxFrame，音符最长持续帧数，超过则切分
//NLSHsize,NLSH点维数
//输出：posPairvector，记录LSH点的起始位置和持续长度
//LSHQueryVectorLinearStretching，记录LSH点
int QueryPitchToLSHVectorLinearStretchingShortToMoreNoteFirst(vector<pair<short, short>>& posPairvector, 
	vector <float> &queryPitch,vector <vector <vector<float>>> &LSHQueryVectorLinearStretching,
	int noteMinFrame,int noteMaxFrame,int NLSHsize)
{
	int sizeTone = queryPitch.size();	//一维音符序列长度
	set<pair<short, short>> posPairTemp;	//记录每个NLSH点的起始位置和持续时间
	vector <vector <float> >LSHQueryVectorOneStretch;	//一次伸缩的结果
	long OneSongPoint=0;	//LSH点计数
	static int numEntry = 0;	//进入此函数计数
	static long QueryPoint20Demention = 0;	//LSH点累计计数

	numEntry++;	//进入此函数的次数

	//遍历查询的一维音高序列
	for (int pos=0;pos<sizeTone;)
	{
		float currentTone = 1000;	//记录当前音符
		short frameNow = 0;		//当前NLSH点持续帧数
		int oneNoteDuration = 0;	//第一个音符持续的帧数
		float TheFirstNote = queryPitch[pos];	//第一个音符

		vector <float> temp;	//当前NLSH点用的帧
		vector <float> LSHVector20Dimention;	//当前LSH点

		int CuNoteNum=0;	//当前音高持续帧数

		//从pos位置开始抽取一个NLSH点
		for (int i=pos;i<sizeTone;i++)
		{
			frameNow++;	//当前LSH点持续帧数
			temp.push_back(queryPitch[i]);

			if (TheFirstNote == queryPitch[i])	//记录第一个音符持续时间
				oneNoteDuration++;
			else
				TheFirstNote=1000;	//第一个音高结束，将TheFirstNote置为最大值防止oneNoteDuration继续累加

			//当前LSH点维数达上限，且当前音高和前一个不同，当前LSH点抽取完毕
			if (LSHVector20Dimention.size()==NLSHsize  && currentTone !=queryPitch[i])
			{
				break;
			} 

			if (currentTone !=queryPitch[i])	//当前音高和前一个不同
			{
				if(CuNoteNum<=noteMinFrame)	//前一个音高持续帧数小于等于最短帧数
				{
					if (!LSHVector20Dimention.empty())	//将长度不足的前一个音高去除
					{
						LSHVector20Dimention.pop_back();
					}
				}
				else	//前一个音高持续帧数大于最短帧数
				{
					CuNoteNum=0;	
				}
				currentTone=queryPitch[i];		//记录当前音高
				LSHVector20Dimention.push_back(queryPitch[i]);	//当前音高录入LSH点末尾
			}
			else	//和前一个音高相同
			{
				CuNoteNum++;	//当前音高持续帧数计数
			}

			//当前LSH点维数达上限，且当前音高持续帧数超过最长帧，当前LSH点抽取完毕
			if (LSHVector20Dimention.size()==NLSHsize && CuNoteNum>=noteMaxFrame)
			{
				break;
			}
			if (CuNoteNum >= noteMaxFrame)	//当前音高持续帧数超过最长帧，切分
			{
				CuNoteNum=0;
				currentTone=queryPitch[i];
				LSHVector20Dimention.push_back(queryPitch[i]);	//录入LSH点末尾
			}
		}

		if (LSHVector20Dimention.size()==NLSHsize)	//抽取了一个完整的LSH点
		{
			OneSongPoint++;	//LSH点计数
			if (posPairTemp.count(make_pair(pos,frameNow)))	//当前LSH点已存在
			{
				;
			} 
			else
			{
				posPairTemp.insert(make_pair(pos,frameNow));	//记录当前LSH点的起始位置和持续长度
				posPairvector.push_back(make_pair(pos,frameNow));	//记录当前LSH点的起始位置和持续长度
				float mean = MeanLSH(temp);	//求当前LSH点内所有帧的均值
				MeanNoteLSH(LSHVector20Dimention,mean);	//减均值
				LSHQueryVectorOneStretch.push_back(LSHVector20Dimention);	//加入LSH点集尾
			}
		}
		else	//当前pv文件抽取不到1个完整的LSH点，跳过
			break;

		//LSH窗移为第一个音符的持续帧数，且不超过最长帧
		if (oneNoteDuration>=noteMaxFrame)
		{
			pos+=noteMaxFrame;
		} 
		else
		{
			pos+=oneNoteDuration;
		}
	}

	LSHQueryVectorLinearStretching.push_back(LSHQueryVectorOneStretch);	//加入本次抽取的LSH点集

	QueryPoint20Demention += OneSongPoint;	//记录LSH点数

	if (numEntry%100==0 || numEntry >=330)
	{
		ofstream outf("wav.result",ofstream::app);
		outf<<"当前歌NLSH点数："<<OneSongPoint<<" 全部歌曲NLSH点数:"<<QueryPoint20Demention<<endl;
		cout<<"当前歌NLSH点数："<<OneSongPoint<<" 全部歌曲NLSH点数:"<<QueryPoint20Demention<<endl;
		outf.close();
	}

	return 0;
}

//线性伸缩并抽取LSH点
//输入：queryPitch，一维音高向量
//FloorLevel，初始伸缩因子，UpperLimit，伸缩因子上限，StretchStep，伸缩步长
//stepFactor，抽取间隔，stepRatio，抽取窗移，LSHsize，抽取窗长，recur，上层函数的循环序号
//输出：LSHQueryVectorLinearStretching，所有伸缩因子下的LSH点集
int QueryPitchToLSHVectorLinearStretchingShortToMore(vector <float> &queryPitch,
	vector <vector <vector<float>>> &LSHQueryVectorLinearStretching,
	float FloorLevel, float UpperLimit,int stepFactor,float stepRatio,float StretchStep,int recur,int LSHsize)
{
	long OneSongPoint=0;	//记录LSH点数
	static int numEntry=0;	//记录进入此函数的次数
	numEntry++;
	static long QueryPoint20Demention=0;	//记录LSH总点数

	//遍历所有伸缩因子
	for (; FloorLevel<UpperLimit+StretchStep; FloorLevel+=StretchStep)
	{
		if ((recur==1 || recur==2 )&& FloorLevel<=1.01 && FloorLevel>=0.99)
		{
			continue;
		}
		if (recur==2 && ((FloorLevel<=0.81 && FloorLevel>=0.79)||(FloorLevel<=1.21 && FloorLevel>=1.19)||(FloorLevel<=1.41 && FloorLevel>=1.39)))
		{
			continue;
		}
		vector <float> queryPitchStretch;	//线性伸缩后的串

		//线性伸缩，根据伸缩因子FloorLevel线性伸缩queryPitch，得到queryPitchStretch
		StringToString(queryPitch,queryPitchStretch,FloorLevel);

		int sizeTone = queryPitchStretch.size();	//线性伸缩后的维数
		int StepFactorCurrent = int(stepFactor*stepRatio*FloorLevel);	//线性伸缩后的帧移
		vector <vector <float> >LSHQueryVectorOneStretch;	//本次线性伸缩后的抽取的LSH点集

		//对线性伸缩后的一维音高序列后抽取LSH点
		for (int pos=0; pos<sizeTone-stepFactor*(LSHsize+1); pos+=StepFactorCurrent)
		{
			vector <float> LSHVector20Dimention;	//记录本次抽取的LSH点

			//从pos位置开始抽取一个LSH点
			for (int i=pos; i<pos+stepFactor*LSHsize; i+=stepFactor)
			{
				vector<float> temp;

				//取StepFactor个点的中值加入LSH点
				for (int k=0;k<stepFactor;k++)
				{
					temp.push_back(queryPitchStretch[i+k]);
				}
				stable_sort(temp.begin(),temp.end());
				LSHVector20Dimention.push_back(temp[stepFactor/2]);
			}
			MinusMean(LSHVector20Dimention);	//减均值
			OneSongPoint++;		//LSH点计数
			LSHQueryVectorOneStretch.push_back(LSHVector20Dimention);	//加入LSH点集
		}
		LSHQueryVectorLinearStretching.push_back(LSHQueryVectorOneStretch);	//本次抽取的LSH点集加入输出数组
	}
	QueryPoint20Demention += OneSongPoint;	//累计记录LSH点数
	
	if (numEntry%100==0 || numEntry >=330)
	{
		ofstream outf("wav.result",ofstream::app);
		outf<<"当前歌线性伸缩后的LSH点数："<<OneSongPoint<<" 全部歌曲线性伸缩的LSH点数:"<<QueryPoint20Demention<<endl;
		cout<<"当前歌线性伸缩后的LSH点数："<<OneSongPoint<<" 全部歌曲线性伸缩的LSH点数:"<<QueryPoint20Demention<<endl;
		outf.close();
	}
	
	return 0;
}


int IndexSignToQueryAndDataVectorHummingBeginQueryLengthFixedAndNoClearMatchLeast(vector <float> &DisCandidates,vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
															  map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,vector <float> queryPitch,
															   vector<float>  &queryX, vector< vector<float>  >&dataY,
															  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
															  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
															  vector<float>  &CandidatesDataYDis)//根据伸缩因子返回候选向量以及对应的歌曲名字
{  
	//匹配最少数目，即精确匹配的数目最少
	int posBegin=10;//与开头比较近的小于长度1/posBegin的全部保留匹配
	queryX.clear();
	StringToString(queryPitch,queryX,1);
	int sizePoints=CandidatesNumStretch.size();
	int sizeCandidates=0;
	int QueryLSHPos=0;
	int DifferPos=0;
	int sizeQuery=queryX.size();
	sizeQuery=sizeQuery*stretchFactor;
	pair<string, short> SongNamePos;
	map<string , vector<float>> :: iterator SongTone;
	vector<float>  DataYOnePoint;
	vector <string> :: iterator SongIter;
	vector< vector<float> >  :: iterator SongPitchIter;
	vector<float>  :: iterator SongDisIter;

	for (int i=0;i<sizePoints;i++)
	{
		int sizeCandidatesCurrent=sizeCandidates;
		int CurrentNum=CandidatesNumStretch[i];
		for (int j=sizeCandidatesCurrent;j<sizeCandidatesCurrent+CurrentNum;j++)
		{
			sizeCandidates++;

			if (IndexLSH.count(IndexCandidatesStretch[j]))
			{
				QueryLSHPos=i*stepFactor;
				SongNamePos=IndexLSH.find(IndexCandidatesStretch[j])->second;
				DifferPos=SongNamePos.second-QueryLSHPos;
				if (indexSongName.count(SongNamePos.first))
				{
					DataYOnePoint.clear();
					SongTone=indexSongName.find(SongNamePos.first);
					int sizeSong=SongTone->second.size();
					if (DifferPos>=0 && DifferPos<sizeQuery/MatchBeginPos &&DifferPos+sizeQuery/MatchBeginPos<sizeSong)//保证长度不是最末尾部分,最多错query一半
					{
						for (int k=DifferPos;k<DifferPos+sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//将此点复制过去
						}
						if (SongMapPosition.count(SongNamePos.first))
						{
							short posInMap=SongMapPosition.find(SongNamePos.first)->second;
							if (posInMap>DifferPos)//如果比列表中的位置靠前
							{
								if	(posInMap>(short)(sizeQuery/posBegin))//如果当前比较远
								{
									SongIter=find(SongNameMapToDataY.begin(),SongNameMapToDataY.end(),SongNamePos.first);
									SongPitchIter=dataY.begin()+(SongIter-SongNameMapToDataY.begin());
									SongDisIter=CandidatesDataYDis.begin()+(SongIter-SongNameMapToDataY.begin());
									SongNameMapToDataY.erase(SongIter);
									CandidatesDataYDis.erase(SongDisIter);
									dataY.erase(SongPitchIter);
									dataY.push_back(DataYOnePoint);
									CandidatesDataYDis.push_back(DisCandidates[j]);
									SongNameMapToDataY.push_back(SongNamePos.first);
									SongMapPosition[SongNamePos.first]=DifferPos;
								}
								else
								{
									dataY.push_back(DataYOnePoint);
									CandidatesDataYDis.push_back(DisCandidates[j]);
									SongNameMapToDataY.push_back(SongNamePos.first);
									SongMapPosition[SongNamePos.first]=sizeQuery/posBegin;
									CandidatesSizeInDWT++;
								}
							}
						}
						else
						{
							SongMapPosition.insert(make_pair(SongNamePos.first,DifferPos));
							dataY.push_back(DataYOnePoint);
							CandidatesDataYDis.push_back(DisCandidates[j]);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;

						}
					}
					else if (DifferPos<0 && DifferPos >-(sizeQuery/MatchBeginPos) &&  DifferPos+sizeQuery<sizeSong)
					{
						for (int k=0;k<sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//将此点复制过去

						}
						
						if (!SongMapPosition.count(SongNamePos.first))//如果此歌曲没有
						{
							SongMapPosition.insert(make_pair(SongNamePos.first,abs(DifferPos)));
							dataY.push_back(DataYOnePoint);
							CandidatesDataYDis.push_back(DisCandidates[j]);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;
						}						
						else
						{
							short posInMap=SongMapPosition.find(SongNamePos.first)->second;
							if	(posInMap> abs(DifferPos) )//比列表中的距离开头近
							{
								if	(posInMap>(short)(sizeQuery/posBegin))//如果当前比较远
								{
									SongIter=find(SongNameMapToDataY.begin(),SongNameMapToDataY.end(),SongNamePos.first);
									SongDisIter=CandidatesDataYDis.begin()+(SongIter-SongNameMapToDataY.begin());
									SongPitchIter=dataY.begin()+(SongIter-SongNameMapToDataY.begin());
									SongNameMapToDataY.erase(SongIter);
									dataY.erase(SongPitchIter);
									CandidatesDataYDis.erase(SongDisIter);
									dataY.push_back(DataYOnePoint);
									CandidatesDataYDis.push_back(DisCandidates[j]);
									SongNameMapToDataY.push_back(SongNamePos.first);
									SongMapPosition[SongNamePos.first]=abs(DifferPos);
								}
								else
								{
									dataY.push_back(DataYOnePoint);
									CandidatesDataYDis.push_back(DisCandidates[j]);
									SongNameMapToDataY.push_back(SongNamePos.first);
									SongMapPosition[SongNamePos.first]=sizeQuery/posBegin;
									CandidatesSizeInDWT++;
								}
							}
						}
					}
					else
						CandidatesNumStretch[i]--;//此矩阵中存储实际精确匹配的数目
				}
				else
					CandidatesNumStretch[i]--;
			}
			else
				CandidatesNumStretch[i]--;
		}
	}
	return 0;

}


int IndexSignToQueryAndDataVectorHummingMatchLeastALL(vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
																				  map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,vector <vector <double>> queryPitch,
																				  vector< vector<double> > &queryX,vector< vector< vector<double> > >&dataY,
																				  map<string , vector<vector<double>>> &indexSongName,vector <string> &SongNameMapToDataY,
																				  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition)//根据伸缩因子返回候选向量以及对应的歌曲名字
{  
	//匹配最少数目，即精确匹配的数目最少
	int posBegin=30;//与开头比较近的小于长度1/posBegin的全部保留匹配
	queryX.clear();
	int sizePoints=CandidatesNumStretch.size();
	int sizeCandidates=0;
	int QueryLSHPos=0;
	int DifferPos=0;
	int sizeQuery=queryX.size();
	sizeQuery=sizeQuery*stretchFactor;
	pair<string, short> SongNamePos;
	map<string , vector<vector<double>>> :: iterator SongTone;
	vector< vector<double> > DataYOnePoint;
	vector <string> :: iterator SongIter;
	vector< vector< vector<double> > > :: iterator SongPitchIter;

	for (int i=0;i<sizePoints;i++)
	{
		int sizeCandidatesCurrent=sizeCandidates;
		int CurrentNum=CandidatesNumStretch[i];
		for (int j=sizeCandidatesCurrent;j<sizeCandidatesCurrent+CurrentNum;j++)
		{
			sizeCandidates++;

			if (IndexLSH.count(IndexCandidatesStretch[j]))
			{
				QueryLSHPos=i*stepFactor;
				SongNamePos=IndexLSH.find(IndexCandidatesStretch[j])->second;
				DifferPos=SongNamePos.second-QueryLSHPos;
				if (indexSongName.count(SongNamePos.first))
				{
					DataYOnePoint.clear();
					SongTone=indexSongName.find(SongNamePos.first);
					int sizeSong=SongTone->second.size();
					if (DifferPos>=0  &&DifferPos+sizeQuery/2<sizeSong)//保证长度不是最末尾部分,最多错query一半
					{
						for (int k=DifferPos;k<DifferPos+sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//将此点复制过去

						}
						SongMapPosition.insert(make_pair(SongNamePos.first,DifferPos));
						dataY.push_back(DataYOnePoint);
						SongNameMapToDataY.push_back(SongNamePos.first);
						CandidatesSizeInDWT++;
					}
					else if (DifferPos<0 && DifferPos >-(sizeQuery/MatchBeginPos) &&  DifferPos+sizeQuery<sizeSong)
					{
						for (int k=0;k<sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//将此点复制过去
						}
						SongMapPosition.insert(make_pair(SongNamePos.first,abs(DifferPos)));
						dataY.push_back(DataYOnePoint);
						SongNameMapToDataY.push_back(SongNamePos.first);
						CandidatesSizeInDWT++;
					}
					else
						CandidatesNumStretch[i]--;//此矩阵中存储实际精确匹配的数目
				}
				else
					CandidatesNumStretch[i]--;
			}
			else
				CandidatesNumStretch[i]--;
		}
	}
	return 0;
}


int LSHresultRate(char *wavename,int sizeQuery,int stepFactor,float stretch,vector<vector<IntT>> &IndexCandidatesStretch,
			  map<unsigned long , pair<string,short>> &IndexLSH,map<unsigned long , pair<string,pair<short,short>>> &IndexLSHNote,
			  float StretchStep,vector<vector<float>> &IndexCandidatesDis)
{
	string   songNameFive;
	int SizePoints=IndexCandidatesStretch.size();
	int QueryLSHPos=0;
	float stretchCurrent=0;
	static int find1=0;
	static int findSmall=0;
	static int returnPointNum=0;
	static int allWav=0;
	static int LSHresultMatch[300]={0};
	allWav++;
	ofstream outf("LSH.result",ofstream::app);//识别结果保存文件
	if (allWav%500==0 || allWav>=330)
	{
		outf<<wavename<< endl;
		cout<<wavename<< endl;
	}

	bool BigRe=FALSE;
	for (int i=0;i<SizePoints;i++)
	{
		stretchCurrent=sizeQuery*(stretch+i*StretchStep);
		int sizeC=IndexCandidatesStretch[i].size();

		for (int j=0;j<sizeC;j++)
		{
			if (IndexLSH.count(IndexCandidatesStretch[i][j]))
			{
				if (i==SizePoints-1)
				{
					songNameFive=IndexLSHNote.find(IndexCandidatesStretch[i][j])->second.first;
				}
				else
					songNameFive=IndexLSH.find(IndexCandidatesStretch[i][j])->second.first;
				QueryLSHPos=IndexLSH.find(IndexCandidatesStretch[i][j])->second.second-i*stepFactor;
				songNameFive.erase(0,6);
				int posSong=songNameFive.rfind("pv");
				//songNameFive.erase(5,6);
				songNameFive.erase(posSong-1,3);
				string nameSong(wavename);

				string::size_type beginiter=0;
				string::size_type enditer=0;
				//beginiter=nameSong.rfind("\");
				enditer=nameSong.rfind(".wav");
				string nameSongResult;
				nameSongResult.assign(nameSong,enditer-4,4);
				if (nameSongResult== songNameFive && QueryLSHPos>-(stretchCurrent/10) 
					&& QueryLSHPos<stretchCurrent/10)
				{
					if	(!BigRe)
					{find1++;}
					findSmall++;
					if (allWav%500==0 || allWav>=4430)
					{
						cout<<"find: "<<find1<<"findSamll: "<<findSmall<<endl;
						outf<<"find: "<<find1<<"findSamll: "<<findSmall<<endl;
						cout<<"allwav: "<<allWav<<endl;
						outf<<"allwav: "<<allWav<<endl;
					}

					j=sizeC;
					i=SizePoints;
				}
				else if	(nameSongResult== songNameFive && QueryLSHPos>-(stretchCurrent/2) 
					&& QueryLSHPos<stretchCurrent/2 && !BigRe)
				{
					find1++;
					if (allWav%500==0 || allWav>=4430)
					{
						cout<<"find: "<<find1<<endl;
						outf<<"find: "<<find1<<endl;
						cout<<"allwav: "<<allWav<<endl;
						outf<<"allwav: "<<allWav<<endl;
					}
					BigRe=TRUE;
				}
			}
		}
	}

	int currentNum=0;
	ofstream outf2("wav.result",ofstream::app);//识别结果保存文件
	ofstream outfCandidates("wavCandidates.result",ofstream::app);

	static int num1=0;
	static int num2=0;
	static int num3=0;
	static int num4=0;
	static int num5=0;
	static int num10=0;
	static int num20=0;
	static int num50=0;
	static int num100=0;
	static int num200=0;
	static int num400=0;
	static int num800=0;
	static int num1200=0;
	static int num1600=0;
	static int num2000=0;
	static int num2400=0;
	static int num2800=0;
	static int num3200=0;
	static int num3600=0;
	static int num4000=0;
	static int findall=0;

	static int rightNum[49]={0};
	static int wrongNum[49]={0};
	map <string, int> songNum;
	map <float, string> songUl;
	for (int i=0;i<SizePoints;i++)
	{
		stretchCurrent=sizeQuery*(stretch+i*StretchStep);
		int sizeC=IndexCandidatesStretch[i].size();
		returnPointNum+=sizeC;
		for (int j=0;j<sizeC;j++)
		{
			if (IndexLSH.count(IndexCandidatesStretch[i][j]))
			{

				if (i==SizePoints-1)
				{
					songNameFive=IndexLSHNote.find(IndexCandidatesStretch[i][j])->second.first;
				}
				else
					songNameFive=IndexLSH.find(IndexCandidatesStretch[i][j])->second.first;

				QueryLSHPos=IndexLSH.find(IndexCandidatesStretch[i][j])->second.second-i*stepFactor;
				songNameFive.erase(0,6);
				int posSong=songNameFive.rfind("pv");
				songNameFive.erase(posSong-1,3);
				songUl.insert(make_pair(IndexCandidatesDis[i][j],songNameFive));
				if (songNum.count(songNameFive))
				{
					songNum[songNameFive]++;
				}
				else
					songNum.insert(make_pair(songNameFive,0));
				string nameSong(wavename);

				string::size_type beginiter=0;
				string::size_type enditer=0;
				enditer=nameSong.rfind(".wav");
				string nameSongResult;
				nameSongResult.assign(nameSong,enditer-4,4);
				if (nameSongResult== songNameFive )
				{
					currentNum++;
				}
			}
		}
	}
	if (currentNum<300)
	{
		LSHresultMatch[currentNum]++;
	}
	if (allWav%500==0 || allWav>=4430)
	{
		cout<<"returnAllPoint: "<<returnPointNum<<endl;
		outf<<"returnAllPoint: "<<returnPointNum<<endl;
		cout<<"RightPoints: ";
		outf<<"RightPoints: ";
		int allmatchSong=0;
		for (int i=0;i<300;i++)
		{
			if (i>0)
			{
				allmatchSong+=LSHresultMatch[i];
			}
			cout<<LSHresultMatch[i]<<"  ";
			outf<<LSHresultMatch[i]<<"  ";
		}
		cout<<"LSHFindAllRightSong: "<<allmatchSong<<endl;
		outf<<"LSHFindAllRightSong: "<<allmatchSong<<endl;
		cout<<endl;
		outf<<endl;
	}

	outf.close();
	map<float ,string> songDis;

	vector <float> Dis;
	set <int> TempDis;
	float fl=0.00001;
	int numfl=1;
	map <string, int> ::const_iterator iterdis=songNum.begin();
	for (;iterdis!=songNum.end();iterdis++)
	{
		numfl++;
		Dis.push_back(iterdis->second+fl*numfl);

		songDis.insert(make_pair(iterdis->second+fl*numfl,iterdis->first));
	}

	set <string> SongFindAlready;
	static int total=0;
	int findtop5=0;
	int findNum=0;
	int accurateFind=0;
	stable_sort(Dis.begin(),Dis.end());
	int totalDis=Dis.size();
	for (int i=0,k=0;i!=songDis.size()/*&&i!=1000 && k<500*/;i++)
	{
		songNameFive=songDis.find(Dis[totalDis-i-1])->second;
		
		accurateFind=SongFindAlready.size();
		string nameSong(wavename);

		string::size_type beginiter=0;
		string::size_type enditer=0;
		enditer=nameSong.rfind(".wav");
		string nameSongResult;
		nameSongResult.assign(nameSong,enditer-4,4);

		int top5R=atoi(nameSongResult.c_str());
		if (top5R<0 || top5R>48)
		{
			top5R=0;
		}
		if (SongFindAlready.count(songNameFive))
		{
			;
		}
		else
		{
			k++;
			if (k<5)
			{
				if (total%200==0 || total >=4410)
				{
					outf<<songNameFive<<endl;
					cout<<songNameFive<<endl;
					outf<<"the distance: "<<Dis[i]<<endl;
					cout<<"the distance: "<<Dis[i]<<endl;
				}
			}

			SongFindAlready.insert(songNameFive);
			if (nameSongResult== songNameFive)
			{

				
				findall++;
				if (accurateFind==0)
				{
					num1++,num2++,num3++,num4++,num5++,num10++,num20++,num50++;num100++;num200++;
					rightNum[top5R]++;
					wrongNum[top5R]--;
					findtop5=1;
				}
				if (accurateFind==1)
				{
					num2++,num3++,num4++,num5++,num10++,num20++,num50++;num100++;num200++;
					rightNum[top5R]++;
					wrongNum[top5R]--;
					findtop5=1;
				}
				if (accurateFind==2)
				{
					num3++,num4++,num5++,num10++,num20++,num50++;num100++;num200++;
					rightNum[top5R]++;
					wrongNum[top5R]--;
					findtop5=1;
				}
				if (accurateFind==3){
					num4++,num5++,num10++,num20++,num50++;num100++;num200++;
					rightNum[top5R]++;
					wrongNum[top5R]--;
					findtop5=1;
				}
				if (accurateFind==4)
				{
					num5++,num10++,num20++,num50++;num100++;num200++;
					rightNum[top5R]++;
					wrongNum[top5R]--;
					findtop5=1;
				}
				if (accurateFind<=9 && accurateFind>4)
				{
					num10++,num20++,num50++;num100++;num200++;
				}
				if (accurateFind<=19 && accurateFind>9)
				{
					num20++,num50++;num100++;num200++;
				}
				if (accurateFind<=50 && accurateFind>19)
				{
					num50++;num100++;num200++;
				}
				if (accurateFind<=100 && accurateFind>50)
				{
					num100++;num200++;
				}
				if (accurateFind<=200 && accurateFind>100)
				{
					num200++;
				}
				if (findNum<=40)
				{
					num400++,num800++,num1200++,num1600++,num2000++,num2400++,num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=80 && findNum>40)
				{
                    num800++,num1200++,num1600++,num2000++,num2400++,num2800++,num3200++,num3600++,num4000++;

				}
				if (findNum<=120 && findNum>80)
				{
					num1200++,num1600++,num2000++,num2400++,num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=160 && findNum>120)
				{
					num1600++,num2000++,num2400++,num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=200 && findNum>160)
				{
					num2000++,num2400++,num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=400 && findNum>200)
				{
					num2400++,num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=800 && findNum>400)
				{
					num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=1200 && findNum>800)
				{
					num3200++,num3600++,num4000++;
				}
				if (findNum<=1600 && findNum>1200)
				{
					num3600++,num4000++;
				}
				if (findNum<=2000 && findNum>1600)
				{
					num4000++;
				}
				if (total%10==0 || total>=330)
				{
					cout<<"准确率：find1: "<<num1<<"  find2: "<<num2<<"  find3: "<<num3<<"  find4: "<<num4<<"  find5: "<<num5<<"  find10: "<<num10<<"  find20: "<<num20<<"  find50: "<<num50<<"  find100: "<<num100<<"  find200: "<<num200<<"  findall: "<<findall<<endl;
					cout<<"召回率：find40: "<<num400<<"  find80: "<<num800<<"  find120: "<<num1200<<"  find160: "<<num1600<<"  find200: "<<num2000<<"  find400: "<<num2400<<"  find800: "<<num2800<<"  find1200: "<<num3200<<"  find1600: "<<num3600<<"  find2000: "<<num4000<<endl;
					outf2<<"准确率：find1: "<<num1<<"  find2: "<<num2<<"  find3: "<<num3<<"  find4: "<<num4<<"  find5: "<<num5<<"  find10: "<<num10<<"  find20: "<<num20<<"  find50: "<<num50<<"  find100: "<<num100<<"  find200: "<<num200<<"  findall: "<<findall<<endl;
					outf2<<"召回率：find40: "<<num400<<"  find80: "<<num800<<"  find120: "<<num1200<<"  find160: "<<num1600<<"  find200: "<<num2000<<"  find400: "<<num2400<<"  find800: "<<num2800<<"  find1200: "<<num3200<<"  find1600: "<<num3600<<"  find2000: "<<num4000<<endl;
				}
			}
		}
		findNum+=Dis[totalDis-i-1];
	}
	total++;
	if (total==1000)
	{
		ofstream out1000("1000.result",ofstream::app);
		out1000<<"find1: "<<num1<<"  find2: "<<num2<<"  find3: "<<num3<<"  find4: "<<num4<<"  find5: "<<num5<<"  find10: "<<num10<<"  find20: "<<num20<<"  find30: "<<num50<<"  findall: "<<findall<<endl;
		out1000.close();
	}
	if (findtop5!=1)
	{
		ofstream outfError("error.result",ofstream::app);
		outfError<<wavename<<endl;
		outfError.close();
	}

	int allsongC=0;
	if (total%500==0 || total>=4428)
	{
		outf<<"正确点和错误点："<<"  ";
		cout<<"正确点和错误点："<<"  ";
		for (int i=0;i<49;i++)
		{
			outf2<<i<<" r:"<<rightNum[i]<<" w: "<<wrongNum[i]<<"  ";
			cout<<i<<" r:"<<rightNum[i]<<" w: "<<wrongNum[i]<<"  ";
		}
		outf2<<"all: "<<allsongC<<endl;
		cout<<"all: "<<allsongC<<endl;
		outf2<<wavename<<"总的候选点数："<<Dis.size()<<" 全部歌曲候选点数："/*<<CandidatesDTWAll*/<< endl;
		outfCandidates<<wavename<<"总的候选点数："<<Dis.size()<< endl;
		cout<<wavename<<"总的候选点数："<<Dis.size()<<" 全部歌曲候选点数："/*<<CandidatesDTWAll*/<< endl;
	}
	for (int i=0;i<49;i++)
	{
		allsongC+=rightNum[i];
		allsongC+=wrongNum[i];
	}
	outfCandidates.close();
	if (total%10==0 || total>=4420)
	{
		cout<<"the total: "<<total<<endl;
		outf2<<"the total: "<<total<<endl;
	}
	return 0;
}


int LSHresult(char *wavename,int sizeQuery,int stepFactor,float stretch,vector<vector<IntT>> &IndexCandidatesStretch,
			  map<unsigned long , pair<string,short>> &IndexLSH,float StretchStep)
{
	string   songNameFive;
	int SizePoints=IndexCandidatesStretch.size();
	int QueryLSHPos=0;
	float stretchCurrent=0;
	static int find1=0;
	static int findSmall=0;
	static int returnPointNum=0;
	static int allWav=0;
	static int LSHresultMatch[300]={0};
	allWav++;
	ofstream outf("LSH.result",ofstream::app);//识别结果保存文件
	if (allWav%500==0 || allWav>=4430)
	{
		outf<<wavename<< endl;
		cout<<wavename<< endl;
	}
	
	bool BigRe=FALSE;
	for (int i=0;i<SizePoints;i++)
	{
		stretchCurrent=sizeQuery*(stretch+i*StretchStep);
		int sizeC=IndexCandidatesStretch[i].size();

		for (int j=0;j<sizeC;j++)
		{
			if (IndexLSH.count(IndexCandidatesStretch[i][j]))
			{
				songNameFive=IndexLSH.find(IndexCandidatesStretch[i][j])->second.first;
				QueryLSHPos=IndexLSH.find(IndexCandidatesStretch[i][j])->second.second-i*stepFactor;
				songNameFive.erase(0,6);
				int posSong=songNameFive.rfind("pv");
				songNameFive.erase(posSong-1,3);
				string nameSong(wavename);

				string::size_type beginiter=0;
				string::size_type enditer=0;
				enditer=nameSong.rfind(".wav");
				string nameSongResult;
				nameSongResult.assign(nameSong,enditer-4,4);
				if (nameSongResult== songNameFive && QueryLSHPos>-(stretchCurrent/10) 
					&& QueryLSHPos<stretchCurrent/10)
				{
					if	(!BigRe)
					{find1++;}
					findSmall++;
					if (allWav%500==0 || allWav>=4430)
					{
						cout<<"find: "<<find1<<"findSamll: "<<findSmall<<endl;
						outf<<"find: "<<find1<<"findSamll: "<<findSmall<<endl;
						cout<<"allwav: "<<allWav<<endl;
						outf<<"allwav: "<<allWav<<endl;
					}
					j=sizeC;
					i=SizePoints;

				}
				else if	(nameSongResult== songNameFive && QueryLSHPos>-(stretchCurrent/2) 
					&& QueryLSHPos<stretchCurrent/2 && !BigRe)
				{
					find1++;
					if (allWav%500==0 || allWav>=4430)
					{
						cout<<"find: "<<find1<<endl;
						outf<<"find: "<<find1<<endl;
						cout<<"allwav: "<<allWav<<endl;
						outf<<"allwav: "<<allWav<<endl;
					}
					BigRe=TRUE;
				}
			}
		}
	}

	int currentNum=0;
	for (int i=0;i<SizePoints;i++)
	{
		stretchCurrent=sizeQuery*(stretch+i*StretchStep);
		int sizeC=IndexCandidatesStretch[i].size();
		returnPointNum+=sizeC;
		for (int j=0;j<sizeC;j++)
		{
			if (IndexLSH.count(IndexCandidatesStretch[i][j]))
			{
				songNameFive=IndexLSH.find(IndexCandidatesStretch[i][j])->second.first;
				QueryLSHPos=IndexLSH.find(IndexCandidatesStretch[i][j])->second.second-i*stepFactor;
				songNameFive.erase(0,6);
				int posSong=songNameFive.rfind("pv");
				songNameFive.erase(posSong-1,3);
				string nameSong(wavename);

				string::size_type beginiter=0;
				string::size_type enditer=0;
				enditer=nameSong.rfind(".wav");
				string nameSongResult;
				nameSongResult.assign(nameSong,enditer-4,4);
				if (nameSongResult== songNameFive )
				{
					currentNum++;
				}
			}
		}
	}
	if (currentNum<300)
	{
		LSHresultMatch[currentNum]++;
	}
	if (allWav%500==0 || allWav>=4430)
	{
		cout<<"returnAllPoint: "<<returnPointNum<<endl;
		outf<<"returnAllPoint: "<<returnPointNum<<endl;
		cout<<"RightPoints: ";
		outf<<"RightPoints: ";
		int allmatchSong=0;
		for (int i=0;i<300;i++)
		{
			if (i>0)
			{
				allmatchSong+=LSHresultMatch[i];
			}
			cout<<LSHresultMatch[i]<<"  ";
			outf<<LSHresultMatch[i]<<"  ";
		}
		cout<<"LSHFindAllRightSong: "<<allmatchSong<<endl;
		outf<<"LSHFindAllRightSong: "<<allmatchSong<<endl;
		cout<<endl;
		outf<<endl;
	}
	
	outf.close();
	return 0;
}

int LSHFilter(char *wavename,map<string ,int> &LSHFilterMap,vector<IntT> &CandidatesFilter,
			  map<unsigned long , pair<string,short>> &IndexLSH)
{
	string   songNameFive;
	int SizePoints=CandidatesFilter.size();
	static int LSHresultMatch[500]={0};
	static int songNum=0;
	songNum++;
	ofstream outf("LSH.result",ofstream::app);//识别结果保存文件
	int currentNum=0;
	for (int i=0;i<SizePoints;i++)
	{
		if (IndexLSH.count(CandidatesFilter[i]))
		{
			songNameFive=IndexLSH.find(CandidatesFilter[i])->second.first;
			songNameFive.erase(0,6);
			int posSong=songNameFive.rfind("pv");
			songNameFive.erase(posSong-1,3);
			string nameSong(wavename);

			string::size_type beginiter=0;
			string::size_type enditer=0;
			enditer=nameSong.rfind(".wav");
			string nameSongResult;
			nameSongResult.assign(nameSong,enditer-4,4);
			if (nameSongResult== songNameFive )
			{
				currentNum++;
			}
		}
	}
	if (currentNum<500)
	{
		LSHresultMatch[currentNum]++;
	}
	if (songNum%500==0 || songNum>=4430)
	{
		cout<<"LSHFilter: "<<endl;
		outf<<"LSHFilter: "<<endl;
		cout<<"RightPoints: ";
		outf<<"RightPoints: ";
		int allmatchSong=0;
		for (int i=0;i<500;i++)
		{
			if (i>0)
			{
				allmatchSong+=LSHresultMatch[i];
			}
			cout<<LSHresultMatch[i]<<"  ";
			outf<<LSHresultMatch[i]<<"  ";
		}
		cout<<"LSHFindAllRightSong  Filter: "<<allmatchSong<<endl;
		outf<<"LSHFindAllRightSong  Filter: "<<allmatchSong<<endl;
		cout<<endl;
		outf<<endl;
	}
	
	outf.close();
	for (int i=0;i<SizePoints;i++)
	{
		if (IndexLSH.count(CandidatesFilter[i]))
		{
			songNameFive=IndexLSH.find(CandidatesFilter[i])->second.first;
			if (LSHFilterMap.count(songNameFive))
			{
				LSHFilterMap[songNameFive]++;
			}
			else
			    LSHFilterMap[songNameFive]=1;
		}
	}
	return 0;
}