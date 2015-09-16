/***********************************************************************************************************************
 * @FILE sender_new.ino
 * @BRIEF UTA Parking App
 *
 * This program recieves data from the sensors and the transmit data via an nrf24l01 transmitter.
 * Assuming 4 sensors are placed as followed at the entrance of a parking lot. Sensors must be placed
 * close enough to where both sensors on one side can sense a vehicle
 * sensor1| |sensor3
 * sensor2| |sensor4
 * 
 *
 **********************************************************************************************************************/

#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>


int message[1];
const uint64_t pipe = 0xF0F0F0F0AA;

#define Sensor1    7
#define Sensor2    2
#define Sensor3    3
#define Sensor4    4

//pin numbers for the nrf24L01
#define CEPIN      9
#define CSNPIN     10
#define MOSI       11      
#define SCK        13
#define MISO       12

RF24 radio(CEPIN, CSNPIN);

void setup(void) {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(pipe);
  pinMode(Sensor1, INPUT);
  pinMode(Sensor2, INPUT);
  pinMode(Sensor3, INPUT);
  pinMode(Sensor4, INPUT);

  attachInterrupt(digitalPinToInterrupt(Sensor2), Leaving, RISING) ;
  attachInterrupt(digitalPinToInterrupt(Sensor3), Entering, RISING) ;
}

void loop(void) {

  Serial.println("In Loop");
  delay(500);
}

void Leaving()
{
  if ((digitalRead(Sensor1) == HIGH) && (digitalRead(Sensor2) == HIGH))
    {
      Serial.println("Leaving");
      message[0] = 2;
      radio.write(message, 1);
    }
}

void Entering()
{
  if ((digitalRead(Sensor4) == HIGH) && (digitalRead(Sensor3) == HIGH))
    {
      Serial.println("Entering");
    message[0] = 1;
    radio.write(message, 1);
    }
}



