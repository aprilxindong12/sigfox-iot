#include <WISOL.h>

#define RST_N 8
uint8_t analogPin = 3;

uint8_t level = 0;
uint16_t rawVoltage = 0;
uint8_t rising = 1;

Isigfox *Isigfox = new WISOL();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(9600);  
  WISOLTest();

  // initial level roughly
  rawVoltage = analogRead(analogPin);  
  if(rawVoltage>17)
    level = (rawVoltage-18)/51+1;    

  Serial.println(rawVoltage);
  volToHeight(rawVoltage);
  Serial.println(level); 
  Send_Voltage();

  Serial.println("End of set up.");
  Serial.println(); 
}

void loop() {
  uint16_t newVoltage = analogRead(analogPin);
  Serial.println(newVoltage); 
  delay(1000);
    
  if(newVoltage>(rawVoltage+13))
  {  
    if(rising)
    {
      rawVoltage = newVoltage;
      if (Get_Level()!=0)
      {               
        Serial.println(level);
        Serial.println();   
        Send_Level();
      }  
    }
    else
    {
      rising = 1;      
      volToHeight(rawVoltage);   
      Serial.println();
      Send_Voltage();
      rawVoltage = newVoltage;
    }    
  }      
  else if(newVoltage<(rawVoltage-13.0))
  {   
    if(rising==0)
    {
      rawVoltage = newVoltage;
      if (Get_Level()!=0)
      {               
        Serial.println(level);
        Serial.println();   
        Send_Level();
      }   
    }
    else
    {
      rising = 0;      
      volToHeight(rawVoltage);   
      Serial.println();
      Send_Voltage();
      rawVoltage = newVoltage; 
    }       
  }
}

int8_t Get_Level()
{
  int8_t changeL = 0;
  
  if(rawVoltage<=17){
    if(level!=0){
      level = 0;
      changeL=1;
    }
  }
  else
  {    
    uint16_t current=0;  
    if(level!=0)
      current = (40*level-25.4)/797.6*1024;            
    uint16_t higher = (40*(level+1)-25.4)/797.6*1024;

   if(rawVoltage<current || rawVoltage>=higher)
   {
      uint16_t higher2 = (40*(level+2)-25.4)/797.6*1024;
      uint16_t lower=0;
      if(level>1)
        lower = (40*(level-1)-25.4)/797.6*1024;
      
      if (rawVoltage>=higher && rawVoltage<higher2)
      {
        level++;
        changeL=1;
      }
      else if (rawVoltage<current && rawVoltage>=lower)
      {
        level--;
        changeL=1;
      }     
      else
      {
        level = (rawVoltage-18)/51+1;
        Serial.println("Water level is changing too fast.");
        changeL=-1;
      }       
    }
  }  
  return changeL;
}

void Send_Voltage(){
  uint8_t buf_str[3];  
  buf_str[0] = (rawVoltage>>8) & 0xFF; 
  buf_str[1] = rawVoltage & 0xFF;
  buf_str[2] = 2;
  Send_Pload(buf_str, 3);
}

void Send_Level(){
  uint8_t buf_str[2];  
  buf_str[0] = level;
  buf_str[1] = 1;
  Send_Pload(buf_str, 2);
}

void volToHeight (uint16_t vol){
  Serial.println((vol+1)/1024.0*797.6+25.4);
}

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

void wakeDeepSleep(){
  cli();
  pinMode(RST_N, OUTPUT);
  digitalWrite(RST_N, LOW);
  delay(50);
  digitalWrite(RST_N, HIGH);
  pinMode(RST_N, INPUT_PULLUP);
  sei();
}

/* The last two functions are from an Xkit demo app by Thinxtra
 https://github.com/Thinxtra/Xkit-Sample/blob/master/DemoApp/DemoApp.ino
*/
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
