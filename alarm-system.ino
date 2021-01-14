/* TODO
 * - Use a FSM approach towards the code, easier to showcase
 * - Replace delay() with millis() for buzzer delay code, revamp it with better code and greater functionality. Use ToneAC library perhaps?
 * - Work on fixing the WiFi code stuff to include all 4 fields (Sensor1 - Sensor 4)
 * - Revamp code to include alarm enabling/disabling with LED and buzzer prompts
 */

#include <SoftwareSerial.h>
#include <Keypad.h>

const int piezoPin = A0;
const int buzzerPin = 9; //buzzer to arduino pin 10
const int armedLED = 10;
const int disarmedLED = 11;

int piezoState = LOW;

/* WiFi stuff */

String AP = "";
String AP_Password = "";

String API_Key = "YOUR_API_KEY";   // CHANGE ME
String API_Host = "api.thingspeak.com";
String API_Port = "80";
String API_Sensor1 = "Sensor1";
String API_Sensor2 = "Sensor2";
String API_Sensor3 = "Sensor3";
String API_Sensor4 = "Sensor4";

int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;

SoftwareSerial esp8266(0, 1);  // Pin 0 = RX, Pin 1 = TX

/* Keyboard stuff */

const String password = "1234";
String input_password;

const byte ROWS = 4; 
const byte COLS = 3;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {8, 7, 6, 5}; 
byte colPins[COLS] = {4, 3, 2}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

/* Here we go! */

void setup(){
  pinMode(buzzerPin, OUTPUT);
  pinMode(armedLED, OUTPUT);
  pinMode(disarmedLED, OUTPUT);
  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ AP_Password +"\"",20,"OK");
  input_password.reserve(32);
}
  
void loop(){
  char customKey = customKeypad.getKey();
  if(customKey){
    Serial.println(customKey);
    input_password += customKey;
    if(input_password == password){
      activateBuzzer();
      piezoState = HIGH;
    }
  }

  if(piezoState){
    int piezoADC = analogRead(piezoPin);
    float piezoV = piezoADC / 1023.0 * 5.0;
    Serial.println(piezoV); // Print the voltage.
    String getData = "GET /update?api_key="+ API_Key +"&"+ API_Sensor1 +"="+String(piezoV);
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ API_Host +"\","+ API_Port,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(1500);
    countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");
  }
}

void activateBuzzer(){
  tone(buzzerPin, 1000); // Send 1KHz sound signal...
  delay(350);        // ...for 1 sec
  noTone(buzzerPin);
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1)){
    esp8266.println(command);     //at+cipsend
    if(esp8266.find(readReplay)){ //ok
      found = true;
      break;
    }
    countTimeCommand++;
  }
  if(found){
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  else{
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
}
