// Alex Bouvy
// WVU RockSat-X 2013
// Original: Marc Gramlich 2012 RS-C
// Flight Program
// Last Updated:

#include "predef.h"
#include <stdio.h>
#include <ctype.h>
#include <basictypes.h>
#include <serialirq.h>
#include <system.h>
#include <constants.h>
#include <ucos.h>
#include <SerialUpdate.h>
#include <pins.h>
#include <..\MOD5213\system\sim5213.h>
#include <cfinter.h>
#include <utils.h>
#include <a2d.h>
#include <pins.h>
#include <math.h>
#include <smarttrap.h>
#include <string.h>
#include <bsp.h>

#define SUPPLY_OUT	0x0200	// 512
#define XGYRO_OUT	0x0400	// 1024
#define YGYRO_OUT	0x0600	// 1536
#define ZGYRO_OUT	0x0800	// 2048
#define XACCL_OUT	0x0A00	// 2560
#define YACCL_OUT	0x0C00	// 3072
#define ZACCL_OUT	0x0E00	// 3584
#define XMAG_OUT	0x1000	// 4096
#define YMAG_OUT	0x1200	// 4608
#define ZMAG_OUT	0x1400	// 5120
#define TEMP_OUT	0x1600	// 5632
#define XHIACCL_OUT	0x1200	// 4608
#define YHIACCL_OUT	0x1400	// 5120
#define ZHIACCL_OUT	0x1600	// 5632

extern "C"
{
    void          UserMain( void *pd );
    void          SetIntc( long func, int vector, int level, int prio );
}

const char * AppName="RSX_Flight_2013";

/********** Name for development tools to identify this application **********/
volatile DWORD pwm_count0,pwm_count1,pwm_count2, pit_count, radio_step;
volatile DWORD radio_count, pitr_count;

/******************************* IMU Data ************************************/
volatile short int adis_data[13]={1,2,3,4,5,6,7,8,9,10,11,12,13};
unsigned int spi_data[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile unsigned int analog_pins[8]={1,2,3,4,5,6,7,8};
unsigned int analog_bits[8]={0,0,0,0,0,0,0,0};

/******************************************************************************/
/* INTERRUPT: GPT1 ISR PWM Measurement of the CONTROL Switch                  */
/******************************************************************************/
unsigned int Geiger_Count[3]={0,0,0};
INTERRUPT(GPT0,0x2600)
{
    WORD temp=sim.gpt.c0;
	Geiger_Count[0]=Geiger_Count[0]+1;
}

INTERRUPT(GPT1,0x2700)
{
    WORD temp=sim.gpt.c1;
	Geiger_Count[1]=Geiger_Count[1]+1;
}

INTERRUPT(GPT2,0x2700)
{
    WORD temp=sim.gpt.c2;
	Geiger_Count[2]=Geiger_Count[2]+1;
}

/******************************************************************************/
/* INIT:GPT Initialization                                                   */
/******************************************************************************/
void InitGeigerCounter() {
	sim.gpt.ddr = 0; // All pins as input
	sim.gpt.ios = 0; // All pins input capture
	sim.gpt.scr1 = 0x90;// 10010000 GPT enabled, fast flag clearing
	sim.gpt.ctl1 = 0; // Inputs disconnected from outputs
	sim.gpt.ctl2 = 0x55; //FC// Capture both edges on Ch 1,2,3,4
	sim.gpt.scr2 = 0x03; // Timer divide by 8
	sim.gpt.pactl = 0; // Pulse accumulator control
	sim.gpt.ie = 0x0F; // Enable interrupt on Ch 0-3
	SetIntc((long) &GPT0, 44, 4, 1);// GPTISR for PPM OR Gate
	SetIntc((long) &GPT1, 45, 4, 2); // GPTISR for PPM Control Switch
	SetIntc((long) &GPT2, 46, 4, 3);// GPTISR for PPM OR Gate
}

/******************************************************************************/
/* INTERRUPT: Read Data from micro-IMU                                        */
/******************************************************************************/
void Read_SPI()
{
    sim.spi.qir=0x0001;
    sim.spi.qar=0x0010; //select the first receiver entry
    int i=0;
    for (i=0; i<13; i++)
    {
        spi_data[i]=(unsigned int) (sim.spi.qdr);
    }
    int j=0;
    for (j=0; j<13; j++)
    {
        adis_data[j]=(short int) ((spi_data[j]&0x3FFF) << 2)/4;
    }
}

void Init_SPI()
{
    sim.spi.qmr=0xC380;//0.258MHz Baud Rate IMU Data
    sim.spi.qar=0x0020;// select the first command RAM entry
    sim.spi.qdr=0x7E00;	// 32256
    sim.spi.qdr=0x7E00;
    sim.spi.qdr=0x7E00;
    sim.spi.qdr=0x7E00;
    sim.spi.qdr=0x7E00;
    sim.spi.qdr=0x7E00;
    sim.spi.qdr=0x7E00;
    sim.spi.qdr=0x7E00;
    sim.spi.qdr=0x7E00;
    sim.spi.qdr=0x7E00;
    sim.spi.qdr=0x7D00;	// 32000
    sim.spi.qdr=0x7D00;
    sim.spi.qdr=0x7D00;
    sim.spi.qar=0x0000; //select the first transmit ram entry IMU Data
    sim.spi.qdr=YACCL_OUT;
    sim.spi.qdr=ZACCL_OUT;
    sim.spi.qdr=XGYRO_OUT;
    sim.spi.qdr=YGYRO_OUT;
    sim.spi.qdr=ZGYRO_OUT;
    sim.spi.qdr=XMAG_OUT;
    sim.spi.qdr=YMAG_OUT;
    sim.spi.qdr=ZMAG_OUT;
    sim.spi.qdr=TEMP_OUT;
    sim.spi.qdr=XACCL_OUT;
    sim.spi.qdr=XHIACCL_OUT;
    sim.spi.qdr=YHIACCL_OUT;
    sim.spi.qdr=ZHIACCL_OUT;
    sim.spi.qwr=0x5C00;	// 23352. After getting to the 10 piece of data return to zero
    sim.spi.qdlyr=(0x03FF|0x8000);	// 1023C|32768
}
void Get_AD()
{
	 int c=0;
	    for (c=0; c<8; c++)
	    {
	        analog_bits[c]=(unsigned int)(ReadA2DResult(c) >> 3);
	    }
	 int a=0;
	    for (a=0; a<8; a++)
	    {
	        analog_pins[a]=(unsigned int)(analog_bits[a]);
	    }
}
/******************************************************************************/
/* INIT: Pin Configuration                                           */
/******************************************************************************/
void InitPinClass()
{
    Pins[2].function( PIN2_UART0_RX );  //flash
    Pins[3].function( PIN3_UART0_TX );  //flash

	//Analog Pins
    Pins[4].function( PIN4_GPIO );	//Camera Record
    Pins[6].function( PIN6_GPIO );	//indicator LED
    Pins[7].function( PIN7_GPIO );	//indicator LED
    Pins[8].function( PIN8_GPIO );	//Camera Power
    Pins[11].function( PIN11_AN2 );	//Gyro
    Pins[12].function( PIN12_AN1 );	//LP
    Pins[13].function( PIN13_AN0 );	//LP
    Pins[14].function( PIN14_AN3 );	//Radio Ant A2D
    Pins[15].function( PIN15_AN7 );	//Radio Relected A2D
    Pins[16].function( PIN16_AN6 );	//Mag X
    Pins[17].function( PIN17_AN5 );	//Mag Y
    Pins[18].function( PIN18_AN4 );	//Mag Z

    Pins[21].function( PIN21_GPIO );    //RPE GHz OpAmp SSR
    Pins[22].function( PIN22_GPIO );    //RPE MHz OpAmp SSR
    Pins[23].function( PIN23_GPIO );    //Camera Power
    Pins[24].function( PIN24_PWM0 );    //Radio Sweep
    Pins[25].function( PIN25_GPIO );    //Camera Record
    Pins[26].function( PIN26_GPT2 );    //Geiger Tube
    Pins[27].function( PIN27_GPT1 );    //Geiger Tube
    Pins[28].function( PIN28_GPT0 );    //Geiger Tube
    // InitSerialPorts()
    Pins[29].function( PIN29_UART1_RX );
    Pins[30].function( PIN30_UART1_TX );

    Pins[31].function( PIN31_GPIO );//GHz Transmitter On/Off
    Pins[32].function( PIN32_GPIO );//MHz Transmitter On/Off

    // Init SPI
    Pins[34].function( PIN34_QSPI_CS1 );
    Pins[35].function( PIN35_QSPI_CS0 );
    Pins[36].function( PIN36_QSPI_DOUT );
    Pins[37].function( PIN37_QSPI_DIN );
    Pins[38].function( PIN38_QSPI_CLK );
}
/******************************************************************************/
/*Assemble The Data Packet                                                    */
/******************************************************************************/
void Assemble_Packet()
{
    char Output_Packet[51];
    Output_Packet[0]=65;                                     // 0    Header #1
    Output_Packet[1]=66;                                     // 1    Header #2
    Output_Packet[2]=67;                                     // 2    Header #3
    for (int k=0; k<10; k++)
    {
        Output_Packet[((k*2)+3)]=(adis_data[k]&0xFF00)>>8; //IMU MSB
        Output_Packet[((k*2)+4)]=(adis_data[k]&0x00FF); ////IMU LSB
    }
    for (int i=0; i<8; i++)
    {
    	Output_Packet[((i*2)+23)]=(analog_pins[i]&0xFF00)>>8;
    	Output_Packet[((i*2)+24)]=(analog_pins[i]&0x00FF); ////IMU LSB
    }
    for (int g=0; g<3; g++)
    {
    	 Output_Packet[((g*2)+39)]=(Geiger_Count[g]&0xFF00)>>8;
    	 Output_Packet[((g*2)+40)]=(Geiger_Count[g]&0x00FF);
    }

    Output_Packet[45]= (pwm_count0&0xFF00)>>8;	// 65280
    Output_Packet[46]=(pwm_count0&0x00FF);		// 255

    Output_Packet[47]= (radio_step&0xFF00)>>8;
    Output_Packet[48]=(radio_step&0x00FF);

    Output_Packet[49]= (pitr_count&0xFF00)>>8;
    Output_Packet[50]=(pitr_count&0x00FF);

    for (int l=0; l<51; l++)
    {
        char temp=Output_Packet[l];
        writechar(0,temp);
    }
}

void Camera_Sequence()
{
	if(pitr_count<72000 || pitr_count>249900)
	{
		if(pitr_count<70800 || pitr_count>251100)
		{
			Pins[8]=0;
			Pins[23]=0;
		}
		else
		{
			Pins[8]=1;
			Pins[23]=1;
		}
	}
	else
	{
		Pins[8]=0;
		Pins[23]=0;
	}
	if(pitr_count>=73200 || pitr_count<=248700)
	{
		if(pitr_count<74400 || pitr_count>247500)
		{
			Pins[25]=1;
			Pins[4]=1;
		}
		else
		{
			Pins[25]=0;
			Pins[4]=0;
		}
	}
	else
	{
		Pins[25]=0;
		Pins[4]=0;
	}
}

/******************************************************************************/
/* INTERRUPT: PIT1 ISR 50 Hz, Main routine for the Controller!                */
/******************************************************************************/
int lp_neg=1,lp_pos=0;
INTERRUPT( my_pitr_func, 0x2600 )	// 9728
{
    WORD tmp = sim.pit[1].pcsr;                                  // Use PIT 1
    tmp &= 0xFF0F;                                           // Bits 4-7 cleared; 65295
    tmp |= 0x0F;                                                 // Bits 0-3 set; 15
    sim.pit[1].pcsr = tmp;
    sim.pwm.pwmdty[0] = pwm_count0;
    sim.pwm.pwmdty[1] = pwm_count1;
    sim.pwm.pwmdty[2] = pwm_count2;
    pwm_count0++;
	Init_SPI();
	Read_SPI();
	Get_AD();
	Camera_Sequence();
	if(pit_count<100)
	{
		Pins[6]=1;
		Pins[7]=0;
		Pins[21]=1;
		Pins[22]=0;
	}
	else if(pit_count<200)
	{
		Pins[6]=0;
		Pins[7]=1;
		Pins[21]=0;
		Pins[22]=1;
	}
	else if(pit_count<300)
	{
		Pins[6]=0;
		Pins[7]=0;
		Pins[21]=0;
		Pins[22]=0;
	}
	else
	{
		pit_count=0;
	}
	if(pit_count==300)
	{
		pit_count=0;
	}
	if(pwm_count0%100==1)
	{
		pwm_count0=1;
	}
	if(lp_neg==1 && lp_pos==0 && pwm_count1<100)
	{
		pwm_count1++;
	}
	else
	{
		pwm_count1=0;
		lp_neg=0;
		lp_pos=1;
	}
	if(lp_neg==0 && lp_pos==1 && pwm_count2<100)
	{
		pwm_count2++;
	}
	else
	{
		pwm_count2=0;
		lp_pos=0;
		lp_neg=1;
	}

	Assemble_Packet();
	pit_count++;
        pitr_count++;
}
/******************************************************************************/
/* INIT: PIT1 Initialization                                                  */
/******************************************************************************/
void InitPIT1( WORD clock_interval, BYTE pcsr_pre /*table 173 bits 811 */ )
{
    WORD tmp;  // Populate the interrupt vector in the interrupt controller
    SetIntc( ( long ) &my_pitr_func, 56, 2 /* IRQ 2 */, 3 );
    sim.pit[1].pmr = clock_interval;          // Set the PIT modulus value
    tmp = pcsr_pre;
    tmp = ( tmp << 8 ) | 0x0F;
    sim.pit[1].pcsr = tmp;
}

void Init_PWM()
{
	sim.pwm.pwme = 0; 			// Disable all PWM channels before making any settings
	sim.pwm.pwmpol &= ~0x01; 		// Set to have an initial low signal, then set high on Duty cycle output compare
	sim.pwm.pwmclk |= 0x01; 		// Set to use clock SA (Scale A)
	sim.pwm.pwmprclk |= 0x07; 		// Set to use clock A prescale value of 2 (Internal Bus Clock/ 2^1)
	sim.pwm.pwmcae &= ~0x01; 		// Set to operate channel 0 in left-aligned output mode
	sim.pwm.pwmctl = 0; 			// All channels are 8-bit channels, doze and debug mode disabled
	sim.pwm.pwmscla = 0x04; 		// Use scale divisor value of 2 to generate clock SA from clock A
	sim.pwm.pwmcnt[0] = 1; 			// Write any value to this register to reset the counter and start off clean
	sim.pwm.pwmper[0] = 100; 		// Set PWM Channel Period register to a value
	sim.pwm.pwmdty[0] = 50; 		// Set PWM Channel Duty register. 50% duty cycle
	sim.pwm.pwmcnt[1] = 1; 			// Write any value to this register to reset the counter and start off clean
	sim.pwm.pwmper[1] = 100; 		// Set PWM Channel Period register to a value
	sim.pwm.pwmdty[1] = 50; 		// Set PWM Channel Duty register. 50% duty cycle
	sim.pwm.pwmcnt[2] = 1; 			// Write any value to this register to reset the counter and start off clean
	sim.pwm.pwmper[2] = 100; 		// Set PWM Channel Period register to a value
	sim.pwm.pwmdty[2] = 50; 		// Set PWM Channel Duty register. 50% duty cycle
	sim.pwm.pwme |= 0x01; 			// Enable PWM Output for PWM Channel 1
}

void UserMain(void * pd)
{
	SimpleUart( 0, 115200 );
	SimpleUart( 1, 115200 );
	assign_stdio( 0 );
	EnableSmartTraps(); // enable smart trap utility
        OSChangePrio( MAIN_PRIO );
        EnableSerialUpdate();
        InitPinClass();  // Assign function to each Pin
        EnableAD();
	Init_SPI();
	InitGeigerCounter(); //Initialize the GPT for the Geiger Counter
	Init_PWM();
	InitPIT1(6912,4);//300Hz Operation
}



