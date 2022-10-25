//----------------------------------------------------------------------------
//
// Main.c - FirstTouch RF coin-cell temperature sensor
//
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
//
// Copyright 2008, Cypress Semiconductor Corporation.
//
// This software is owned by Cypress Semiconductor Corporation (Cypress)
// and is protected by and subject to worldwide patent protection (United
// States and foreign), United States copyright laws and international
// treaty provisions. Cypress hereby grants to licensee a personal,
// non-exclusive, non-transferable license to copy, use, modify, create
// derivative works of, and compile the Cypress Source Code and derivative
// works for the sole purpose of creating custom software in support of
// licensee product to be used only in conjunction with a Cypress integrated
// circuit as specified in the applicable agreement. Any reproduction,
// modification, translation, compilation, or representation of this
// software except as specified above is prohibited without the express
// written permission of Cypress.
//
// Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,
// WITH REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
// Cypress reserves the right to make changes without further notice to the
// materials described herein. Cypress does not assume any liability arising
// out of the application or use of any product or circuit described herein.
// Cypress does not authorize its products for use as critical components in
// life-support systems where a malfunction or failure may reasonably be
// expected to result in significant injury to the user. The inclusion of
// Cypress' product in a life-support systems application implies that the
// manufacturer assumes all risk of such use and in doing so indemnifies
// Cypress against all charges.
//
// Use may be limited by and subject to the applicable Cypress software
// license agreement.
//
//--------------------------------------------------------------------------


#include "main.h"		 /* part specific constants and macros */
#include "PSoCAPI.h"    /* PSoC API definitions for all User Modules */
//#define SS 0x10;
//-------------------------------------------------------------------------
//
// Global Variables
//
//-------------------------------------------------------------------------

BYTE HBeat;
//uint HBeat;
BYTE devid=0x01;
BYTE loadValue;
WORD bCount;
bound = FALSE; //Reset the bound to the hub flag
data_transferred = FALSE; //Reset the data_transferred to the hub flag
static char txPktSz=2;
static char pktType = CYFISNP_1_API_TYPE_CONF_BCDR;
char payload_data[16];
BYTE devId = 0;
char length = 0;
//char packet_type = 0;
WORD lvDelay;
WORD reportDly;
BOOL reportDlySet;

BYTE cRXData;

static const WORD reportTimes[] = {1, 5, 30, 60, 300};  // in Seconds
static char       reportTimesIdx = 1;                   // Start at 5 sec
#define REPORT_TIMES_MAX   (sizeof(reportTimes)/2)      // # entries



//-------------------------------------------------------------------------
//
// Functions
//
//------------------------------------------------------------------------


static void ServeSNPPackets(void);
static void blinkRedLed       (WORD delayCount);
static void blinkFastGreenLed   (WORD delayCount);
//static void ReadNewTemperature  (void);
static void ReadNewHbeat  		(void);
static void Calibrate_ILO       (void);
static void ShowRxPkts          (void);
static void TimerDelay1ms       (void);
static BOOL CheckBindButton     (void);
static BOOL wakeButtonIsOn		(void);
static BOOL bindButtonIsOn		(void);
static BOOL CheckWakeButton     (void);
static void ReadWakeButton      (void);
static void showLeds			(void);
//static int CalculateThermData   (int Vexc, long Vtemp);
static void ReadBatteryLevel    (void);
static void SPISlave(void);


// ---------------------------------------------------------------------------
//
// main() - Powerup entry
//
// ---------------------------------------------------------------------------
void main(void)
{
	
   //	static long reportTimeSec;
	CYFISNP_1_PROT_STATE eProtStateOld;
	//HBeat=PIN_2_0_Read();
	SPIS_Start(SPIS_SPIS_MODE_0);
	//SPIS_EnableInt();
	
	LED_GREEN_Start();
    LED_RED_Start();
	
	
	 LED_GREEN_Off();
	 LED_RED_Off();
	
    //LED_RED_OFF;
	//LED_RED_On();
    //LED_GRN_ON;
	//LED_GREEN_On();
	
	// Write a one to the PSoC register so the switches can be read
    SW1_Data_ADDR |= SW1_MASK;
    SW2_Data_ADDR |= SW2_MASK;



    M8C_EnableGInt;
	INT_MSK0 |= 0x20; // enable GPIO interrupt

#if (defined DEBUG) || (defined CYFISNP_1_DEBUG)
    TX8_Start(TX8_PARITY_NONE);
#endif

    CYFISNP_1_Start();
    // --------------------------------------------------------
    // Disable SNP_Radio Power Management Unit (saves about 32uA)
    // --------------------------------------------------------
    CYFISNP_1_Write(CYFISNP_1_PWR_CTRL_ADR,
                  (CYFISNP_1_Read(CYFISNP_1_PWR_CTRL_ADR) & ~CYFISNP_1_PMU_EN) | CYFISNP_1_PMU_MODE_FORCE);

    //LED_GRN_OFF;
	 //LED_GREEN_Off();
	 // LED_RED_Off();

    CYFISNP_1_TimeSet(&oneSecTimer, sleepTicksPerSec);

    // -----------------------------------------------------------------------
    // POLLING LOOP
    // -----------------------------------------------------------------------
    for (;;)
    {
        // -------------------------------------------------------------------
        // Watch Start Binding Button activity
        // -------------------------------------------------------------------
        if (CheckBindButton())
        {
            CYFISNP_1_BindStart(ON_THE_FLY_DEV_ID);
            //LED_GRN_ON;
			 //LED_GREEN_On();
        }

        // -------------------------------------------------------------------
        // Watch "Force New Report" button
        // -------------------------------------------------------------------
        if (CheckWakeButton())
        {
           // ReadNewTemperature();       // Manually get new reading
		   	  ReadNewHbeat();
        }
		
		
		// -------------------------------------------------------------------
		// Updating the LEDs on the node
		// -------------------------------------------------------------------
		showLeds();
		
		
        // -------------------------------------------------------------------
        // Run SNP less frequently to save energy
        // -------------------------------------------------------------------
        if (--snpRunScaler == 0)
        {
            snpRunScaler = SNP_RUN_SCALER;
            CYFISNP_1_Run();          // Poll SNP machine
        }

        // -------------------------------------------------------------------
        // Process received SNP data packets
        //  (only supports update report rate for now)
        // -------------------------------------------------------------------
        if (CYFISNP_1_RxDataPend() == TRUE)
        {
            /*CYFISNP_1_API_PKT *pRxApiPkt;
            pRxApiPkt = CYFISNP_1_RxDataGet();
            reportTimeSec = pRxApiPkt->payload[0];    // Update report rate
            CYFISNP_1_RxDataRelease();*/
			//Get the Received Packet
			ServeSNPPackets(); // Service pending Rx data
			CYFISNP_1_RxDataRelease();// Free the Rx buffer for reuse

        }

        // -------------------------------------------------------------------
        // Periodic 1 sec events
        // -------------------------------------------------------------------
        if (CYFISNP_1_TimeExpired(&oneSecTimer) == TRUE)
        {
            CYFISNP_1_TimeSet(&oneSecTimer, sleepTicksPerSec);

            // ---------------------------------------------------------------
            // Blink Heartbeat LED
            // ---------------------------------------------------------------
            if (--ledHeartbeatSec == 0)
            {
                ledHeartbeatSec = LED_HEARTBEAT_SEC;
                blinkRedLed(5000);           // *100 uS ON
                Calibrate_ILO();        // Calibrate ILO against IMO
            }

            // ---------------------------------------------------------------
            // Periodic temperature report
            // ---------------------------------------------------------------
            if (--reportTimerSec == 0)
            {
                //ReadNewTemperature();
   				  ReadNewHbeat();
				 // ReadWakeButton();
			}
        }

        // -------------------------------------------------------------------
        // Sleep PSoC until next Sleep Timer interrupt to conserve energy
        // -------------------------------------------------------------------
/*#if LOW_POWER_TEST
        TP_0_3_Data_ADDR &= ~TP_0_3_MASK;       // TestPin LOW in SLEEP
        M8C_Sleep;
        TP_0_3_Data_ADDR |= TP_0_3_MASK;        // TestPin HIGH in WAKE
#endif
*/
        // -------------------------------------------------------------------
        // Turn OFF Green LED when not in Bind Mode
        // -------------------------------------------------------------------
        if (CYFISNP_1_eProtState != CYFISNP_1_BIND_MODE)
        {
           // LED_GRN_OFF;
		   LED_GREEN_Off();
        }
        //LED_RED_OFF;        // RED LED always goes OFF
		//LED_RED_Off();
    }
}


// -------------------------------------------------------------------------
//
// ServeSNPPackets()
//
// -------------------------------------------------------------------------
static void ServeSNPPackets(void) {

BYTE index = 0;
pRxApiPkt = CYFISNP_1_RxDataGet(); // ptr to Rx API Packet

//if the packet is of the specified type
if (pRxApiPkt->type == CYFISNP_1_API_TYPE_CONF_BCDR)
devId = pRxApiPkt->devId; //store the device ID
length = pRxApiPkt->length; //store the packet length

//get the data from the payload of the received packet and store it into an array
for (index = 0; index<=length; index++) {
payload_data[index] = pRxApiPkt->payload[index];
}

}



static void blinkRedLed(WORD delayCount)
{
    //LED_RED_ON;
	 LED_RED_On();
    while (delayCount != 0)
    {
        CYFISNP_1_Delay100uS();       // Get good short delays
        --delayCount;
    }
    //LED_RED_OFF;
	LED_RED_Off();
}


static void blinkFastGreenLed(WORD delayCount)
{
    //LED_RED_ON;
	 LED_GREEN_On();
    while (delayCount != 0)
    {
        CYFISNP_1_Delay100uS();       // Get good short delays
		CYFISNP_1_Delay100uS();       // Get good short delays
		CYFISNP_1_Delay100uS();       // Get good short delays
        --delayCount;
    }
    //LED_RED_OFF;
	LED_GREEN_Off();
}



// ---------------------------------------------------------------------------
//
// showLeds(void) - Update the Leds
//
// ---------------------------------------------------------------------------
static void showLeds(void)
{
//if the node is bound to the hub
if(bound == TRUE)
{
//LED_GREEN_On();
//blinkFastGreenLed(3000);
//blinkFastGreenLed(3000);
bound = FALSE;
}
//if data has been transmitted to the hub
if(data_transferred == TRUE)
{
blinkFastGreenLed(3000);
data_transferred = FALSE;
//blinkFastRedLed(2000);

}
/*else LED_GREEN_Off();
}
else LED_GREEN_Off();*/
}



// ---------------------------------------------------------------------------
//
// loadTxData(void) - Load the Temperature data bytes into Tx packet payload
//
// ---------------------------------------------------------------------------
static void loadTxData(void)//long Temp)
{
	SPISlave();
//static long loadValue;
//static long HeartBeat;
//HeartBeat=Temp;
#ifdef DEBUG
    CYFISNP_1_OutStr("\n\r      HBeat :");
    TX8_PutSHexInt(HBeat);
#endif


	
   // Load MSByte as payload '1'
    asm("mov A, [_devid]");
    asm("mov [_loadValue], A");
    //loadValue=HBeat;
	txApiPkt.payload[1] = loadValue;
	
   // Load LSByte as payload '0'
    asm("mov A, [_HBeat]");
    asm("mov [_loadValue], A");
    //loadValue=HBeat;
	txApiPkt.payload[0] = loadValue;

    //Load LSByte as payload '0'
   	//asm("mov A, [_HeartBeat]");
    //asm("mov [_loadValue], A");
	//loadValue=HeartBeat;
   // txApiPkt.payload[0] = loadValue;

#ifdef DEBUG
    CYFISNP_1_OutStr(" = 0x");
    CYFISNP_1_OutHex(txApiPkt.payload[0]);
    CYFISNP_1_OutStr(" ");
   	CYFISNP_1_OutHex(txApiPkt.payload[1]);
   	CYFISNP_1_OutStr("\n\r");
#endif
}


// ---------------------------------------------------------------------------
//
// Calibrate_ILO() - Calibrate Int Low Freq Osc against the Int Main Osc
//
// ---------------------------------------------------------------------------
static void Calibrate_ILO(void)
{
    //WORD bCount;

    // -----------------------------------------------------------------------
    // Get number of ILO ticks in 1 mS (as measured by IMO)
    // -----------------------------------------------------------------------
    Timer8_WritePeriod(255);
    M8C_DisableGInt;
    Timer8_Start();
    for (bCount=0; bCount != 10; ++bCount)
    {
        CYFISNP_1_Delay100uS();
    }
    bCount = Timer8_bReadTimer();
    bCount = 255 - bCount;
    Timer8_Stop();
    M8C_EnableGInt;

    sleepTicksPerSec = bCount<<1;   // Start by assuming ILO(wake) = ILO(sleep)

    // -----------------------------------------------------------------------
    // Without ILO in HighBias mode (See ILO_TR register in TRM), the ILO
    //  operates FASTER awake than asleep.
    // Therefore approximate ILO(sleep) as 25% lower than ILO(wake).
    // -----------------------------------------------------------------------
    sleepTicksPerSec -= bCount>>1;  // Decrease by 25% to estimate ILO(sleep)
}



// ---------------------------------------------------------------------------
//
// ReadWakeButton() - Send a reading to the HUB
//
// ---------------------------------------------------------------------------
static void ReadWakeButton(void)
{
    reportTimerSec = reportTimeSec;

    // -----------------------------------------------------------------------
    // If in CYFISNP_1_DATA_MODE and prior Tx data is done,
    //  then take a new reading and send it.
    // -----------------------------------------------------------------------
    if (CYFISNP_1_eProtState == CYFISNP_1_DATA_MODE && CYFISNP_1_TxDataPend() == FALSE)
    {
        // Load tx buffer payload bytes with reading to be transmitted
        txApiPkt.length = 1;
        txApiPkt.type   = CYFISNP_1_API_TYPE_CONF_BCDR;
        txApiPkt.payload[0] = wakeButtonIsOn() != 0;

        CYFISNP_1_TxDataPut(&txApiPkt); //Load the Tx data
       // LED_GREEN_On();
   	}
	
    else
    {
        CYFISNP_1_Jog();
    }
}




static void	ReadNewHbeat(void)
{
   // static char txPktSz;
    //static char pktType = CYFISNP_1_API_TYPE_CONF_BCDR;
	//static long HBeat;
    //int iVexc;
    //static long lVtemp;
	//LED_GREEN_Off();
    reportTimerSec = reportTimeSec;

    //txPktSz = 2;    // 1 byte size for the temperature packet

    // -----------------------------------------------------------------------
    // If in CYFISNP_1_DATA_MODE and prior Tx data is done,
    //  then take a new temperature reading and send it.
    // -----------------------------------------------------------------------
    if (CYFISNP_1_eProtState == CYFISNP_1_DATA_MODE && CYFISNP_1_TxDataPend() == FALSE)
    {
        if (txPktSz > CYFISNP_1_FCD_PAYLOAD_MAX)
            txPktSz = 0;

        //Vtherm_div_ON; // Turn power ON to the thermistor

       // Analog_ON;     // Power on the Analog SC and CT blocks - Med Power

       // PGA_Start(PGA_MEDPOWER); // Enable PGA block - the input to ADC
                                 //  block is routed through PGA (Gain =1)

        //ReadVTEMPInput;        // Enable the Analog Column Input Mux to
                               // read Port0.0/VTemp input

       // ADCINC_Start(ADCINC_MEDPOWER);      // Start ADC in Med Power
       // ADCINC_GetSamples(1);
      //  while (ADCINC_fIsDataAvailable() == 0);
      //  lVtemp = ADCINC_iClearFlagGetData();    // Store value read at Vtemp

        //ReadVTEMP_EXCInput;    // Enable the Analog Column Input Mux to
                               // read Port0.1/VTemp_exc input

       // ADCINC_GetSamples(1);
       // while (ADCINC_fIsDataAvailable() == 0);
       // iVexc = ADCINC_iClearFlagGetData();     // Store value read at Vexc

      //  Vtherm_div_OFF;     // Disable power supply to thermistor

#ifdef BATTERY_MONITOR
        ReadBatteryLevel();
#endif

        //PGA_Stop();         // Disable power to the PGA Block

        // Disable ADC and stop the ADC PWM
       // ADCINC_StopADC();
       // ADCINC_Stop();

       // Analog_OFF; // Disable power to the PSoC SC and CT blocks

        // Calculate Temperature
      //  iTemp = CalculateThermData(iVexc, lVtemp);
	    // HBeat=0x80;
		 
		//HBeat=cRXData;
        // Load tx buffer payload bytes with temperature to be transmitted
        loadTxData();

        txApiPkt.length = txPktSz;    // Specify Tx length
        txApiPkt.type   = pktType;    //

        CYFISNP_1_TxDataPut(&txApiPkt); //Load the Tx data
		data_transferred = TRUE;
        //LED_GRN_ON;
		// LED_GREEN_On();
		
    }
    else
    {
        CYFISNP_1_Jog();
    }
}



// ---------------------------------------------------------------------------
//
// bindButtonIsOn()
//
// ---------------------------------------------------------------------------
/*static BOOL bindButtonIsOn(void) {
#if SWITCHES_ACTIVE_HIGH
SW1_Data_ADDR &= ~SW1_MASK; // Ensure GPIO pulldown active
return((SW1_Data_ADDR & SW1_MASK) != 0);
#else
SW1_Data_ADDR |= SW1_MASK; // Ensure GPIO pullup active
return(((SW1_Data_ADDR & SW1_MASK) == 0));
#endif
}*/
static BOOL bindButtonIsOn(void)
{
   	//SET_SW1_FOR_INPUT;
	SW1_Data_ADDR &= ~SW1_MASK;                  // Ensure GPIO pulldown active
    return(SW1_Data_ADDR & SW1_MASK);
}

// ---------------------------------------------------------------------------
//
// CheckBindButton()
//
// ---------------------------------------------------------------------------
static BOOL CheckBindButton(void)
{
    WORD lvDelay;
	//SET_SW1_FOR_INPUT;
    if (bindButtonIsOn())
    {
        CYFISNP_1_TimeSet(&lvDelay, DEBOUNCE_TIME);       // Debouncing, delay
        while (CYFISNP_1_TimeExpired(&lvDelay) == 0) ;    // WAIT
        if (bindButtonIsOn())           // If button still ON
        {
            while (bindButtonIsOn())    // Wait for button release
            {
                M8C_ClearWDTAndSleep;
#if LOW_POWER_TEST
                M8C_Sleep;
#endif
            }
			bound = TRUE;
            return(TRUE);
        }
    }
    return(FALSE);
}


// ---------------------------------------------------------------------------
//
// buttonIncReportTime() - Manually step report interval
//
// ---------------------------------------------------------------------------

static void buttonIncReportTime(void)
{
    if (++reportTimesIdx >= REPORT_TIMES_MAX)
    {
        reportTimesIdx = 0;
    }
    reportTimeSec = reportTimes[reportTimesIdx];
    reportTimerSec = reportTimeSec;
}

// ---------------------------------------------------------------------------
//
//  showReportDelay() - Pulse LED to show INDEX of new report delay
//
// ---------------------------------------------------------------------------
static void showReportDelay(void)
{
#define HI_LO_PULSE   10    // Hi or Lo pulse width (in Sleep intervals)
    char pulseCt = reportTimesIdx + 1;  // 1st entry [0] gives 1 pulse
    char wait;

    //LED_RED_OFF;
	LED_RED_Off();
    for (wait=HI_LO_PULSE; wait != 0; --wait)
    {
#if LOW_POWER_TEST
        M8C_Sleep;
#endif
    }

    while (pulseCt-- != 0)
    {
        //LED_RED_ON;
		LED_RED_On();
        for (wait=HI_LO_PULSE; wait != 0; --wait)
        {
#if LOW_POWER_TEST
            M8C_Sleep;
#endif
        }
        //LED_RED_OFF;
		LED_RED_Off();
        for (wait=HI_LO_PULSE; wait != 0; --wait)
        {
#if LOW_POWER_TEST
            M8C_Sleep;
#endif
        }
    }
    ledHeartbeatSec = LED_HEARTBEAT_SEC;    // Suppress next heartbeat pulse
}


// ---------------------------------------------------------------------------
//
// wakeButtonIsOn()
//
// ---------------------------------------------------------------------------
static BOOL wakeButtonIsOn(void)
{
    SW2_Data_ADDR &= ~SW2_MASK;              // Ensure GPIO pulldown active
    return((SW2_Data_ADDR & SW2_MASK));
}




// ---------------------------------------------------------------------------
//
// CheckWakeButton() - Also cyclic steps report rate if is button held.
//
// ---------------------------------------------------------------------------

#define BUTTON_HOLD_TIME_TO_INC_REPORT_DELAY   (1500/CYFISNP_1_TIMER_UNITS)

static BOOL CheckWakeButton(void)
{
    //WORD lvDelay;
    //WORD reportDly;

    //BOOL reportDlySet;
	//SET_SW2_FOR_INPUT;

    if (wakeButtonIsOn())
    {
        CYFISNP_1_TimeSet(&lvDelay, DEBOUNCE_TIME);       // Debouncing, delay
        while (CYFISNP_1_TimeExpired(&lvDelay) == 0) ;    // WAIT
        if (wakeButtonIsOn())           // If button still ON
        {
            CYFISNP_1_TimeSet(&reportDly, BUTTON_HOLD_TIME_TO_INC_REPORT_DELAY);
            reportDlySet = TRUE;
            while (wakeButtonIsOn())    // Wait for button release
            {
                if (CYFISNP_1_TimeExpired(&reportDly) == TRUE
                    && reportDlySet == TRUE)
                {
                    reportDlySet = FALSE;
                    buttonIncReportTime();
                   // LED_RED_ON;             // Keep LED ON while switch held
				    LED_RED_On();
                }
                M8C_ClearWDTAndSleep;
#if LOW_POWER_TEST
                M8C_Sleep;
#endif

            }
            if (reportDlySet == FALSE)
            {
                showReportDelay();
            }
            return(TRUE);
        }
    }
    return(FALSE);
}

static void SPISlave(void)
{
    /* start SPIS user module and enable its interrupts	*/
	//SPIS_Start(0x00);
	//SPIS_Start(SPIS_SPIS_MODE_0);
		SPIS_EnableInt();

	/* initialize TX register with 0	
	//SPIS_SetupTxData(0x00);
	
	// continously take samples and send them via SPI to the master	
	while(1)
	{
		if(ADCINC_fIsDataAvailable())
		SPIS_SetupTxData(ADCINC_cClearFlagGetData());	
	}*/
	
	//PRT2DR &= ~SS;
	while(1)
	{
		/* read status */
		char cStatus = SPIS_bReadStatus();
		
		if(cStatus & SPIS_SPIS_SPI_COMPLETE)
		{
			/* store received data */
			cRXData = SPIS_bReadRxData();
			HBeat=cRXData;
			
			/* print status to LCD screen 
			LCD_Position(0,8);
			LCD_PrHexByte(cStatus);
			LCD_Position(1,8);
			LCD_PrHexByte(cRXData);*/
			
			/* update PWM output */
			//PWM8_WritePulseWidth(cRXData); 
		}
}
}

#ifdef BATTERY_MONITOR
// ---------------------------------------------------------------------------
//
// Read Battery Voltage()
//
// ---------------------------------------------------------------------------
static void ReadBatteryLevel(void)
{
    //ARF_CR = 0x36;
    AMX_IN = 0xe3;
    ABF_CR0 = 0x80;

    ADCINC_GetSamples(1);
    while (ADCINC_fIsDataAvailable() == 0);
    BattLvl = ADCINC_wClearFlagGetData();//Store value read in BattLvl

    AMX_IN = 0xe0;

    CYFISNP_1_OutStr("\n\r Battery Level");
    CYFISNP_1_OutStr("\n\r");
    CYFISNP_1_OutStr("\n\r");
    CYFISNP_1_OutStr("\n\r");
    TX8_PutSHexInt(BattLvl);
}
#endif  // BATTERY_MONITOR
