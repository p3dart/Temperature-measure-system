/***********************************************************************************************
	File Name:	
	Author:		PLad
	Date:		27 Feb, 2020



	Description: This program will be used for fetching the temperature and humidity values
	             from the dht22 sensor. As the output of the program, the values will be
		     shown on the thingspeack channel to display the data for the temperature and humidity.
	
***********************************************************************************************/
/*Preprocessor ===================================================================*/

#include "pragmas.h"
#include <adc.h>
#include <stdlib.h>
#include <delays.h>
#include <stdio.h>
#include "usart.h"
#include <p18f45k22.h>



// Constants ---------------------------------------------------------------------------------

#define TRUE		1			
#define	FALSE		0	
#define DHTPIN 		PORTBbits.RB0
#define DHTDIR 		TRISBbits.TRISB0

#define SIZE      	11

#define OUTPUT		0
#define INPUT		1
#define MAXDATA		32
#define MAXCS		8
#define LIMIT		26
#define MAXSIZE     50
#define SSID        TempSense
#define PASS        12345678
#define API         IDU0J5BBIAEVUG2V
#define T0FLAG          INTCONbits.TMR0IF

// Global Variables --------------------------------------------------------------------------


int Temp,humd;
int i;
char str[MAXSIZE];
int index = 0, temp = 0, humd = 0;
int Time=0;


// Functions ---------------------------------------------------------------------------------
/*** set_osc_p18f45k22_4MHz: ******************************************************
Author:     	PLad   
Date:		22 Jan, 2020		
Modified:	
Desc:		Sets the internal Oscillator of the Pic 18F45K22 to 4MHz.
Input: 		None
Returns:	None
**********************************************************************************/
void set_osc_p18f45k22_4MHz(void)
{
	OSCCON  =  	0x52; 	
	OSCCON2 = 	0x04;	 
	OSCTUNE = 	0x80;	
	
	while (OSCCONbits.HFIOFS != 1); 	
}
//eo: set_osc_p18f45k22_4MHz:: ***************************************************



/*** portConfig: **************************************************************
Author:		PLad
Date:		21 Feb 2020
Modified:	
Desc:		Initializes ports according to project needs.
Input: 		None
Returns:	None
**********************************************************************************/
void portConfig(void)
{
	//PORT A

		ANSELA = 0x03;     
		LATA   = 0x00;     
		TRISA  = 0xFF;     

	//PORT B

		ANSELB = 0x00;     
		LATB   = 0x01;      
		TRISB  = 0x00;     

	//PORT C

		ANSELC = 0x00;     
		LATC   = 0x00;      
		TRISC  = 0x00;     

	//PORT D

		ANSELD = 0x00;     
		LATD   = 0x00;     
		TRISD  = 0xF0;    // set the out put / input pin 33

}
//eo: portConfig:: *************************************************************

/*** serialConfig: **************************************************************
Author:		pLad
Date:	              22 Feb, 2020
Modified:	
Desc:		Serial port configuration
Input: 		None
Returns:	None
**********************************************************************************/
void serialConfig(void)
{
	SPBRG1= 8;                // set the baud rate value at 115200
	TXSTA1= 0x26;              // set the tx
	RCSTA1= 0X90;              // set the rx
	BAUDCON1=0X40;             // select the baud config
}
//eo: serialConfig::*************************************************************


/*** TMR0Config **************************************************************
Author:		plad
Date:	    22/01/2020
Modified:	
Desc:		configure the timer 0
Input: 		None
Returns:	None
**********************************************************************************/
void TMR0Config(void)
{
	T0CON=0x91;           // set the timer for the one second
	TMR0H=0x0B;           // set the timer0 high bits
	TMR0L=0xDC;           // set the timer0 low bits
	T0FLAG=FALSE;
}
//eo: TMR0Config: *************************************************************



/*** TMR0Reset **************************************************************
Author:		plad
Date:	    22/01/2020
Modified:	
Desc:		reset the timer 0 
Input: 		None
Returns:	None
**********************************************************************************/
void TMR0Reset(void)
{
	T0FLAG=FALSE;       //making the timer0 flag 0
	TMR0H=0x0b;         // set the timer0 high bits
	TMR0L=0xdc;         // set the timer0 low bits	
}
//eo: TMR0reset: *************************************************************





/*** startSignal****************************************************************
Author:		PLad
Date:		24 Feb, 2020		
Modified:	CTalbot
Desc:		
Input: 		None
Returns:	None
**********************************************************************************/  
void startSignal(void) 
{	
	DHTDIR = OUTPUT;	// Output
	DHTPIN = FALSE;		// Go low
	Delay1KTCYx( 1 );	// Delay 20ms
	DHTPIN = TRUE;
	Delay10TCYx( 4 );	// Delay 40us
	DHTPIN = FALSE;
	DHTDIR = INPUT;

	// End of start sequence
	// DHT22 should take over at this point and pull the line
	// Ack signal will delay 80 + 80 us

	while( !DHTPIN );
//	while( DHTPIN );
	
	// Get data - checkResponse
}
//eo: startSignal:: ***************************************************************
 


/*** checkResponse ****************************************************************
Author:		CTalbot
Date:		24 Feb, 2020		
Modified:	CTalbot
Desc:		
Input: 		None
Returns:	None
**********************************************************************************/
void checkResponse()
{
	long data = 0;
	char checksum = 0;
	for( index=0; index< MAXDATA; index++)
	{
		while( !DHTPIN );
		TMR0H=0;
		TMR0L=0;
		while( DHTPIN );
		
		if( TMR0L > LIMIT )
		{
			data = (data<<1) + 1;
		}
		else if( TMR0L < LIMIT )
		{
			data = (data<<1);
		}
	}
	for( index=0; index< MAXCS; index++)
	{
		while( DHTPIN );
		TMR0H=0;	
		TMR0L=0;
		while( !DHTPIN );
		
		if( TMR0L > LIMIT )
		{
			checksum = (checksum<<1)+ 1;
		}
		else if( TMR0L < LIMIT )
		{
			checksum = (checksum<<1);
		}
	}
	temp = data;
	humd = (data>>16) ;

	Nop();
	DHTDIR = OUTPUT;
	DHTPIN = TRUE;

	printf("H: %i.%i\n\rT: %i.%i\n\r", humd/10, humd%10, temp/10, temp%10 ); // collect the data of quantity and send it to the serial output
	Nop();
}
//eo: checkResponse:: ***************************************************************




/*** readData ********************************************************************
Author:		PLad
Date:		24 Feb, 2020		
Modified:	
Desc:		
Input: 		None
Returns:	None
**********************************************************************************/
char readData(char *ptr)
{
	char i;
	for(i=0; i<=7; i++)
	{
		TMR0H=0;
		TMR0L=0;
		while(!DHTPIN)
		{
			if(TMR0L>100)
			{
				return 1;
			}
		}
		TMR0H=0;
		TMR0L=0;
		while(DHTPIN)
		{
			if(TMR0L>100)
			{
				return 1;
			}
			else if(TMR0L>50)
			{
				*ptr |= (1<<(7-i));
			}
		}
		
	}
}
//eo: readData:: ***************************************************************


/*** display ********************************************************************
Author:		PLad
Date:		24 Feb, 2020		
Modified:	
Desc:		
Input: 		None
Returns:	None
**********************************************************************************/
char display()
{
	startSignal();
}
//eo: display:: ***************************************************************

/*** clrString ********************************************************************
Author:		PLad
Date:		24 Feb, 2020		
Modified:	
Desc:		
Input: 		None
Returns:	None
**********************************************************************************/
void clrString()
{
	char index=0;
	for(index=0;index<MAXSIZE;index++)
	{
		str[index]=0;
	}
}
//eo: clrString:: ***************************************************************

/*** initialESP ********************************************************************
Author:		PLad
Date:		24 Feb, 2020		
Modified:	
Desc:		
Input: 		initialize the esp module and configure with the local wifi
Returns:	None
**********************************************************************************/
void initialESP()
{
	clrString();
	sprintf(str,"AT");              // send first at command           /
	puts1USART(str);
	Delay1KTCYx( 100 );
	clrString();
	sprintf(str,"ATE0");              // set at command to enable the echo command
	puts1USART(str);
	Delay1KTCYx( 100 );
	clrString();
	sprintf(str,"AT+CWMODE=3");       // at command to set the wifi module on both station and host mode
	puts1USART(str);
	Delay1KTCYx( 100 );
	clrString();
	sprintf(str,"AT+CIPMUX=1");       // to set the esp with the multiple devices
	puts1USART(str);
	Delay1KTCYx( 100 );
	clrString();
	sprintf(str,"AT+CWJAP=");           // to set/connect the esp with the local network.
	sprintf(str,"TempSense,");           // name of the wifi
	sprintf(str,"12345678");             // password to connect the wifi
	puts1USART(str);
	Delay1KTCYx( 100 );
}
//eo: clrString:: ***************************************************************






/*** initializeSystem: ***********************************************************
Author:		Plad
Date:		26 Feb, 2020
Modified:	None
Desc:		All the required function for initiation are called within this function.
Input: 		None
Returns:	None
**********************************************************************************/
void initializeSystem(void)
{									
	set_osc_p18f45k22_4MHz();           							
	portConfig();                        						    
	serialConfig();        
	TMR0Config();
	TMR0Reset();
	T0CON = 0x90;             
}	
// eo initializeSystem::**********************************************************




/*--- MAIN FUNCTION -------------------------------------------------------------------------
-------------------------------------------------------------------------------------------*/

void main(void)
{
	initializeSystem();
	initialESP();
	while(1)
	{
		if(Time>= 120)
		{
			Time=0;
		  	startSignal();
			checkResponse();
			clrString();
			sprintf(str,"AT+CIPSTART=4,");     // to set the tcp and api link for the thingspeak website.
			sprintf(str,"TCP,");               //  to set the tcp mode
			sprintf(str,"api.thingspeak.com");   // send the api of thingspeak
			printf("%s",str);
			puts1USART(str);
			Delay1KTCYx( 100 );
			clrString();
			sprintf(str,"AT+CIPSEND=80");        // send at command that we can send 80 chacters
			printf("%s",str);
			puts1USART(str);
			Delay1KTCYx( 100 );
			sprintf(str,"GET /update?api_key= IDU0J5BBIAEVUG2V&field1=%i.%i",temp/10, temp%10); // send the field data of temperature.
			printf("%s",str);
			puts1USART(str);
			sprintf(str,"AT+CIPCLOSE");
			printf("%s",str);
			puts1USART(str);
			Delay1KTCYx( 100 );
			clrString();
			sprintf(str,"GET /update?api_key= IDU0J5BBIAEVUG2V&field2=%i.%i",humd/10, humd%10);   // send the field data of humidity.
			printf("%s",str);
			puts1USART(str);
			sprintf(str,"AT+CIPCLOSE");
			printf("%s",str);
			puts1USART(str);
			Delay1KTCYx( 100 );
			Time++;
			TMR0Reset();
		}	
	}//eo while	
}// eo main
