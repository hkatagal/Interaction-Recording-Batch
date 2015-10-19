#pragma interrupt_handler Timer_ISR

/* Timer ISR in C where timer interrupts are processed */
unsigned int i_msec = 0;
unsigned int i_sec = 0;
unsigned int i_min = 0;
unsigned int i_hour = 0;
unsigned int cnt_sec = 0;
unsigned int cnt_min = 0;
unsigned int cnt_hour = 0;
unsigned int downCount = 10;
unsigned int dumCount = 5;
unsigned int dispCount = 2;

extern unsigned int mat_flag;
extern unsigned int temp_value;
extern unsigned int mode_check_count;
extern unsigned int downcountFlag;
extern unsigned int dumflag;
extern unsigned int dispflag;

void Timer_ISR(void)
{
	i_msec = i_msec + 1;
	
	asm (" NOP");
	asm (" NOP");
	
	if (i_msec >= 1000)
	{
		i_sec = i_sec + 1;
		i_msec = 0;
		if (mat_flag == 1)
			cnt_sec = cnt_sec + 1;
		if (dumflag == 1)
			dumCount--;
		if (temp_value == 1)
			mode_check_count = mode_check_count + 1;
		if (mode_check_count >= 10)
			mode_check_count = 0;
		if (downcountFlag == 1)
			downCount--;
		if	(dispflag == 1)
			dispCount--;
	}
	
	asm (" NOP");
	asm (" NOP");
	
	if ( i_sec >= 60)
	{
		i_min = i_min + 1;
		i_sec = 0;
	}

	if ( cnt_sec >= 60)
	{

		cnt_min = cnt_min + 1;
		cnt_sec = 0;
	}


	asm (" NOP");
	asm (" NOP");
	
	if (i_min >= 60)
	{
		i_hour = i_hour + 1;
		i_min = 0;
	}
	if (cnt_min >= 60)
	{
		cnt_hour = cnt_hour + 1;
		cnt_min = 0;
	}
	
	asm (" NOP");
	asm (" NOP");
	
	if (i_hour == 99)
		i_hour = 0;			// Re Starting count after 99 hours
	if (cnt_hour == 99)
		cnt_hour = 0;	
	
	asm (" NOP");
	asm (" NOP");
}
