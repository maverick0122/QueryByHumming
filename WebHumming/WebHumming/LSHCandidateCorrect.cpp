#include"LSHCandidateCorrect.h"



int NLSHCandidateCorrect(char*wavename,map<unsigned long , pair<string,pair<short,short>>> &IndexLSHNote,vector<int> &IndexCandidatesStretch,vector<int> &CandidatesNumStretch)
{
	string songName(wavename);
	string wavName;
	string canSong;
	int sizePoints=CandidatesNumStretch.size();
	
	int allCandidates=0;//所有候选点计数
	int candidates=0;
	int curPos=0;
	float rate=0;//正确率

	string::size_type wavEnd=0;
	string::size_type wavStart=0;
	ofstream filename("NLSHCandidateCorrect.txt",ofstream::app);
	//取wav文件歌曲名，去除路径
	//filename<<"歌曲准确率及点数："<<sizePoints<<endl;
	filename<<songName<<endl;
	wavStart=songName.rfind("\\");
	wavEnd=songName.rfind(".wav");
	wavName.assign(songName,wavStart+1,wavEnd-wavStart-1);
	//filename<<wavName<<":"<<wavStart<<","<<wavEnd<<endl;
	
	for(int i=0;i<sizePoints;i++)
	{
		int curPos=allCandidates;
		int curNum=CandidatesNumStretch[i];
		//filename<<"每个候选点数"<<curNum<<endl;
		for(int j=curPos;j<curNum+curPos;j++)
		{
			allCandidates++;
			int canNum=IndexCandidatesStretch[j];
			if(IndexLSHNote.count(canNum))
			{
				canSong=IndexLSHNote.find(canNum)->second.first;
				//filename<<canSong<<endl;
				canSong.erase(0,6);
				int pvPos=canSong.rfind("pv");
				canSong.erase(pvPos-1,3);
				//filename<<canSong;
				if(canSong==wavName)
				{
					candidates++;
				}
			}
		}
		//filename<<endl;

	}
	filename<<allCandidates<<endl;
	filename<<candidates<<endl;
	rate=(float)candidates/allCandidates;
	filename<<rate<<endl;
	filename.close();

	return 0;
}



int LSHCandidateCorrect(char*wavename,map<unsigned long , pair<string,short>> &IndexLSH,vector<int> &IndexCandidatesStretch,vector<int> &CandidatesNumStretch,vector<int> &allStretchCandidates,vector<int> &allCorrectCan)
{
	string songName(wavename);
	string wavName;
	string canSong;
	int sizePoints=CandidatesNumStretch.size();
	
	int allCandidates=0;//所有候选点计数
	int restCandidates=0;//去重后的候选
	int candidates=0;
	int curPos=0;
	float rate=0;//正确率
	
	
	string::size_type wavEnd=0;
	string::size_type wavStart=0;
	ofstream filename("LSHCandidateCorrect.txt",ofstream::app);

	set <int> allCandidate;
	set <int>::iterator iterbegin;
	set <int>::iterator iterend;
	//取wav文件歌曲名，去除路径
	//filename<<"歌曲准确率及点数："<<sizePoints<<endl;
	filename<<songName<<endl;
	wavStart=songName.rfind("\\");
	wavEnd=songName.rfind(".wav");
	wavName.assign(songName,wavStart+1,wavEnd-wavStart-1);
	//filename<<wavName<<":"<<wavStart<<","<<wavEnd<<endl;
	
	for(int i=0;i<sizePoints;i++)
	{
		int curPos=allCandidates;
		int curNum=CandidatesNumStretch[i];
		//filename<<"每个候选点数"<<curNum<<endl;
		for(int j=curPos;j<curNum+curPos;j++)
		{
			allCandidates++;
			int canNum=IndexCandidatesStretch[j];
			allCandidate.insert(canNum);
		}
	}
	iterbegin=allCandidate.begin();
	iterend=allCandidate.end();
	for(;iterbegin!=iterend;++iterbegin)
	{
		restCandidates++;
		int canNum=*iterbegin;
		if(IndexLSH.count(canNum))
		{
			canSong=IndexLSH.find(canNum)->second.first;
			//filename<<canSong<<endl;
			canSong.erase(0,6);
			int pvPos=canSong.rfind("pv");
			canSong.erase(pvPos-1,3);
			//filename<<canSong;
			if(canSong==wavName)
			{
				candidates++;
			}
		}
	}
		//filename<<endl;

	allStretchCandidates.push_back(restCandidates);
	allCorrectCan.push_back(candidates);
	filename<<"一次伸缩候选点数："<<allCandidates<<endl;
	filename<<"去重后的候选点数："<<restCandidates<<endl;
	filename<<"正确候选点数："<<candidates<<endl;
	if(restCandidates>0)
	{
		rate=(float)candidates/restCandidates;
	}
	else
		rate=0;
	filename<<"正确比率："<<rate<<endl;
	filename.close();
	
	allCandidate.clear ();
	return candidates;
}

int BALSnLSHCandidateCorrect(char*wavename,vector <string> &SongNameMapToDataY)
{
	string songName(wavename);
	string wavName;
	string canSong;
	int allCandidates=0;//所有候选点计数
	int candidates=0;
	int sizePoints=SongNameMapToDataY.size();
	float rate=0;//正确率
	string::size_type wavEnd=0;
	string::size_type wavStart=0;
	ofstream filename("BALSnLSHCandidateCorrect.txt",ofstream::app);
	//取wav文件歌曲名，去除路径
	filename<<songName<<endl;
	wavStart=songName.rfind("\\");
	wavEnd=songName.rfind(".wav");
	wavName.assign(songName,wavStart+1,wavEnd-wavStart-1);

	for(int i=0;i<sizePoints;i++)
	{
		allCandidates++;
		canSong=SongNameMapToDataY[i];
		canSong.erase(0,6);
		int pvPos=canSong.rfind("pv");
		canSong.erase(pvPos-1,3);
		//filename<<canSong;
		if(canSong==wavName)
		{
			candidates++;
		}
	}
	filename<<allCandidates<<endl;
	filename<<candidates<<endl;
	rate=(float)candidates/allCandidates;
	filename<<rate<<endl;
	filename.close();

	return 0;

}

int BALSLSHCandidateCorrect(char*wavename,vector <string> &SongNameMapToDataY)
{
	string songName(wavename);
	string wavName;
	string canSong;
	int allCandidates=0;//所有候选点计数
	int candidates=0;
	int sizePoints=SongNameMapToDataY.size();
	float rate=0;//正确率
	string::size_type wavEnd=0;
	string::size_type wavStart=0;
	ofstream filename("BALSLSHCandidateCorrect.txt",ofstream::app);
	//取wav文件歌曲名，去除路径
	filename<<songName<<endl;
	wavStart=songName.rfind("\\");
	wavEnd=songName.rfind(".wav");
	wavName.assign(songName,wavStart+1,wavEnd-wavStart-1);

	for(int i=0;i<sizePoints;i++)
	{
		allCandidates++;
		canSong=SongNameMapToDataY[i];
		canSong.erase(0,6);
		int pvPos=canSong.rfind("pv");
		canSong.erase(pvPos-1,3);
		//filename<<canSong;
		if(canSong==wavName)
		{
			candidates++;
		}
	}
	filename<<allCandidates<<endl;
	filename<<candidates<<endl;
	rate=(float)candidates/allCandidates;
	filename<<rate<<endl;
	filename.close();

	return 0;

}