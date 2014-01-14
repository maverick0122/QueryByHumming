#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include<iostream>
#include "define.h"
//using namespace std;


float detect_pitch(short *sdata,int Fs)
{
	float *RR;
	float Rmax;
	int Imax;
	short i;
	short LF,HF;
	short clip;
	float pitch;
	float silence;
	float *data;
	data=(float*)malloc(sizeof(float)*FLENGTH);
	RR=(float *)malloc(FLENGTH*sizeof(float));
 
	filter(sdata,data);

    clip=findclip(data);
    doclip(data,clip);

	silence=0.4*v_inner(data,data,FLENGTH);
 	xcorr(data,RR);

	LF=(short)floor(double(Fs/320));
	HF=(short)floor(double(Fs/60));
	Rmax=0;Imax=0;
    for(i=LF;i<HF;i++) // find max 
	{
		if(RR[i]>Rmax)
		{
			Rmax=RR[i];
            Imax=i;
		}
	}
	if(Imax==0)
		Imax=20;
	pitch=Fs/Imax;
    if ((Rmax > silence)  && (pitch >60) && (pitch <320))
        pitch = pitch;
    else          // -- its unvoiced segment ---------
        pitch = 0;
    
	return pitch;

}

float findclip(float *data)
{ 
 	float max1=0;
    float max2=0;
	int onethird;
	int twothird;
	short i;
	float clip;
	onethird=(int)floor(double(FLENGTH/3));
	twothird=onethird*2;
	for(i=0;i<onethird;i++)
	{
		if(abs(data[i])>max1)
			max1=abs(data[i]);
	}	
    for(i=twothird;i<FLENGTH;i++)
	{
		if(abs(data[i])>max2)
			max2=abs(data[i]);
	}	
	if(max1>max2)
		clip=0.68*max2;
	else
		clip=0.68*max1;
	return clip;
}

void xcorr(float *data,float *RR)     //qiu chu zixiangguanzhi de xulie RR[FrameLen*2-1]
{
	int i,j;
	for(i=0;i<FLENGTH;i++)
	{   
		RR[i]=0.0;
		for(j=0;j<FLENGTH-i;j++) //i+1 biaoshi jigeshuchonghe 
			RR[i]+=data[i+j]*data[j];
	}
}

void doclip(float *data,float clip)
{
	short i;
    for(i=0;i<FLENGTH;i++)
	{
		if ((data[i]>clip)||(data[i]<(-1)*clip))
			data[i]=data[i]-clip;
		else 
			data[i]=0;
	}
}
/*void polflt(float input[],float coeff[],float output[],int ORDER,int npts)
{
	int i,j;
	float accum;
	for(i=0;i<npts;i++)
	{
		accum=input[i];
		for(j=1;j<=ORDER;j++)
			accum-=output[i-j]*coeff[j];
		output[i]=acum;
	}
}
void zerflt(float input[],float coeff[],float output[],int ORDER,int npts)
{
	int i,j;
	float accum;
	for(i=npts-1;i>=0;i--)
	{
		accum=0.0;
		for(j=0;j<=ORDER;j++)
			accum+=output[i-j]*coeff[j];
		output[i]=acum;
	}
}*/
float v_inner(float *v1,float *v2,int n)
{
	int i;
	float innerprod;
	innerprod=0.0;
	for(i=0;i<n;i++)
		innerprod+=v1[i]*v2[i];
	return innerprod;
}

void filter(short input[],float output[])
{
	int i,j;
	float *inputbuff,*outputbuff;
	inputbuff=(float*)malloc(sizeof(float)*(FLENGTH+ORDER));
	outputbuff=(float*)malloc(sizeof(float)*(FLENGTH+ORDER));
	//allocate and distribute
	for (i=0;i<FLENGTH;i++)
		inputbuff[ORDER+i]=input[i];
	for (i=0;i<ORDER;i++)
	{
		inputbuff[i]=0;
        outputbuff[i]=0;
	}
    // filter inputbuff to outputbuff
	for(i=ORDER;i<FLENGTH+ORDER;i++)
	{   
		outputbuff[i]=0;
		for(j=0;j<=ORDER;j++)
            outputbuff[i]+=bf[j]*inputbuff[i-j];
		for(j=1;j<=ORDER;j++)
			outputbuff[i]-=af[j]*outputbuff[i-j];
	}
    // output is result
    for(i=0;i<FLENGTH;i++)
	output[i]=outputbuff[i+ORDER];
}
