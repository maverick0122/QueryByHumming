/**																	**/
/*********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include<iostream>
#include "define.h"

int pitchana(char *wavename,int **pitchnumParam,int *FrameBegin)
{

    FILE* fp;
	//fp=fopen(wavename,"w+");
	int   sample_rate;      //Sample rate of raw data
    short int   *Sdata;     //Sdata means data from one session.
	long  int   Slength;    //Length of raw data
	unsigned int   nFrame;  //Number of frames
    float pitch,pitchtemp,*pitcharray;
    short *framedata;
	short i,j;
	float avgF0 = 0;
	int * pitchnum;
    framedata=(short*)malloc(FLENGTH*sizeof(short));
	double energy=0;
	waveread(wavename, &sample_rate, &Slength, &Sdata);	//sample_rate表示采样率；
	                                                    //Slength表示一个session的数据长度。
	for (int k=0;k<Slength;k++)
	{
		energy+=Sdata[k]*Sdata[k];
	}
	energy/=Slength;
	bool beginEnergy=true;
	
	
	
	nFrame=(Slength-FLENGTH)/FSHIFTW+1;
	pitcharray=(float*)malloc(nFrame*sizeof(float));
	pitchnum=(int*)malloc(nFrame*sizeof(int));
	for(i=0;i<nFrame;i++)
	{
		for(j=0;j<FLENGTH;j++)
			framedata[j]=Sdata[FSHIFTW*i+j];
		float frameEnergy=0;
		if (beginEnergy)
		{
			for (int k=0;k<FLENGTH;k++)
			{
				frameEnergy+=framedata[k]*framedata[k];
			}
			frameEnergy/=FLENGTH;
			if (frameEnergy>energy*0.4)
			{
				*FrameBegin=i;
				beginEnergy=false;
			}
		}
		

		pitch=detect_pitch(framedata,sample_rate);
		//pitch=Mydetect_pitch(framedata,sample_rate,energy);
	    pitcharray[i]=pitch;
		/*
		//do some median filtering 
		if(i>2){
		{
			pitchtemp=(pitcharray[i-2]+pitcharray[i-1]+pitch)/3;
		}
		if(abs(pitchtemp-pitcharray[i-2])>3)
			pitcharray[i-2]=0;
	
		}
		*/
		*(pitchnum+i)=pitcharray[i];
		//fprintf(fp,"%.3f",pitcharray[i]);
		//fputc('\n',fp);

	}
	*pitchnumParam=pitchnum;
	free(pitcharray);
	free(framedata);
	delete []Sdata;
	return Slength;
}
