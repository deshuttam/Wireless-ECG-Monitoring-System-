//----------------------------------------------------------------------------
// C main line
//----------------------------------------------------------------------------

#include <m8c.h>        // part specific constants and macros
#include "PSoCAPI.h"    // PSoC API definitions for all User Modules
#include "stdlib.h" 
#include <math.h> 
#include "ports.h" 
#include  "DigBuf_2.h"  

#define CR 0x0D   // carriage return
#define LF 0x0A   // line feed
#define SS 0x20

char pulse_lcd[5];  // Define RAM string 

int nADCdata, lpf_data, lpf_data1, lpf_data2, lpf_data3, lpf_data4, lpf_data5, hpf_data; 
unsigned int g_time =0;        // time counter
unsigned int g_time_old = 0;   // time counter when rqs detected    
unsigned int Q_threshold_low = 20;  // low threshold value 
unsigned int prev_Q_threshold_low = 90;
unsigned int rr_array_counter = 0;  // dimention of median filter
unsigned int t_pulse_rate = 0;  // pulse rate 
unsigned int rr = 0;
unsigned char arr = 0;
unsigned int rr_interval[11] = {0,0,0,0,0,0,0,0,0,0,0};
unsigned int counter_1200_msec = 0;  // the time after rqs detected
BOOL counter_1200_msec_enable = FALSE;
BOOL noise_detected = FALSE;
first_af = TRUE;
unsigned int max_200_msec; // the maximum value after 200 msec when rqs detected
unsigned int threshold_counter = 0;
unsigned int tx_counter = 0;
unsigned char temp;

//----------------------------------------------------------

void delay_sec(int sec)
{
int i,j,secd;
for (secd=0;secd<=sec;secd++)
for(i=0;i<=1;i++)
{
for (j= 0;j<=10;j++);
}
}

void SPIM(unsigned char tx)
{
unsigned char transmit = 0;
BYTE cRXData;
transmit=tx;

//PRT0DR &= ~SS;
	Port0_5(0);

	SPIM_SendTxData(transmit);


		
	while(!(SPIM_bReadStatus() & SPIM_SPIM_RX_BUFFER_FULL|SPIM_SPIM_SPI_COMPLETE)){};
	cRXData = SPIM_bReadRxData();
	LCD_1_Position(0,12);            // Place LCD cursor at .
	LCD_1_PrCString("RX");
	LCD_1_PrHexByte(cRXData);
	Port0_5(1);
	//DigBuf_2_Stop();
	}


int hpf1 (int x) // first order high pass filter f = 30 hz  (differentiator)
{ 
  static long x_hpf1_1, y_hpf1_1;
  int y;
  y = ( (((x - x_hpf1_1)<< 16)>> 1) + (y_hpf1_1 << 10)) >> 16;
  x_hpf1_1 = x;
  y_hpf1_1 = y;
  return y;
}


//----------------------------------------------------------
int lpf (int x) // 
{ 
  static long xn_1, xn_2, yn_1, yn_2;
   long diff1;
  int y;
  diff1 = yn_1<<16; 
  y = ( (((x + xn_2)<<16)>>4) + ((xn_1<<16)>>3) + (diff1>>1) + (diff1>>2) - (((yn_2)<<16)>>2) )>>16;
  xn_2 = xn_1;
  xn_1 = x;
  yn_2 = yn_1;
  yn_1 = y;
  return y;
}


//----------------------------------------------------------
int lpf1 (int x) // 
{ 
  static long xn_1, xn_2, yn_1, yn_2;
   long diff1;
  int y;
  diff1 = yn_1<<16; 
  y = ( (((x + xn_2)<<16)>>4) + ((xn_1<<16)>>3) + (diff1>>1) + (diff1>>2) - (((yn_2)<<16)>>2) )>>16;
  xn_2 = xn_1;
  xn_1 = x;
  yn_2 = yn_1;
  yn_1 = y;
  return y;
}

//----------------------------------------------------------
int lpf2 (int x) // 
{ 
  static long xn_1, xn_2, yn_1, yn_2;
   long diff1;
  int y;
  diff1 = yn_1<<16; 
  y = ( (((x + xn_2)<<16)>>4) + ((xn_1<<16)>>3) + (diff1>>1) + (diff1>>2) - (((yn_2)<<16)>>2) )>>16;
  xn_2 = xn_1;
  xn_1 = x;
  yn_2 = yn_1;
  yn_1 = y;
  return y;
}

//----------------------------------------------------------
int lpf3 (int x) // 
{ 
  static long xn_1, xn_2, yn_1, yn_2;
   long diff1;
  int y;
  diff1 = yn_1<<16; 
  y = ( (((x + xn_2)<<16)>>4) + ((xn_1<<16)>>3) + (diff1>>1) + (diff1>>2) - (((yn_2)<<16)>>2) )>>16;
  xn_2 = xn_1;
  xn_1 = x;
  yn_2 = yn_1;
  yn_1 = y;
  return y;
}
//----------------------------------------------------------
unsigned char af (int xa) // four order averaging filter   
{ 
  unsigned int ya = 0;
  static unsigned int x_af_1;
  if (first_af) { x_af_1 = xa; first_af = FALSE;}
  ya = (xa + x_af_1) >> 1;
  x_af_1 = xa;
  return ya;
}

//----------------------------------------------------------
int af2 (int xa) // two order averaging filter   
{ 
 unsigned int ya = 0;
 static unsigned int x_af2_1;
 ya = (xa + x_af2_1) >> 1;
 x_af2_1 = xa;
 return ya;
}

int median_filter(unsigned int *x)
{
unsigned int k; // current element of array
unsigned int buf;// buffer for rearrangements of elements of array 
BOOL changed = TRUE; // TRUE, if in the current cycle the rearrangements are occured
// array sorting

while (changed){
				changed = FALSE; // let there are no rearrangements in the current cycle
				// (k=0; k<=9; k++)
					for (k=0; k<=3; k++)
									{  
									if (x[k+1] < x[k]) {  // rearrangement of k and k+1 elements
  														buf = x[k];
  														x[k] = x[k+1];
  														x[k+1] = buf;
  														changed = TRUE;
  				   										}
  				   					}
				}
				if (((x[3] - x[1])<<2) > x[2]) return 0;   // return error
				return (x[2]);
//if (((x[6] - x[4])<<4) > x[5]) return 0;   // return error 			
}




void main()
{
 //CMPPRG_1_Start(CMPPRG_1_LOWPOWER);
 //ACB02CR3 |= 0x08;   // set low current comparator
 
 	
//ACB03CR2 &= 0xE3;                //Enable the AGND output on the second column (Chan3)
//ACB03CR2 |= 0x14;
//INSAMP_1_Start(INSAMP_1_HIGHPOWER);
//LPF2_1_Start(LPF2_1_HIGHPOWER);
SCBLOCK_1_Start (SCBLOCK_1_HIGHPOWER);
PGA_1_Start(PGA_1_HIGHPOWER);
ADCINC14_1_Start(ADCINC14_1_HIGHPOWER);  
SPIM_Start(SPIM_SPIM_MODE_0|SPIM_SPIM_LSB_FIRST); //Mode 0 LSB first
SPIM_EnableInt();
ADCINC14_1_GetSamples(0);
DigBuf_2_EnableInt();  
DigBuf_2_Start();


//start_init: 
Port1_2(1);
LCD_1_Start();                  // Initialize LCD  
 LCD_1_Position(1,0);            // Place LCD cursor at row 0, col 0.
 LCD_1_PrCString("PSoC  pulsometer");   // Print "PSoC  pulsometer" on the LCD  ;
 LCD_1_Position(0,0);       // Place LCD cursor at row 0, col 0.
 LCD_1_PrCString("  Please wait  ");  // Print "Please wait" on the LCD  ;
TX8_1_Start(TX8_1_PARITY_NONE);
M8C_EnableGInt;       // Enable global interrupts 

  
start_init:
	// A loop repeatedly reads the ADC when valid 
 
//while((CMP_CR0 & 0x40) == 0x40) 
while(1)
{ 
if(ADCINC14_1_fIsDataAvailable()) // Wait for data to be ready  
{
   nADCdata=ADCINC14_1_iGetDataClearFlag(); // Get Data
   	  nADCdata=ADCINC14_1_iGetDataClearFlag(); // Get Data
   			
	//LCD_1_Position(0,0);	
	//LCD_1_PrHexByte(CMP_CR0);
    
  // lpf_data = lpf (nADCdata);    //  low pass filtering received data
 
   lpf_data = lpf(nADCdata)<<1; // 50 and 60 Hz suppression 
   lpf_data1 = lpf1(lpf_data)<<1; // 50 and 60 Hz suppression 
   lpf_data2 = lpf2(lpf_data1)<<2; // 50 and 60 Hz suppression  
  // lpf_data3 = lpf3(lpf_data2); // 50 and 60 Hz suppression 
  // pulse
  hpf_data = abs(hpf1 (af2(lpf_data2)));
   
	LCD_1_Position(1,0);            // Place LCD cursor at row 0, col 0.
	if (arr == 0x00)
	{
	LCD_1_PrCString("----- Start ----"); 
	}
	else if(arr < 0x3C)
	{
	LCD_1_PrCString("-- Bradycardia--");
	}
	else if((arr >= 0x3C) && (arr <= 0x5A))
	{
	LCD_1_PrCString("---- Normal ----");
	}
	else
	{
	LCD_1_PrCString("-- Tachycardia--");
	}
	
	//delay_sec(1);
	// rqs detection
 if ((hpf_data > (Q_threshold_low)) && (hpf_data < 1400) && (g_time > 400)) 
 {
	if (counter_1200_msec < 1) 
	{
	max_200_msec = hpf_data;  // lets the first data is maximum
	counter_1200_msec_enable = TRUE;
	
	 if (!noise_detected)	
			{
			rr_interval[rr_array_counter] = (g_time - g_time_old);
			
			g_time_old = g_time;
			
			Port1_1(1);
			}      // end if noise_detected

	rr_array_counter++;
	noise_detected = FALSE;
	
	/*
	if ((rr_array_counter > 4) && (rr_array_counter <6)) 
	{
	SPIM(arr);
	}*/
		/*if(rr_array_counter < 11)
	{
	SPIM(nADCdata);
	//SPIM(arr);
	//counter_1200_msec=48;	
	}*/
	
	//if (rr_array_counter > 10) 
		if (rr_array_counter > 4 )
		{
		t_pulse_rate = median_filter(rr_interval);
		rr_array_counter = 0;
		//arr=arr++;
		//SPIM(arr);
		

		
		if ((t_pulse_rate > 54) && (t_pulse_rate < 360))
			{ 
	
			LCD_1_Position(0,1);            // Place LCD cursor at row 0, col 5.
			LCD_1_PrCString("P:");   // Print "Pulse: " on the LCD  ;
			
			//LCD_1_Position(0,12);            // Place LCD cursor at .
			//LCD_1_PrHexByte(t_pulse_rate);
			rr = 7744 / t_pulse_rate;
			arr = af(rr);
			temp=arr;
			SPIM(temp);
			itoa(pulse_lcd, arr,10); // Converts an integer to a string.
			LCD_1_Position(0,3);
			LCD_1_PrString(pulse_lcd);  // Print pulse value (RR duration)on the LCD  ;
			LCD_1_Position(0,5);
			LCD_1_PrCString(" b/min ");
			//Port0_5(1);
			//arr=0x30;
			//SPIM(nADCdata);
				
			LCD_1_Position(1,0);            // Place LCD cursor at row 0, col 0.
			if (arr == 0x00)
			{
			LCD_1_PrCString("----- Start ----"); 
			}
			else if(arr < 0x3C)
			{
			LCD_1_PrCString("-- Bradycardia--");
			}
			else if((arr >= 0x3C) && (arr <= 0x5A))
			{
			LCD_1_PrCString("---- Normal ----");
			}
			else
			{
			LCD_1_PrCString("-- Tachycardia--");
			}
					//counter_1200_msec=48;	
			
			}
			//SPIM(temp);
			
		}  // end if ( rr_array_counter > 6)
		
	}  // end if counter_1200_msec
    else if ((counter_1200_msec < 48) && (hpf_data > max_200_msec))   //for  varying threshold adaptive algorithm
    {
	 	max_200_msec = hpf_data;
	}    // end else if ((counter_1200_msec < 24)
	else if (counter_1200_msec > 48) // Error   200 msec
	{
	LCD_1_Position(0,0);            // Place LCD cursor at row 0, col 0.
	LCD_1_PrCString(" ---- Error --- ");   // Print "Noise detected" on the LCD  ;
	LCD_1_Position(1,0);            // Place LCD cursor at row 0, col 0.
	LCD_1_PrHexByte(Q_threshold_low);
		LCD_1_Position(0,0);            // Place LCD cursor at row 0, col 0.
	LCD_1_PrHexByte(hpf_data);
	
	noise_detected = TRUE;
	Q_threshold_low = Q_threshold_low + 12;
	if (rr_array_counter > 0){
						 //rr_interval[rr_array_counter] = rr_interval[rr_array_counter-1];
						 if (rr_array_counter > 2) rr_array_counter--; 
						 }
	} // end else if ((counter_1200_msec > 40)

 //varying threshold adaptive algorithm
 Q_threshold_low = (max_200_msec >> 1) + (max_200_msec >> 2) ;
 if (Q_threshold_low > (prev_Q_threshold_low << 1)) Q_threshold_low = prev_Q_threshold_low << 1;
 prev_Q_threshold_low = Q_threshold_low;
 }   // end if (hpf_data > Q_threshold_low)


threshold_counter++; 
if (threshold_counter > 42) 
{
if (Q_threshold_low > 5000) Q_threshold_low = Q_threshold_low - 200;
else if (Q_threshold_low > 200) Q_threshold_low = Q_threshold_low - 40;
else if (Q_threshold_low > 60) Q_threshold_low = Q_threshold_low - 8;
else if (Q_threshold_low > 20) Q_threshold_low = Q_threshold_low - 3;
else if (Q_threshold_low > 6) Q_threshold_low = Q_threshold_low - 1;
threshold_counter = 0;
}	

if (counter_1200_msec_enable) counter_1200_msec++;
if (counter_1200_msec > 47)    // 240 msec
    { 
    counter_1200_msec = 0; 
    counter_1200_msec_enable = FALSE;
    }

// send the ecg signal to PC
/*
if (tx_counter == 1) 
{
TX8_1_CPutString("ecg");	  
TX8_1_PutSHexInt(nADCdata + 10000);
TX8_1_PutCRLF();
}
else if (tx_counter > 1) 
{
TX8_1_CPutString("res2");	  
TX8_1_PutSHexInt(hpf_data + 10000);
TX8_1_PutCRLF();
tx_counter = 0;
}
tx_counter++;	*/
	
	
	
	M8C_ClearWDT;
	g_time++;	
	Port1_1(0);	

} // end if(ADCINC14_1_fIsDataAvailable() 
  
}  // end while

goto start_init;		
}  // end main