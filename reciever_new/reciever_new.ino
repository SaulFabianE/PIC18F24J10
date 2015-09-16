/***********************************************************************************************************************
 * @FILE reciever_new.ino
 * @BRIEF UTA Parking App
 *
 * This program recieves data to from the sensing arduino via an nrf24l01 transmitter and sends the recieved data
 * to a computer to be uploaded to the database
 *
 *
 **********************************************************************************************************************/

#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

int message[1];

//pin numbers for the nrf24L01
#define CEPIN      9
#define CSNPIN     10
#define MOSI       11      
#define SCK        13
#define MISO       12

RF24 radio(CEPIN, CSNPIN);
const uint64_t pipe = 0xF0F0F0F0AA;
 
void setup(void){
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(1,pipe);
  radio.startListening();

}
 
void loop(void){
  if (radio.available()){
    bool done = false;  
      
    while (!done){
      done = radio.read(message, 2); 
      
      Serial.println(message[0]); //this will send the instruction via USB to the Java Program
     }
    }
}
