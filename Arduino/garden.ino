#include <WISOL.h>
#include <SimpleTimer.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <Tsensors.h>
#include <Wire.h>
#include <Ports.h>
#include <util/atomic.h>

const uint8_t soilPin = A2;
const uint8_t soilPin2 = A3;
const uint8_t soilPower = 4;
const uint8_t soilPower2 = 9;
const uint8_t RST_N = 8;
const uint8_t photoPin = A0;

uint16_t Abuffer[8] = {};
uint8_t pointer = 0;
uint8_t seq = 1;

RTC_DS1307 rtc;
Isigfox *Isigfox = new WISOL();
SimpleTimer timer;
Tsensors *tSensors = new Tsensors();

ISR(WDT_vect) { Sleepy::watchdogEvent();}

void setup() {
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(soilPower, OUTPUT);
  digitalWrite(soilPower, LOW);
  pinMode(soilPower2, OUTPUT);
  digitalWrite(soilPower2, LOW);

  // WISOL test
  WISOLTest();

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("Initializing SD card...");
  if (!SD.begin(10)) {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("card initialized.");
  
  if (SD.exists("datalog.txt")) {
         
    if (!SD.exists("old_data")) {
      SD.mkdir("old_data");
    }

    File myFileIn = SD.open("datalog.txt", FILE_READ);
    
    String time32S = "";
    for(int i=0;i<8;i++)
      time32S += char(myFileIn.read());
    String myFileName = "old_data/" + time32S + ".txt";  
    Serial.println(myFileName);
    
    File myFileOut = SD.open(myFileName, FILE_WRITE);
  
    while (myFileIn.available())
      myFileOut.write(myFileIn.read());  
    myFileIn.close();
    myFileOut.close();
    
    SD.remove("datalog.txt");
  }

  unsigned long interval = 90000;
  timer.setInterval(interval, timeIRSoil);
  interval = 720000;
  timer.setInterval(interval, timeIR);
  
  Send_Time();
  goDeepSleep();
}

void loop() { 
  //delay(90000);
  Sleepy::loseSomeTime(50000);
  timer.run();
}

void WISOLTest(){
    int8_t flagInit = -1;
    while (flagInit == -1) {
      Serial.println("");
      wakeDeepSleep();
      delay(1000);     
      flagInit = Isigfox->initSigfox(); // "AT$DR?"
      Isigfox->testComms(); // "AT"
      GetDeviceID(); // "AT$I=10"
    }
}

void goDeepSleep(){
  Serial.println("AT$P=2");
  Serial.print('\0');
  Serial.flush ();
}

void wakeDeepSleep(){
  cli();
  pinMode(RST_N, OUTPUT);
  digitalWrite(RST_N, LOW);
  delay(50);
  //Sleepy::loseSomeTime(50);
  digitalWrite(RST_N, HIGH);
  pinMode(RST_N, INPUT_PULLUP);
  sei();
}

void timeIRSoil(){
  if(seq!=10)
  {
    if(pointer==8)
    {
      Serial.println("Buffer overflows.");
      pointer = 0;
    }
    else if(pointer==0 || pointer==2 || pointer==4 || pointer==6){
      digitalWrite(soilPower, HIGH);
      delay(10);         
      Abuffer[pointer] = analogRead(soilPin);
      digitalWrite(soilPower, LOW);
  
  DateTime timenow = rtc.now();
  Print_Time(timenow);
  Serial.println(Abuffer[pointer]);
  Serial.flush();
           
      pointer++;    
    }
    else
    {
        digitalWrite(soilPower2, HIGH);
        delay(10);         
        Abuffer[pointer] = analogRead(soilPin2);
        digitalWrite(soilPower2, LOW);         

  DateTime timenow = rtc.now();
  Print_Time(timenow);
  Serial.println(Abuffer[pointer]);
  Serial.flush();        
        
        pointer++;
    }
  }
}

void timeIR(){
  pointer = 0;  
  wakeDeepSleep();
  delay(1000);
  if(seq==10)
  {
    Send_Time();
    seq=1;
  }    
  else
  {
    Send_Message();
    seq++;
  }
  goDeepSleep();
}

void Send_Message(){
  
  // remove duplicate readings
  uint16_t AbufferSub[8] = {};
  AbufferSub[0] = Abuffer[0];
  AbufferSub[1] = Abuffer[1];
  uint8_t newV = 17;  
  uint8_t count = 2;
  
  for(int i=2;i<8;i+=2)
  {
    if(Abuffer[i]!=Abuffer[i-2])
    {
      switch(i){
        case 2:
          newV+=2;
          break;
        case 4:
          newV+=4;
          break;
        case 6:
          newV+=8;
          break;       
     }
     count++;
     AbufferSub[count-1]=Abuffer[i];
    }        
  }

  for(int i=3;i<8;i+=2)
  {
    if(Abuffer[i]!=Abuffer[i-2])
    {
      switch(i){
        case 3:
          newV+=32;
          break;
        case 5:
          newV+=64;
          break;
        case 7:
          newV+=128;
          break;       
     }
     count++;
     AbufferSub[count-1]=Abuffer[i];
    }        
  }

  // pack 10-bit readings side by side
  uint8_t buf_str[10]={};
  uint8_t Aoperator = 0; //LSB
  uint8_t Boperator = 0; //MSB
  
  uint8_t current = 0;
  uint8_t index = 0;   
  uint8_t endBit = 2;

  while(current<count)
  {    
    if(endBit==10)
    {
      endBit=2;
      index++; 
    }
      
    switch (endBit) {
      case 2:
        Aoperator = 0xFF;
        Boperator = B11000000;
        break;
      case 4:
        Aoperator = B00111111;
        Boperator = B11110000;
        break;
      case 6:
        Aoperator = B00001111;
        Boperator = B11111100;
        break;
      case 8:
        Aoperator = B00000011;
        Boperator = 0xFF;
        break;
    }          
    
    buf_str[index] = buf_str[index] + (AbufferSub[current] & Aoperator);
    buf_str[index+1] = (AbufferSub[current]>>2) & Boperator;
    
    index++;
    current++;    
    endBit = endBit+2;     
  }
  
  uint8_t buf_str_final[index+3]={};  
  for(int i=0;i<=index;i++)
    buf_str_final[i]=buf_str[i];
  buf_str_final[index+1]=newV;  
  buf_str_final[index+2]=seq;

  DateTime timenow = rtc.now();
  Print_Time(timenow);
  
  // send payload
  const uint8_t payloadSize = index+3;  
  Send_Pload(buf_str_final, payloadSize);

  // log data
  String dataString = "";
  File dataFile = SD.open("datalog.txt", FILE_WRITE);  
  if (dataFile) {
    for (int i = 0; i < 8; i++) {
      dataString += String(Abuffer[i]);
      if (i < 7) {
        dataString += ",";
      }
    }
    Serial.println(dataString); 
    dataFile.println(dataString);
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
  }
}

void Send_Time(){   
  //uint32_t time32 = rtc.now().unixtime(); 
  DateTime timenow = rtc.now();
  uint32_t time32 = timenow.unixtime();
  Print_Time(timenow);

  // get sensor values
  float temptF = tSensors->getTemp();
  uint16_t tempt = temptF * 100; 
  float pressureF = tSensors->getPressure(); 
  uint16_t pressure = pressureF/3;
  uint16_t light = analogRead(photoPin);
    
  uint8_t buf_str[12];  
  buf_str[3] = time32 & 0xFF;
  buf_str[2] = (time32>>8) & 0xFF;
  buf_str[1] = (time32>>16) & 0xFF;  
  buf_str[0] = (time32>>24) & 0xFF; 
  
  buf_str[5] = tempt & 0xFF;
  buf_str[4] = (tempt>>8) & 0xFF;
  
  buf_str[7] = pressure & 0xFF;
  buf_str[6] = (pressure>>8) & 0xFF;
  
  buf_str[9] = light & 0xFF;
  buf_str[8] = (light>>8) & 0xFF;
      
  buf_str[10] = 0;
  buf_str[11] = 0;
  
  Send_Pload(buf_str, 12);

  String dataString = String(temptF,2)+","+String(pressureF,0)+","+String(light);
  Serial.println(dataString);
   
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(time32, HEX);    
    dataFile.println(dataString);
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
  }
}

void Print_Time(DateTime timenow){
    Serial.println(timenow.unixtime());
    
    Serial.print(timenow.year(), DEC);
    Serial.print('/');
    Serial.print(timenow.month(), DEC);
    Serial.print('/');
    Serial.print(timenow.day(), DEC);
    Serial.print(" ");
    Serial.print(timenow.hour(), DEC);
    Serial.print(':');
    Serial.print(timenow.minute(), DEC);
    Serial.print(':');
    Serial.print(timenow.second(), DEC);
    Serial.println();  
}

void Send_Pload(uint8_t *sendData, const uint8_t len){
  // No downlink message require
  recvMsg *RecvMsg;

  RecvMsg = (recvMsg *)malloc(sizeof(recvMsg));
  Isigfox->sendPayload(sendData, len, 0, RecvMsg);
  for (int i = 0; i < RecvMsg->len; i++) {
    Serial.print(RecvMsg->inData[i]);
  }
  Serial.println("");
  free(RecvMsg);
}

void GetDeviceID(){
  recvMsg *RecvMsg;
  const char msg[] = "AT$I=10";

  RecvMsg = (recvMsg *)malloc(sizeof(recvMsg));
  Isigfox->sendMessage(msg, 7, RecvMsg);

  Serial.print("Device ID: ");
  for (int i=0; i<RecvMsg->len; i++){
    Serial.print(RecvMsg->inData[i]);
  }
  Serial.println("");
  free(RecvMsg);
}


