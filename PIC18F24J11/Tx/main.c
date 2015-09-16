/* 
 * File:   main.c
 * Author: Saul F Estrada
 *
 * Created on April 13, 2015, 2:34 PM
 */
//TX
//#include <stdio.h>
//#include <stdlib.h>

#include <p18f24j11.h>
#include <spi.h>
#include <timers.h>

#pragma config WDTEN = OFF
#pragma config XINST = OFF
#pragma config OSC = INTOSC

void initialize_pic(void);
void initialize_module(void);
unsigned char spi_Send_Read(unsigned char byte);
void delay(unsigned int c);
void reset(void);
void transmit(void);

#define CE          LATCbits.LATC1		// CE output pin, PORTC pin 1
#define CSN         LATCbits.LATC2		// CSN output pin, PORTC pin 2
#define SCK         LATCbits.LATC3		// Clock pin, PORTC pin 3 
#define SI          PORTCbits.RC4		// Serial input pin, PORTC pin 4 
#define SO          LATCbits.LATC5		// Serial output pin, PORTC pin 5 
#define IRQ         PORTBbits.RB0		// IRQ input pin, PORTB pin 0
#define SPI_SCALE	4   				// postscaling of signal 
#define LED			LATCbits.LATC0      // status LED    
#define nop() _asm nop _endasm          // got from sampleCode, ***delete after testing
#define sensorInput PORTBbits.RB4       // sensor input
/*
 * 
 */
void main (void) {
    initialize_pic();       // initialize the PIC18F24J11
    initialize_module();    // initialize the nRF24L01
    
    while(1)
    {
        if(sensorInput==1)  // for testing, will need to be moved to an interrupt
        {
            transmit();     // transmit random data for now
            LED = 1;        // led on
            delay(63973);   // not sure how many seconds, but good for now	
            LED = 0;        // led off   
            delay(40000);   // not sure how many seconds, but good for now	
            nop();          // got from sample prog online, not really needed
        }
    }
}

void delay(unsigned int c)
{
	INTCONbits.TMR0IF = 0;
	WriteTimer0(c);
	while (!INTCONbits.TMR0IF);
}

void transmit(void)
{
    CSN=0;
    
    spi_Send_Read(0x27); //0010 0111 = 07, STATUS
 	spi_Send_Read(0x7E); //0111 1110 , clear bit, reset value
	
    //Interrupt not reflected on the IRQ pin, Enable CRC, PTX, Power UP
    CSN = 1;
    CSN = 0;
    spi_Send_Read(0x20); //0010 0000 = 00, CONFIG
 	spi_Send_Read(0x3A); //0011 1010
    
    CSN = 1;
    CSN = 0;
    spi_Send_Read(0xE1); //1110 0001, FLUSH_TX
    
    spi_Send_Read(0xA0); //1010 0000, W_TX_PAYLOAD, 4 byte payload
    spi_Send_Read(0x34); //0011 0100  mess around with these values, see what
  	spi_Send_Read(0x33); //0011 0011  happens, print through one of the ports
   	spi_Send_Read(0x32); //0011 0010
  	spi_Send_Read(0x31); //0011 0001
    CSN = 1;
    
    //Pulse CE to start transmission
    CE = 1;
    delay(65000);			//delay 69 ms, check with oscope
    CE = 0;
    
}
void initialize_module(void)
{
    //Every new command must be started by a 
    //high to low transition on CSN
    //<Command word: MSBit to LSBit (one byte)>
    //<Data bytes: LSByte to MSByte, MSBit in each byte first>
    
    //W_REGISTER=001A AAAA (where LSByte is first)
    CSN=0;
    CE=0;
        
    //Interrupt not reflected on the IRQ pin, Enable CRC, PTX
    spi_Send_Read(0x20); //0010 0000 = 00, CONFIG
    spi_Send_Read(0x38); //0011 1000
    
    //Auto Retransmit disabled
    CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x24);//0010 0100= 04, SETUP_RETR
    spi_Send_Read(0x00);//0000 0000 //auto retransmit disabled
     
    CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x23); //0010 0011= SETUP_AW
	spi_Send_Read(0x03); //0000 0011= RX/TX Address field width of 5 bytes
    
    //Air Data Rate=1Mbps, RF output power in TX mode=0dBm, Setup LNA gain
    CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x26); //0010 0101= RF_SETUP
	spi_Send_Read(0x07); //0000 0111
    
    ////**for debugging
    //Sets the frequency channel nRF24L01 operates on
    CSN = 1;   
	CSN = 0;
  	spi_Send_Read(0x25);//0010 0101 = 5, RF_CH
	spi_Send_Read(0x02);//0000 0010 =  channel 2
    
    ////**for debugging
    CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x30);//0011 0000 = 10, TX_ADDR
    spi_Send_Read(0xE7);//reset value = 0xE7E7E7E7E7
    spi_Send_Read(0xE7);
    spi_Send_Read(0xE7);
    spi_Send_Read(0xE7);
    spi_Send_Read(0xE7);

    //maybe not needed???
    CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x21);//0010 0001 = 01, EN_AA Enhanced ShockBurst?
 	spi_Send_Read(0x00);//0000 0000 = 00, Disable Auto Ack on all pipes
    
    //not needed since we only power on when we transmit
    /*CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x20);//0010 0000= 0,CONFIG
    spi_Send_Read(0x3B);//0011 1011= POWER UP *look at first part of this function*/
    
    CSN = 1;    
    //CE = 1;
}




void initialize_pic(void)
{
    //if(RCONbits.POR==0)  //*** delete if not needed, or prog does not run
    //    RCONbits.POR=1; //A Power-on Reset has not occurred p64
    
    //run internal oscillator at 8 MHz (INTOSC drives clock directly) p44
    OSCCON= OSCCON | 0b01110000; 
    while (OSCCONbits.OSTS == 0); // wait for oscillator to be ready p44
	
    ANCON1bits.PCFG12= 1;    // B0 is set as a digital input p353
	TRISCbits.TRISC3 = 0;	// SDO output
	TRISCbits.TRISC5 = 0;   // SCK output
	TRISCbits.TRISC2 = 0;	// CSN output
	TRISCbits.TRISC1 = 0;	// CE output
	TRISBbits.TRISB0 = 1;	// IRQ input
	TRISBbits.TRISB4 = 1;   //RB4, sensor input
    CSN = 1;			    // CSN high
	SCK = 0;			    // SCK low
	CE	= 0;			    // CE low
	TRISCbits.TRISC0 = 0; 	// RC0 (LED) output
    
    OpenSPI(SPI_FOSC_16, MODE_00, SMPMID); //open SPI1
	OpenTimer0( TIMER_INT_OFF &
            	T0_16BIT &
            	T0_SOURCE_INT &
            	T0_PS_1_256 );
}

unsigned char spi_Send_Read(unsigned char byte)
{
	SSPBUF = byte;	
	while(!DataRdySPI())
		;	
    /*same as
     *    while(!SSPSTATbits.BF)
     
     */
	return SSPBUF; 
}	
