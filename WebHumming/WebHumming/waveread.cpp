/**************************************************************************/
/*             Function for reading data from WAV_PCM file                    */
/*         Return value:                                                  */
/*              (short int*): Pointer to the data buffer,if success       */
/*              NULL : If fail                                            */
/**************************************************************************/
#include<stdlib.h>
#include<math.h>
#include<stdio.h>
#include "define.h"
struct wavehead
{
	char sign[4];         //"RIFF"标志
    long int flength;     //文件长度 
    char wavesign[4];     //"WAVE"标志
    char fmtsign[4];      //"fmt"标志
	long int unused;      // 过渡字节（不定）
    short formattype;     //格式类别（10H为PCM形式的声音数据)
    short  channelnum;    //通道数，单声道为1，双声道为2
	long int  samplerate; //采样率（每秒样本数），表示每个通道的播放速度
    long int transferrate;//波形音频数据传送速率，其值为通道数×每秒数据位数×每样
                          //本的数据位数／8。播放软件利用此值可以估计缓冲区的大小
    short int adjustnum;  //数据块的调整数（按字节算的），其值为通道数×每样本的数据位值／8。
	                      //播放软件需要一次处理多个该值大小的字节数据，以便将其值用于缓冲区的调整。
	short int databitnum; //每样本的数据位数，表示每个声道中各个样本的数据位数
	char datasign[4];     //数据标记符＂data＂
	long int datalength;  //语音数据的长度(字节）
}wavhead;

void rm_dc(short sample[],long int dataszize);
//消除直流分量
//============================================================================
//读取wav文件
void waveread(char filename[],int *FS,long int *size,short **wavdata)
{
	wavehead wavhead;
	FILE *fp;
	if((fp=fopen(filename,"rb"))==NULL)
	{
		printf("cannot read wave file\n");
		exit(0);
	}
	int headnum;
	headnum=sizeof(wavhead);
	//WAVE文件头00H~28H.
	fread(&wavhead,headnum,1,fp);
	short int bitnum;
	long int datasize;
	//分别定义采样率，语音数据的长度，每样本的数据位数。
	*FS=wavhead.samplerate;
	bitnum=wavhead.databitnum;
	datasize=wavhead.datalength;
	*size=datasize/sizeof(unsigned char);
	//得到所有数据样点的个数，每个样点用short int表示。
    //printf("The total wave data length:%d\n",datasize/sizeof(short int));
	//printf("The sample rate is:");
	//printf("%d\n",*FS);
	//int tt;
	//float total_t=float(datasize)/((*FS)*2);
	unsigned char *data=new unsigned char[datasize/sizeof(unsigned char)];
	//WAVE文件的每个样本值包含在一个整数i中
	//存取得到的数据;
	//16位PCM int 32768 
	long int offset=sizeof(wavehead);
	fseek(fp,offset,0);
	//要求位移量是long型数据
	//short int samplenew=0;
	//unsigned char samplenew;
	//long int size_i=0;
	//while(fread(&samplenew,1,1,fp)==1)
	//{
	//	*data++=samplenew;
	//	//size_i++;
	//}
	if(fread(data,datasize,1,fp)!=1)
	{
		if(feof(fp)) exit(0);
		printf("filereaderror\n");
	}
	short int *datashort=new short int[datasize/sizeof(unsigned char)];
	for (long int i=0;i<*size;i++)
	{
		*(datashort+i)=*(data+i);
	}
	rm_dc(datashort,datasize);
	delete []data;
	//消除直流分量
	*wavdata=datashort;
	fclose(fp);
}

void WavHeadRead(char filename[],int *FS,long int *size)
{
	wavehead wavhead;
	FILE *fp;
	//char filename[]="E:\\SpeechNoise\\F07B522.wav";
	//char filename[]="E:\\noise\\white.wav";
	//char filename[20];
	//printf("Please input the wave filename:");
	//scanf("%s",filename);
	if((fp=fopen(filename,"rb"))==NULL)
	{
		printf("cannot read wavehead file\n");
		exit(0);
	}
	int headnum;
	headnum=sizeof(wavhead);
	//WAVE文件头00H~28H.
	fread(&wavhead,headnum,1,fp);
	short int bitnum;
	long int datasize;
	//分别定义采样率，语音数据的长度，每样本的数据位数。
	*FS=wavhead.samplerate;
	bitnum=wavhead.databitnum;
	datasize=wavhead.datalength;
	*size=datasize/sizeof(short int);
	//得到所有数据样点的个数，每个样点用short int表示。
	printf("The total wave data length:%d\n",datasize/sizeof(short int));
	//printf("The sample rate is:");
	//printf("%d\n",*FS);
	//int tt;
	//float total_t=float(datasize)/((*FS)*2);

	fclose(fp);
}

//对于8位PCM情况如下：
//8位PCM unsigned int 225 0 
//归一化dat.Data = (dat.Data-128)/128;  [-1,1)
/* remove DC by removing the average of the whole utterance */
void rm_dc(short sample[],long int datasize)
{
  long i,total=0;
  long no_samples=datasize/sizeof(unsigned char);
  for(i=0;i<no_samples;i++)
     total+=sample[i];
  total/=no_samples;
  for(i=0;i<no_samples;i++)
     sample[i]-=(short)total;
}


long int wavSamleRate(char filename[])
{
	wavehead wavhead;
	FILE *fp;
	if((fp=fopen(filename,"rb"))==NULL)
	{
		printf("cannot read wave file\n");
		exit(0);
	}
	int headnum;
	headnum=sizeof(wavhead);
	//WAVE文件头00H~28H.
	fread(&wavhead,headnum,1,fp);
	//short int bitnum;
	long int datasize;
	//分别定义采样率，语音数据的长度，每样本的数据位数。
	datasize=wavhead.datalength;
	fclose(fp);
	return datasize/sizeof(unsigned char);

}
