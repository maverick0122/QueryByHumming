#include"LSHVectorToFile.h"

//将LSH点写入文件
//输入：LSHVector，LSH点，每个点为一个音高序列
//filename，输出文件路径
int LSHVectorToFile(vector<vector<float>> LSHVector, string filename)
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
int IndexLSHToFile(map<unsigned long , pair<string,short>> IndexLSH, string filename)
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