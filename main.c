/* 
 * File:   main.c
 * Author: Saul F Estrada
 *
 * Created on April 12, 2015, 3:10 PM
 */
//RX
#include <p18f24j11.h>
//#include <stdio.h>
//#include <stdlib.h>
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

// Defines
#define CE          LATCbits.LATC1		// CE output pin, PORTC pin 1
#define CSN         LATCbits.LATC2		// CSN output pin, PORTC pin 2
#define SCK         LATCbits.LATC3		// Clock pin, PORTC pin 3 
#define IRQ         PORTBbits.RB0		// IRQ input pin, PORTB pin 0
#define SPI_SCALE	4              		// postscaling of signal 
#define LED			LATAbits.LATA0
#define DELAY_TIME  0xF800
#define MISO        LATCbits.LATC5	// Serial output pin, PORTC pin 5 **delete
#define MOSI        PORTCbits.RC4		// Serial input pin, PORTC pin 4  **delete

/*
 * 
 */
void main(void) {
    int i;
    initialize_pic();
    initialize_module();

    while(1)
    {
   		if (IRQ == 0)    //wait for anything
        {
            for (i = 0; i < 2; i++)  //flash LED 2 times if data is received
            {
                LED = 1;
                delay(DELAY_TIME );		// should be approx 200mS, not sure how many seconds, but good for now
                LED = 0;
                delay(DELAY_TIME );		// not sure how many seconds, but good for now
            }
            delay(DELAY_TIME);			//**delete, used for debuging not sure how many seconds, but good for no
            reset();            
        }
    }
}

void reset(void)
{
    unsigned char i;
    unsigned char buffer[4];    
    
	//Read RX payload   
    CSN = 0;    
   	spi_Send_Read(0x61); //0110 0001, R_RX_PAYLOAD   
    for (i = 0; i < 4; i++) //check number of buffers/pipes;
    {        
       	buffer[i] = spi_Send_Read(0); //read operation always starts at byte 0.        
    }    
    
    CSN = 1;       
    CSN = 0;    
 	spi_Send_Read(0xE2); //1110 0010 = FLUSH_RX
    
    CSN = 1;
    CSN = 0;  
  	spi_Send_Read(0x27); //0010 0111 = W_REGISTER, 07=STATUS
	spi_Send_Read(0x40); //0100 0000 = Asserted when new data arrives RX FIFO, clear bit
                         
    CSN = 1;
}

void delay(unsigned int c)
{
	INTCONbits.TMR0IF = 0;
	WriteTimer0(c);
	while (INTCONbits.TMR0IF == 0)
		;
}

void initialize_pic(void)
{
    if(RCONbits.POR==0)  //*** delete if not needed, or prog does not run
        RCONbits.POR=1; //A Power-on Reset has not occurred p64
    
    //run internal oscillator at 8 MHz (INTOSC drives clock directly) p44
    OSCCON= OSCCON | 0b01110000; 
    while (OSCCONbits.OSTS == 0); // wait for oscillator to be ready p44
	
    ANCON1bits.PCFG12= 1;    // B0 is set as a digital input p353
	TRISCbits.TRISC3 = 0;	// SDO output
	TRISCbits.TRISC5 = 0;   // SCK output
	TRISCbits.TRISC2 = 0;	// CSN output
	TRISCbits.TRISC1 = 0;	// CE output
	TRISBbits.TRISB0 = 1;	// IRQ input
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

void initialize_module(void)
{
    //Every new command must be started by a 
    //high to low transition on CSN
    //<Command word: MSBit to LSBit (one byte)>
    //<Data bytes: LSByte to MSByte, MSBit in each byte first>
    
    //W_REGISTER=001A AAAA (where LSByte is first)
    CSN=0;
    CE=0;
    
    
    //Interrupt not reflected on the IRQ pin, Enable CRC, PRX
    spi_Send_Read(0x20); //00 = CONFIG
    spi_Send_Read(0x39); //0011 1001
    
    //Auto ACK disabled on all data pipes
    CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x21);//01 = Enhanced ShockBurst
    spi_Send_Read(0x00);//0000 0000 //auto ACK disabled on all data pipes
     
    CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x23); //0010 0011= SETUP_AW
	spi_Send_Read(0x03); //0000 0011= RX/TX Address field width of 5 bytes
    
    //Air Data Rate=1Mbps, RF output power in TX mode=0dBm, Setup LNA gain
    CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x26); //0010 0101= RF_SETUP
	spi_Send_Read(0x07); //0000 0111
    
    CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x31); //0011 0001 = 11, RX_PW_P0
	spi_Send_Read(0x04); //0000 0100= 4 bytes in RX payload in data pipe 0
      
    //**for debugging
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

    CSN = 1;   
	CSN = 0;
    spi_Send_Read(0x20);//0010 0000= 0,CONFIG
    spi_Send_Read(0x3B);//0011 1011= POWER UP *look at first part of this function
    
    CSN = 1;    
    CE = 1;
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
