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
void LCD_Clear(void); 
void getFFTOutput(void);
void matlabOut(void);
void cordic1(void);
void max_likelihood(void);
void soundLocalize(void );
void serialDataOut(void );
void matlabOut(void );


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
double ar1[32],ar2[32];
double  freq,avg_Freq=0,avg_Mat=0;    // time record
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
	LCD_Position(1,0);
	LCD_PrCString("hELLO");
	
	UART_CPutString("\n\rFFT OUTPUT:\n\r");
	soundLocalize();
	matlabOut();
}

void getFFTOutput(void)
{
	int i,Status,flag=0;
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
			if (hannValue > 0x0040 || flag == 1)
			{
				
				flag =1;
				hannMul = 0.5*(1-cos(0.0494*i));
				hannValue = hannMul * hannValue;
				hannValue1 = hannMul*hannValue1;
				data_re[i] = hannValue;
				data_re1[i] = hannValue1;
				DUALADC_ClearFlag();
				i++;
			}
		}
		FFT(1,exponent,data_re,data_imm,mod);
		serialDataOut();

	return ;
}


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

void matlabOut(void )
{
	int i,j,Status;
	char *temp1;
	for(i=0;i<4;i++)
	{
		for(j=0;j<512;j++)
		{
			while(DUALADC_fIsDataAvailable() == 0); 
			hannValue = DUALADC_iGetData1ClearFlag();
			hannMul = 0.5*(1-cos(0.0494*j));
			hannValue = hannMul * hannValue;
			temp1 = ftoa(hannValue, &Status);
			UART_PutString(temp1);
			UART_CPutString("\n\r");
			
		}
	}
}

void LCD_Clear(void)
{
	LCD_Position(0,0);
	LCD_PrCString("                ");	
	LCD_Position(1,0);
	LCD_PrCString("                ");	
}

void soundLocalize(void )
{
	getFFTOutput();
	FFT(1,exponent,data_re1,data_imm1,mod);
	serialDataOut();
	cordic1();
	max_likelihood();
	
}
	
void cordic1(void )			//CORDIC
{
	int neg, offset,value,i;
	
	for(i=0;i<32;i++)
	{		
	neg=0;
	offset=0;
	value=0;
	
	if((data_imm[i]<0) && (data_re[i]<0))
	{
		data_re[i]=-data_re[i];
		data_imm[i]=-data_imm[i];
		offset=3.142;
	}
	else if((data_imm[i]<0) && (data_re[i]>0))
	{
		data_imm[i]=-data_imm[i];
		neg=1;
		offset=3*1.571;
	}
	else if((data_imm[i]>0) && (data_re[i]<0))
	{
		data_re[i]=-data_re[i];
		neg=1;
		offset=1.571;
	}

	
	if((data_imm[i]==0) && (data_re[i]>0))
	{
		ar1[i]=0;
	}
	else if((data_imm[i]==0) && (data_re[i]<0))
	{
		ar1[i]=3.142;
	}
	else if((data_imm[i]>0) && (data_re[i]==0))
	{
		ar1[i]=1.571;
	}
	else if((data_imm[i]<0) && (data_re[i]==0))
	{
		ar1[i]=3*1.571;
	}
	else
	{
		
		ar1[i]=atan(data_imm[i]/data_re[i]);
		//ar1[i]=atan_cordic(x1[i],z1[i]);
		if(neg==1)
			ar1[i]=1.571-ar1[i];
		ar1[i]=ar1[i]+offset;
	}
	
	
	if((data_imm1[i]==0) && (data_re1[i]>0))
	{
		ar2[i]=0;
	}
	else if((data_imm1[i]==0) && (data_re1[i]<0))
	{
		ar2[i]=3.142;
	}
	else if((data_imm1[i]>0) && (data_re1[i]==0))
	{
		ar2[i]=1.571;
	}
	else if((data_imm1[i]<0) && (data_re1[i]==0))
	{
		ar2[i]=3*1.571;
	}
	else
	{
		ar2[i]=atan(data_imm1[i]/data_re1[i]);
		//ar2[i]=atan_cordic(x1[i],z1[i]);
		if(neg==1)
			ar2[i]=1.571-ar2[i];
		ar2[i]=ar2[i]+offset;
	}
	
	
	}
	LCD_Clear();
	LCD_Position(0,0);
	LCD_PrCString("Exiting cordic");
}



//x1  --> data_re
//x2 --> data_imm
//z1 --> mod
//z2 --> data_re1
void max_likelihood(void)
{
int i,n,q,Status;
	double phin,max,qmax,ph, temp;
	double doa;
	char *temp1;
	double value;
for(i=0;i<31;i++)
	data_re[i]=ar1[i]-ar2[i];



for(i=0;i<64;i++)
{
data_imm[i]=0;
mod[i]=0;
data_re1[i]=0;
}

phin=-0.397604;
max=0;
qmax=0;

for(q=0;q<162;q++)
{
	phin = phin + 0.0049087;
	
	for(n=0;n<31;n++)
	{
//		i=n;
//		ph=0;
//		for(i=0;i<n;i++)
//			ph = ph + phin;
		
		ph = phin * n;
		if(q<64)
		{
			data_imm[q]=data_imm[q]+cos(data_re[n]-ph);
			temp = data_imm[q];
		}
		
		if(q>63)
		{
		if(q<128)
		{
			mod[q-64]=mod[q-64]+cos(data_re[n]-ph);
			temp = mod[q-64];
		}
		}
		
		if(q>127)
		{
			data_re1[q-128]=data_re1[q-128]+cos(data_re[n]-ph);
			temp = data_re1[q-128];
		}
		LCD_Position(1,8);
		LCD_PrHexInt(n);
		
	}
	
	if(temp>max)
	{
		max=temp;
		qmax=q;
	}
	LCD_Position(1,0);
	LCD_PrHexInt(q);
}

qmax=qmax-81;
qmax=qmax/10;
value = qmax*0.12937823;			//345) / (0.20*13333);
if (value > 1)
    value = 1;
if (value < -1)
    value = -1;
UART_CPutString("\n\rBefore Sine\n\r");
doa = asin(value); 
temp1 = ftoa(doa, &Status);
UART_PutString(temp1);
doa = doa*57.29577;
UART_CPutString("\n\rAfter Multiplication\n\r");
temp1 = ftoa(doa, &Status);
UART_PutString(temp1);
//doa = doa*100;
LCD_Clear();
UART_CPutString("\n\r");
if(doa<0)
{
doa=-1*doa;
LCD_Position(1,11);
LCD_PrCString("n");
temp1 = ftoa(doa, &Status);
UART_PutString(temp1);
LCD_Position(1,12);
LCD_PrHexInt(doa);
}
else
{
LCD_Position(1,11);
LCD_PrCString("p");
temp1 = ftoa(doa, &Status);
UART_PutString(temp1);
LCD_Position(1,12);
LCD_PrHexInt(doa);
}

}
































