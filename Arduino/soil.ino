#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include <WISOL.h>
#include <SDISerial.h>
#include <string.h>

#define DATA_PIN 2
#define RST_N 8
#define SIGFOX_PWR 7

uint16_t intervalCount = 407; // 60min
uint16_t watchdogCounter = 0;

float readings[9]={};
float moistures[6]={};
float temperatures[6]={}; 
uint8_t moistureAborted = 0;
uint8_t temperatureAborted = 0;

Isigfox *Isigfox = new WISOL();
SDISerial connection(DATA_PIN);

void setup() {
  power_all_disable();
  power_usart0_enable();
  power_timer0_enable();
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SIGFOX_PWR, OUTPUT);
  digitalWrite(SIGFOX_PWR, LOW); 

  Serial.begin(9600);
  welcome();  

  power_timer0_disable();
  power_usart0_disable();
  
  watchdogSetupI8s();
  watchdogCounter = 0;
  powerDown();
}

void loop() {
  if (watchdogCounter == intervalCount) {
      power_usart0_enable();
      power_timer0_enable();
      
      Take_Measurements();
      if(!moistureAborted){
        digitalWrite(SIGFOX_PWR, HIGH);
        wakeDeepSleep();
        delay(1000);          
        Send_Moistures();
        if(temperatureAborted)        
          goDeepSleep();
        else
          delay(1000);
        digitalWrite(SIGFOX_PWR, LOW); 
      }      

      power_timer0_disable();
      power_usart0_disable();
  }
  else if((watchdogCounter>intervalCount) && !temperatureAborted)
  {
      power_usart0_enable();
      power_timer0_enable();
      
      digitalWrite(SIGFOX_PWR, HIGH);
      if(moistureAborted){
        wakeDeepSleep();
        delay(1000);
      }      
      Send_Tempertures();
      goDeepSleep();
      digitalWrite(SIGFOX_PWR, LOW); 
      
      power_timer0_disable();
      power_usart0_disable();  
      watchdogCounter=0;
  }

wdt_reset(); 
  powerDown();
}

void welcome(){
  Take_Measurements();
  
  digitalWrite(SIGFOX_PWR, HIGH);
  WISOLTest();
  if(!moistureAborted)
    Send_Moistures();  
  delay(1000);
  digitalWrite(SIGFOX_PWR, LOW);
  
  delay(5000);
  
  digitalWrite(SIGFOX_PWR, HIGH); 
  Send_Tempertures_Welcome();
  goDeepSleep();
  digitalWrite(SIGFOX_PWR, LOW); 
}

void Send_Moistures(){
  uint8_t buf_str1[9];  
  uint16_t moisturesInt[4]={};
  uint8_t signs1 = 0;
  uint8_t shiftThis = 16;

  uint8_t j=0;
  for(uint8_t i=0;i<4;i++)
  {
    int32_t moisture = (moistures[i]+0.0005-55)*1000;
    if(moisture>0){
      signs1 = signs1 + (shiftThis<<i);
      moisturesInt[i] = (moistures[i]+0.0005-55)*1000;
    }
    else
      moisturesInt[i] = (55-moistures[i]+0.0005)*1000;
Serial.println(moisturesInt[i]);
            
    buf_str1[j] = (moisturesInt[i]>>8) & 0xFF;
    j++;
    buf_str1[j] = moisturesInt[i] & 0xFF;
    j++;  
  }

Serial.println(signs1);
  buf_str1[8] = signs1+2;

  Send_Pload(buf_str1, 9);
}

void Send_Tempertures(){
  uint8_t buf_str2[7];  
  uint16_t temperaturesInt[4]={};
  uint8_t signs2 = 0;
  uint8_t shiftThis = 16;

  for(uint8_t i=0;i<4;i++)
  {
    int32_t temperature = (temperatures[i]+0.005-15)*100;
    if(temperature>0){
      signs2 = signs2 + (shiftThis<<i);
      temperaturesInt[i] = (temperatures[i]+0.005-15)*100;
    }
    else
      temperaturesInt[i] = (15-temperatures[i]+0.005)*100;
Serial.println(temperaturesInt[i]); 
  }
Serial.println(signs2);

  buf_str2[0] = temperaturesInt[0] & 0xFF;
  buf_str2[1] = ((temperaturesInt[0]>>4) & 0xF0) + (temperaturesInt[1] & 0x0F);
  buf_str2[2] = (temperaturesInt[1]>>4) & 0xFF;
  buf_str2[3] = temperaturesInt[2] & 0xFF;
  buf_str2[4] = ((temperaturesInt[2]>>4) & 0xF0) + (temperaturesInt[3] & 0x0F);
  buf_str2[5] = (temperaturesInt[3]>>4) & 0xFF;
  buf_str2[6] = signs2+3;

  Send_Pload(buf_str2, 7);    
}

void Send_Tempertures_Welcome(){
  uint8_t buf_str2[8]; 
  if(!temperatureAborted){ 
    uint16_t temperaturesInt[4]={};
    uint8_t signs2 = 0;
    uint8_t shiftThis = 16;
  
    for(uint8_t i=0;i<4;i++)
    {
      int32_t temperature = (temperatures[i]+0.005-15)*100;
      if(temperature>0){
        signs2 = signs2 + (shiftThis<<i);
        temperaturesInt[i] = (temperatures[i]+0.005-15)*100;
      }
      else
        temperaturesInt[i] = (15-temperatures[i]+0.005)*100;
  Serial.println(temperaturesInt[i]); 
    }
  Serial.println(signs2);
  
    buf_str2[0] = temperaturesInt[0] & 0xFF;
    buf_str2[1] = ((temperaturesInt[0]>>4) & 0xF0) + (temperaturesInt[1] & 0x0F);
    buf_str2[2] = (temperaturesInt[1]>>4) & 0xFF;
    buf_str2[3] = temperaturesInt[2] & 0xFF;
    buf_str2[4] = ((temperaturesInt[2]>>4) & 0xF0) + (temperaturesInt[3] & 0x0F);
    buf_str2[5] = (temperaturesInt[3]>>4) & 0xFF;  
    buf_str2[6] = 60;
    buf_str2[7] = signs2+1;
  
    Send_Pload(buf_str2, 8);    
  }
  else{
    buf_str2[0] = 15;
    buf_str2[1] = 4;
    Send_Pload(buf_str2, 2);        
  }
}

//----------------------------------------------------------------------
void Take_Measurements(){
  connection.begin();  
  char* response=GetResponse("0!",1000);

  moistureAborted = GetReadings('0',StartMeasurement("0M!",1000));
  if(!moistureAborted){
    for(uint8_t i=0;i<6;i++)
      moistures[i]=readings[i];  
  }
  temperatureAborted = GetReadings('0',StartMeasurement("0M1!",1000));
  if(!temperatureAborted){
    for(uint8_t i=0;i<6;i++)
      temperatures[i]=readings[i];
  }    
}

uint8_t GetReadings(char addr, uint8_t nn){ 
  uint8_t number = 0;
  uint8_t count = 48; // '0'
  char Dcmd[5]={addr, 'D', '0','!'};
  bool Aborted = 0;
 
  while(number<nn && !Aborted){
wdt_reset();
    char* response=GetResponse(Dcmd,2000);
    String responseS = String(response);
        
    // find starting index
    int8_t start_index=0;
    int8_t index1=responseS.indexOf("+",start_index);
    int8_t index2=responseS.indexOf("-",start_index);
    
    if(index1==-1 && index2==-1)
    {
      Aborted = 1;
      Serial.println("Measurement aborted.");
      break;                    
    }  
    else
    {
      if(index1==-1 || (index1>index2 && index2!=-1))
        start_index=index2;
      else if(index2==-1 || (index1<index2 && index1!=-1))  
        start_index=index1;             
    }
   
    bool DataEnd=0;    
    while(!DataEnd){
      index1=responseS.indexOf("+",start_index+1);
      index2=responseS.indexOf("-",start_index+1);
            
      if(index1==-1 && index2==-1)
      {
        DataEnd=1;         
        readings[number]=responseS.substring(start_index).toFloat();                 
      }  
      else
      {
        if(index1==-1 || (index1>index2 && index2!=-1))
        {
          readings[number]=responseS.substring(start_index,index2).toFloat();
          start_index=index2;
        }
        else if(index2==-1 || (index1<index2 && index1!=-1))
        {
          readings[number]=responseS.substring(start_index,index1).toFloat();
          start_index=index1;          
        }                       
      }
Serial.println(readings[number],6);
      number++;
    }       
    count++;   
    Dcmd[2]=char(count);
  }
  return Aborted;  
}

uint8_t StartMeasurement(const char* cmd,uint32_t timeout_ms){
  char* response=GetResponse(cmd,timeout_ms);
  String responseS = String(response);  
  uint32_t ttt_ms = responseS.substring(1,4).toInt()*1000;
  uint8_t n = responseS.substring(4,5).toInt();
  
  Serial.print("ttt_ms: ");
  Serial.print(ttt_ms);
  Serial.print(" n: ");
  Serial.println(n);

  if(ttt_ms!=0){    
    bool result = ServiceRequest(cmd[0], ttt_ms);
    Serial.print("Service Request: ");
    Serial.println(result);
  }
  
  return n;
}

bool ServiceRequest(char addr, uint32_t timeout_ms){
wdt_reset();
  char* response = connection.wait_for_response(timeout_ms);
wdt_reset();
  
  Serial.print("RECV: ");
  Serial.println(response);
  
  if(response[0]==addr)
    return 1;
  else
    return 0;    
}

char* GetResponse(const char* cmd,uint32_t timeout_ms){
  Serial.print("Command: ");
  Serial.println(cmd);
  
  char* response = connection.sdi_query(cmd,timeout_ms);
  while(response==NULL || response[0] == '\0'){
    response = connection.sdi_query(cmd,timeout_ms);
    delay(1000);
 wdt_reset();
  }
    
  Serial.print("RECV: ");
  Serial.println(response);
  
  return response;  
}

//--------------------------------------------------------------------
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
  power_usart0_enable();
  Serial.print("WD interrupt: ");
  Serial.println(watchdogCounter);
  Serial.flush();
  power_usart0_disable();
}

//---------------------------------------------------------------
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
  //Serial.println("");
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

