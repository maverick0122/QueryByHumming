#include "songname.h"
#include "lib.h"

//从文件读入歌曲ID和歌名映射表
//文件每行为：ID 歌名
void songread(string IDAndNameFile, map <string ,string> &songIDAndName)
{
	ifstream file(IDAndNameFile);
	string line;
	while (getline(file,line))
	{
		int pos = line.find_first_of(' ');//返回第一个匹配空格的位置
		if(pos != string::npos)//说明匹配成功
		{
			string name(line,0,pos);
			line.erase(0,pos+1);//删除歌曲ID 剩下歌名映射表
			songIDAndName.insert(make_pair(name,line));
		}
	}
}