#include"LSHVectorToFile.h"

//将LSH点写入文件
//输入：LSHVector，LSH点，每个点为一个音高序列
//filename，输出文件路径
int LSHVectorToFile(vector<vector<float>> &LSHVector, string filename)
{
	if(!LSHVector.size())
		return 0;

	ofstream file(filename);

	file << LSHVector[0].size() << endl;

	for(int i=0; i<LSHVector.size(); i++)
	{
		for(int j=0; j<LSHVector[i].size(); j++)
		{
			file << LSHVector[i][j] << " ";
		}
		file << endl;
	}

	file.close();

	return 1;
}


//将LSH索引写入文件
//输入：IndexLSH，LSH索引，记录在LSHVector的序号，带路径的文件名，起始位置
//filename，输出文件路径
int IndexLSHToFile(map<unsigned long , pair<string,short>> &IndexLSH, string filename)
{
	if(!IndexLSH.size())
		return 0;

	ofstream file(filename);

	file << "3" << endl;	//每行3个参数：序号，带路径的文件名，起始位置

	map<unsigned long , pair<string,short>>::iterator it;

	for(it=IndexLSH.begin(); it!=IndexLSH.end(); it++)
	{
		file << it->first << " " << it->second.first << " " <<  it->second.second << endl;
	}

	file.close();

	return 1;
}

//将NLSH索引写入文件
//输入：IndexLSH，NLSH索引，记录在LSHVector的序号，带路径的文件名，起始位置，持续帧数
//filename，输出文件路径
int IndexLSHNoteToFile(map<unsigned long , pair<string,pair<short,short>>> &IndexLSH, string filename)
{
	if(!IndexLSH.size())
		return 0;

	ofstream file(filename);

	file << "4" << endl;	//每行4个参数：序号，带路径的文件名，起始位置，持续帧数

	map<unsigned long , pair<string,pair<short,short>>>::iterator it;

	for(it=IndexLSH.begin(); it!=IndexLSH.end(); it++)
	{
		file << it->first << " " << it->second.first << " " <<  
			it->second.second.first << " " << it->second.second.second << endl;
	}

	file.close();

	return 1;
}

//将LS后的LSH点写入文件
//输入：LSHVectorLS，LS后的LSH点，每个点为一个音高序列，vector[i][j][k]表示第i个伸缩因子下的第j个采样点的第k个数据
//filename，输出文件路径，输出文件中，每行一个LSH点
int LSHVectorLSToFile(vector<vector<vector<float>>> &LSHVectorLS, string filename)
{
	if(!LSHVectorLS.size())
		return 0;

	ofstream file;
	file.open(filename.c_str(),ios::app);	//有来自多个文件的查询，需要追加打开

	//file << LSHVectorLS[0][0].size() << endl;

	for(int i=0; i<LSHVectorLS.size(); i++)
	{
		for(int j=0; j<LSHVectorLS[i].size(); j++)
		{
			for(int k=0; k<LSHVectorLS[i][j].size(); k++)
				file << LSHVectorLS[i][j][k] << " ";
			file << endl;
		}
	}

	file.close();

	return 1;
}

//将LS后的LSH索引写入文件
//输入：LSHVectorLS，LS后的LSH点，每个点为一个音高序列，vector[i][j][k]表示第i个伸缩因子下的第j个采样点的第k个数据
//filename，输出文件路径，输出文件中，每行为上述LSH点对应的索引（目前使用LSH点所属的文件名）
int IndexLSHLSToFile(vector<vector<vector<float>>> &LSHVectorLS, string wavname, string filename)
{
	if(!LSHVectorLS.size())
		return 0;

	ofstream file;
	file.open(filename.c_str(),ios::app);	//有来自多个文件的查询，需要追加打开

	//file << LSHVectorLS[0][0].size() << endl;

	for(int i=0; i<LSHVectorLS.size(); i++)
	{
		for(int j=0; j<LSHVectorLS[i].size(); j++)
		{
			file << wavname << endl;
		}
	}

	file.close();

	return 1;
}