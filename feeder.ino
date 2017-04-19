/*
Created by DEEPAK KUMAR
Date: 16-04-2017

This code will be uploaded on a arduino used for interfacing many sensors
Code is indented to be used for "FEEDER BIKE"
______________________________________________________________
Connections

D13 -- ONBOARD LED will be used as indicator
A0 -- For Reading battery current status
A3 -- Connected to voltage sensor
D7 -- FOR Restting file name
D6 -- For EEPROM CAP reset


*/
#include <SD.h>

File myFile;
#include<EEPROM.h>
//_______________________________________________________________
//Battery Status Variable


float current_raw=0,current=0;
float batt_stat=0, volt_batt_stat=0;
float cap=0;
int rom_addr=0,rom_file=15;
int count=0;
int count_int=0;
byte i;
String data="Power(W),Volt(V),Current(I)";
char scurrent[15],svolt[15],scap[15];
int filePre=0;
String fileName = String(filePre) + ".TXT";

void setup() {
  pinMode(7,INPUT_PULLUP);
  pinMode(6,INPUT_PULLUP);
  digitalWrite(7,HIGH);
  digitalWrite(6,HIGH);
//Setting the serial communication
Serial.begin(9600); //needs only in testing mode

 while (!SD.begin(8)) ;
 filePre = EEPROM_readFloat(rom_file);
 fileName = String(filePre) +".TXT";
 Serial.println(fileName);
 //Settingup the timer2 interrupt to read the sensor @ 500 us
 //CTC mode ..first read the datasheet of ATMEGA328p
cli();
TCCR2A=0;
TCCR2B=0;
TCNT2=0;
OCR2A=249; 
TCCR2A|=(1<<WGM21);
TCCR2B|=(1<<CS20)|(1<<CS21);
TIMSK2|=(1<<OCIE2A);


cap = EEPROM_readFloat(rom_addr);

//Serial.println(EEPROM_readFloat(rom_addr));
sei();
}

ISR(TIMER2_COMPA_vect){
 //Interrupt at every 500 uS
  
  count++;
  
  batt_stat = analogRead(A3);
  current_raw=analogRead(A0);
  
  volt_batt_stat += (batt_stat/1024*5*104.7/4.7) + 0.0 ;  // -/\/\/\--/\/\/\- 100k--4.7k 
  current += (511-current_raw)/1024*5/.066;  //66mv/A
  
  //Sumup the total capacity
  cap += current*volt_batt_stat/3600/2000; // freq is 2000Hz
  
  //Calculate the avg. current and voltage and save to EEPROM
  if(count==2000){
    count=0;
    count_int++;
    EEPROM_writeFloat(rom_addr,cap);
    dtostrf(cap/2000.00/2000.00,7,4,scap);
    dtostrf(current/2000.00,5,3,scurrent);
    dtostrf(volt_batt_stat/2000.00,5,3,svolt);
    String sscap = String(scap);
    String ssvolt= String(svolt);
    String SScurr = String(scurrent);
    data = "\n" + sscap + "," + ssvolt + "," + SScurr;
    current=0;
    volt_batt_stat=0;
    save();
    
  }
}
    
    



void loop() {
  if(digitalRead(7)==LOW){
    //Reset the file name
    filePre=0;
    fileName = String(filePre)+".TXT";
    EEPROM_writeFloat(rom_file,filePre);
  }
  else if(digitalRead(6)==LOW){
   cap=0;
   EEPROM_writeFloat(rom_addr,cap);
  }
}


//Function to save data in SDCard. 
void save(){
   
 myFile = SD.open(fileName.c_str(), FILE_WRITE);
 if(myFile.size()==0){
 myFile.println("Power(W),Volt(V),Current(I)");
 }
 else if((myFile.size()/1024.00)>1000){
   filePre++;
   fileName = String(filePre)+".TXT";
   EEPROM_writeFloat(rom_file,filePre);
   myFile.close();
   myFile = SD.open(fileName.c_str(), FILE_WRITE);
   myFile.println("Power(W),Volt(V),Current(I)");
 }
 myFile.println(data);
 myFile.close();
 Serial.println("Ok!");
}


//EEROM float value read write
void EEPROM_writeFloat(int ee, float value)
{
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       EEPROM.write(ee++, *p++);
}

float EEPROM_readFloat(int ee)
{
   double value = 0.0;
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       *p++ = EEPROM.read(ee++);
   return value;
}


