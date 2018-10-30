#include <WISOL.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

const uint8_t soilPin = A1;
const uint8_t soilPin2 = A2;
const uint8_t soilPower = 5;
const uint8_t soilPower2 = 6;
const uint8_t sigfoxPower = 7;
const uint8_t RST_N = 8;
const uint8_t photoPin = A0;

const uint16_t intervalCount = 10; // 90s 
uint16_t watchdogCounter = 0;

uint16_t Abuffer[8] = {};
uint8_t pointer = 0;
uint8_t seq = 1;

Isigfox *Isigfox = new WISOL();

void setup() {
  power_all_disable();
  power_usart0_enable();
  power_timer0_enable();
  power_adc_enable();
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(soilPower, OUTPUT);
  digitalWrite(soilPower, LOW);
  pinMode(soilPower2, OUTPUT);
  digitalWrite(soilPower2, LOW);
  pinMode(sigfoxPower, OUTPUT);
  digitalWrite(sigfoxPower, LOW);

  Serial.begin(9600);

  welcome();  

  power_timer0_disable();
  power_adc_disable();
  power_usart0_disable();
  
  watchdogSetupI8s();
  watchdogCounter = 0;
  powerDown();
}

void loop() {
    if (watchdogCounter == intervalCount) {     
      if(seq==10)
      {       
        pointer++;
        if(pointer==8){
          power_usart0_enable();
          power_timer0_enable();
          power_adc_enable();
          
          digitalWrite(sigfoxPower, HIGH);
          wakeDeepSleep();
          delay(1000);          
          SendEnvData();
          goDeepSleep();
          digitalWrite(sigfoxPower, LOW);
          seq=1;
          pointer=0;
          
          power_timer0_disable();
          power_adc_disable();
          power_usart0_disable();
        }
      }    
      else
      {
        power_timer0_enable();
        power_adc_enable();
               
        if(pointer==0 || pointer==2 || pointer==4 || pointer==6){
          digitalWrite(soilPower, HIGH);
          delay(10);
          Abuffer[pointer] = analogRead(soilPin);       
          digitalWrite(soilPower, LOW);                  
          pointer++;    
        }
        else
        {
          digitalWrite(soilPower2, HIGH);
          delay(10);
          Abuffer[pointer] = analogRead(soilPin2);
          digitalWrite(soilPower2, LOW);                         
          pointer++;
          
          if(pointer==8){
            power_usart0_enable();
           
            digitalWrite(sigfoxPower, HIGH);
            wakeDeepSleep();
            delay(1000);          
            SendSoilData();
            goDeepSleep();
            digitalWrite(sigfoxPower, LOW);            
            seq++;
            pointer = 0;
            
            power_usart0_disable();           
          }
        }
        power_timer0_disable();
        power_adc_disable();
      }    
    watchdogCounter = 0;
  }
  wdt_reset(); 
  powerDown(); 
}

void welcome(){
  digitalWrite(sigfoxPower, HIGH);
  WISOLTest();
  SendEnvData();
  goDeepSleep();
  digitalWrite(sigfoxPower, LOW); 
}

void SendEnvData(){   
  uint16_t light = analogRead(photoPin);

  digitalWrite(soilPower, HIGH);
  delay(10);
  uint16_t soil = analogRead(soilPin);
  digitalWrite(soilPower, LOW);

  digitalWrite(soilPower2, HIGH);
  delay(10);
  uint16_t soil2 = analogRead(soilPin2);
  digitalWrite(soilPower2, LOW);

  uint8_t buf_str[10];  
  buf_str[1] = 0;
  buf_str[0] = 0;  
  buf_str[3] = 0;
  buf_str[2] = 0;
    
  buf_str[4] = light & 0xFF;  
  buf_str[5] = ((light>>2) & B11000000) + (soil & B00111111);
  buf_str[6] = ((soil>>2) & B11110000) + (soil2 & B00001111);
  buf_str[7] = (soil2>>2) & B11111100;     
  
  buf_str[8] = 0;
  buf_str[9] = 0;
  
  Send_Pload(buf_str, 10);
}

void SendSoilData(){ 
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

  uint8_t buf_str_final[index+4]={};  
  for(int i=0;i<=index;i++)
    buf_str_final[i]=buf_str[i];
  buf_str_final[index+1]=newV;  
  buf_str_final[index+2]=seq;
  buf_str_final[index+3]=seq;

  // send payload
  uint8_t payloadSize = index+4;
  if(payloadSize>12)
      payloadSize=12;
  Send_Pload(buf_str_final, payloadSize);
}

//--------------------------------------------------------------
void powerDown(void){
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();
  //sleep_bod_disable();
  sei();
  sleep_cpu();
  sleep_disable();
  sei();  
}

void watchdogSetupI8s(void) {
  cli();
  wdt_reset();
  MCUSR &= ~(1<<WDRF);  
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = B01100001; // interrupt, 8s
  sei();
}

void WDT_off(void)
{
  cli();
  wdt_reset();
  /* Clear WDRF in MCUSR */
  MCUSR &= ~(1<<WDRF);
  /* Write logical one to WDCE and WDE */
  /* Keep old prescaler setting to prevent unintentional time-out
  */
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  /* Turn off WDT */
  WDTCSR = 0x00;
  sei();
}

ISR(WDT_vect)
{
  watchdogCounter++;
//  power_usart0_enable();
//  Serial.print("WD interrupt: ");
//  Serial.println(watchdogCounter);
//  Serial.flush();
//  power_usart0_disable();
}

//--------------------------------------------------------
void WISOLTest(){
    int8_t flagInit = -1;
    while (flagInit == -1) {
      //Serial.println("");
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
  digitalWrite(RST_N, HIGH);
  pinMode(RST_N, INPUT_PULLUP);
  sei();
}

void Send_Pload(uint8_t *sendData, const uint8_t len){
wdt_reset();
  // No downlink message require
  recvMsg *RecvMsg;

  RecvMsg = (recvMsg *)malloc(sizeof(recvMsg));
  Isigfox->sendPayload(sendData, len, 0, RecvMsg);
wdt_reset();
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
  //Serial.println("");
  Serial.flush();
  free(RecvMsg);
}

