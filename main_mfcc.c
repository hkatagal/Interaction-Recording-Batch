//----------------------------------------------------------------------------
// C main line
//----------------------------------------------------------------------------
//Project 3 Final Code
#include <m8c.h>        // part specific constants and macros.
#include "PSoCAPI.h"    // PSoC API definitions for all User Modules.
#include "stdio.h"
#include "fft.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "libmfcc.h"
#include "kmeans.h"

extern unsigned int i_msec;
extern unsigned int i_sec;
extern unsigned int i_min;
extern unsigned int i_hour;
extern unsigned int cnt_sec;
extern unsigned int cnt_min;
extern unsigned int cnt_hour,dispCount;
extern unsigned int downCount,dumCount;



int test_int= 0;
unsigned int mat_flag=0;
unsigned int downcountFlag = 0;
unsigned int dispflag;

//List of Functions used in the Code.
void LCD_Clear(void); 
void LCD_Time(void);
void serialDataOut(void);
void getCalibrationData(void);
void getFFTOutput(void);
int  compare(void );
void matlabOut(void);

unsigned int gblSec =0;gblMin=0;gblHour=0,calibFlag=0;
unsigned int mode = 0,noConv=0; // 0 - Free Run (Default) ; 1 - Caliberation ; 2 - Matching.
unsigned int avgFirstCal,avgFirstMat;
unsigned int mode_check_count = 0;
unsigned int temp_value = 0;
unsigned int temp_mode = 0,dumflag=0;
int  max;
int compMax, compFreq, lclMax, lclFreq;
double hannMul, hannValue,hannValue1;
unsigned int h=0,j=0,m=0;         //index
unsigned int ind; 
unsigned int sampFreq=13330;
double  freq,avg_Freq=0,avg_Mat=0,noMatch=0;    // time record
float mfcc[13];
int kmean_dat[4];
double maxf;
unsigned int avgCalNoise;

void main(void)
{
	char str[15];
	int a=0,i =0,mode_flag =0,isMatch;
	LCD_Start();
	UART_Start(UART_PARITY_NONE);
	PGA_1_Start(PGA_1_HIGHPOWER);
	PGA_2_Start(PGA_2_HIGHPOWER);
	LPF2_3_Start(LPF2_3_HIGHPOWER);
	PGA_3_Start(PGA_3_HIGHPOWER);
	PGA_4_Start(PGA_4_HIGHPOWER);
	LPF2_1_Start(LPF2_1_HIGHPOWER);
    M8C_EnableGInt;                     // Enable global interrupts	
	Timer_Start();
	LCD_Clear();
	Timer_EnableInt();
	DUALADC_Start(DUALADC_HIGHPOWER);     // Turn on Analog section
    DUALADC_GetSamples(0);
	freq=104;
    i=0;
	LCD_Clear();
	while (1)
	{
		temp_value = PRT0DR & 0x01;
		if((mode_check_count >= 5) && (mode_check_count <= 10)&& (temp_value == 0))
		{	
			mode_flag=2;
			mode_check_count=0;
		}
		else if (((mode_check_count < 5) && (mode_check_count >= 2)&& (temp_value == 0)) || mode_flag == 1)
		{
			mode_flag =1;
			mode_check_count=0;
		}
		else 
		{
			mode_flag=0;
		}                                     
		if (mode_flag == 0)
		{
			PRT1DR=0x01;
			LCD_Time();
			cnt_sec = 0;
			cnt_min = 0;
			cnt_hour = 0;
		}
		if (mode_flag == 1)
		{
			PRT1DR=0x04;
			LCD_Position(1,0);
			LCD_PrCString("Mode:Matching   ");
			if (calibFlag == 1)
			{
				isMatch = compare();
				if(isMatch == 2)
				{
					mat_flag = 1;
					noMatch = 0;
					
				}
				else if(isMatch ==1)
				{
					noMatch++;
				}
				else if (isMatch == 3)
				{
					LCD_Clear();
					mat_flag = 0;
					noMatch = 0;
					mode_flag = 0;
					if(cnt_sec > 0 || cnt_min > 0 || cnt_hour > 0)
					{
						noConv++;
						gblSec = gblSec + cnt_sec;
						if(gblSec > 60)
						{
							gblMin++;
							gblSec = gblSec % 60;
						}
						gblMin += cnt_min;
						if(gblMin > 60)
						{
							gblHour++;
							gblMin = gblMin % 60;
						}
						gblHour = gblHour + cnt_hour;
					}
					LCD_Clear();
				}
				if(noMatch == 1)
				{
					mat_flag = 0;
					noMatch = 0;
					mode_flag = 0;
					if(cnt_sec > 0 || cnt_min > 0 || cnt_hour > 0)
					{
						noConv++;
						gblSec = gblSec + cnt_sec;
						if(gblSec > 60)
						{
							gblMin++;
							gblSec = gblSec % 60;
						}
						gblMin += cnt_min;
						if(gblMin > 60)
						{
							gblHour++;
							gblMin = gblMin % 60;
						}
						gblHour = gblHour + cnt_hour;
					}	
					LCD_Clear();
				}			
			}
			else 
			{
				LCD_Position(1,0);
				LCD_PrCString("Pls Calibrate   ");
				mode_flag = 0;
				dumflag=1;
				while (dumCount != 0);
				dumflag=0;
				dumCount = 5;
				LCD_Clear();
				
			}
			
		}
			
		if (mode_flag == 2)
		{
			PRT1DR=0x02;
			LCD_Time();
			LCD_Position(1,0);
			LCD_Clear();
			LCD_PrCString("Mode:Calibration");
			getCalibrationData();
			mode_flag = 0;
			LCD_Clear();
		}
	}
}

//Function to display time statistics
void LCD_Time(void)
{
	char temp[4];
	//LCD_Clear();
	LCD_Position(0,0);
	LCD_PrCString("H:");
	LCD_Position(0,2);
	itoa(temp,gblHour,10);
	LCD_PrString(temp);
	LCD_Position(0,5);
	LCD_PrCString("M:");
	LCD_Position(0,7);
	itoa(temp,gblMin,10);
	LCD_PrString(temp);
	LCD_Position(0,10);
	LCD_PrCString("S:");
	LCD_Position(0,12);
	itoa(temp,gblSec,10);
	LCD_PrString(temp);
	LCD_Position(1,0);
	LCD_PrCString("Length:");
	LCD_Position(1,9);
	itoa(temp,noConv,10);
	LCD_PrString(temp);
	LCD_Position(1,14);
	LCD_PrCString("FR");
}	

//Function which reads data from ADC and computes FFT.
void getFFTOutput(void)
{
	int i,Status;
	char temp[4],*temp1;
	for(i=0;i<N_points;i++)
    {  	
       	mod[i]=0;       //init 0
		//mod1[i]=0;
       	data_re[i]=0;
		data_re1[i]=0;
       	data_imm[i]=0;
		data_imm1[i]=0;
    }
	i=0;
		while(i!= N_points) // Wait for data to be ready. 
		{                                       
    	while(DUALADC_fIsDataAvailable() == 0); 
		hannValue = DUALADC_iGetData1();
		hannValue1 = DUALADC_iGetData2ClearFlag();
		hannMul = 0.5*(1-cos(0.0494*i));
		hannValue = hannMul * hannValue;
		hannValue1 = hannMul*hannValue1;
		data_re[i] = hannValue;
		data_re1[i] = hannValue1;
		DUALADC_ClearFlag();
		i++;
		}
		FFT(1,exponent,data_re,data_imm,mod);
		serialDataOut();

	return ;
}

//Function to stream FFT output serially
void serialDataOut(void )
{
	char temp[4],*temp1;
	int i,Status;
	UART_CPutString("\n\rFFT OUTPUT:\n\r");
	for (i=0;i<N_points;i++)
	{
		
		UART_CPutString("{");
		//itoa(temp,mod[i],10);
		temp1 = ftoa(mod[i], &Status);
		UART_PutString(temp1);
		//UART_PutString(temp);
		UART_CPutString("},");
		
	}
	UART_CPutString("\n\r");
	
}

//Function to clear the LCD.
void LCD_Clear(void)
{
	LCD_Position(0,0);
	LCD_PrCString("                ");	
	LCD_Position(1,0);
	LCD_PrCString("                ");	
}

void getCalibrationData(void)
{
	int i,y,avgTemp = 0;
	int a=0,noiseSamp=20;
		int Status;
	char *temp1,temp[4];
	avgCalNoise = 0;
	LCD_Clear();
	LCD_Position(0,0);
	LCD_PrCString("Calibrating");
	LCD_Position(1,0);
	LCD_PrCString("Noise");
	for (i=0;i<noiseSamp;i++)
	{
		getFFTOutput();
		for(y=0;y<N_points;y++)
			avgTemp = avgTemp + mod[y];
		avgTemp = avgTemp/N_points;
		avgCalNoise += avgTemp;
	}
	avgCalNoise = (avgCalNoise*2)/noiseSamp;
	LCD_Position(1,0);
	LCD_PrCString("Voice");

	getFFTOutput();
	for (i = 0; i < 13; i++){
		LCD_Position(0,0);
		LCD_PrHexInt(i);
		mfcc[i] = GetCoefficient(mod, 46867, 48, 64, i);
	}
	UART_CPutString("\n\rMFCC OUTPUT:\n\r");
	for (i=0;i<13;i++)
	{
		
		UART_CPutString("{");
		
		temp1 = ftoa(mfcc[i], &Status);
		UART_PutString(temp1);
		//UART_PutString(temp);
		UART_CPutString("},");
	}
	
	UART_CPutString("\n\rKmean output:\n\r1");
	kmeans(4,mfcc,13,4,mfcc,kmean_dat);
	UART_CPutString("\n\rKmean output:\n\r1");
	for(i=0;i<4;i++)
	{
		
		UART_CPutString("{");
		itoa(temp,kmean_dat[i],10);
		UART_PutString(temp);
		UART_CPutString("},");
	}
}


int compare(void)
{
	int i, compInd,avgTemp=0,p,count=0;
	int a=0,modeFlag=0,y=0,avgFirstFlag, avgMaxFlag;
	int noSamp = 10;
	avgFirstMat = 0;
	while(a != noSamp)
	{
		while(avgTemp < avgCalNoise)
		{
			avgTemp = 0;
			getFFTOutput();
			for (i=0;i<N_points;i++)
				avgTemp = avgTemp + mod[i];
			avgTemp = avgTemp/N_points;
			downcountFlag = 1;
			if(downCount == 5)
			{
				LCD_Position(0,0);
				LCD_PrCString("5 Seconds");
			}
			if (downCount == 0)
			{
				downcountFlag =0;
				downCount = 10;
				LCD_Clear();
				LCD_Position(0,0);
				LCD_PrCString("******Fail******"); 
				LCD_Position(1,0);
				LCD_PrCString("Back to Free Run");
				dumflag=1;
				while (dumCount != 0);
				dumflag=0;
				dumCount = 5;
				return 3;
			}
		}
		avgTemp =0;
		downcountFlag =0;
		downCount=10;
		a++;
	}
	getFFTOutput();
	for (i = 0; i < 13; i++){
		mfcc[i] = GetCoefficient(mod, 46867, 48, 64, i);
	}
	kmeans(4,mfcc,13,4,mfcc,kmean_dat);
	return 1;
}
