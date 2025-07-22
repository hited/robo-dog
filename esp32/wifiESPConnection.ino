#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PWMServoDriver.h>
#include <cstring>
#include <MPU9250_asukiaaa.h>
#include <NetworkClient.h>
#include <WiFiAP.h>
#include "pitches.h"

WiFiServer wifiServer(8081); //lan 
NetworkServer networkServer(8080); //create own wifi server
WiFiClient client;
IPAddress myIP;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 21
#define SCL_PIN 22
#define SERVOMIN  125
#define SERVOMAX  600

String varNameArray[7];
int varValueArray[7];
int varCurrentIndex = 0;
int delayBetweenEachStep = 5;//5 is correct

bool canBeTrue = false;
int lastPosArray[12] = {180,0,0,180,    180,0,0,180,     0,180,180,0};
int lastPosArrayMoved[12] = {0,0,0,0,    0,0,0,0,     0,0,0,0};
int numOfCorrectPos = 0; //12

int melody[] = {
NOTE_D4, NOTE_G4, NOTE_FS4,END
};
int noteDurations[] = {       //duration of the notes
8,4,8,};
int musicSpeed=90;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
MPU9250_asukiaaa mpuSensor;

String dots = "";
void PrintIP(String text, String ip) {
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.println(text);

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.println(ip);
  display.drawLine(0, 51, 128, 51, WHITE);

  display.display();
}
int angleToPulse(int ang){
   int pulse = map(ang,0, 180, SERVOMIN,SERVOMAX);
   //Serial.print("Angle: ");Serial.print(ang);
   //Serial.print(" pulse: ");Serial.println(pulse);
   return pulse;
}
String callAccel(String identify){
  int result;
  float aX, aY, aZ;
  result = mpuSensor.accelUpdate();
  if (result == 0) {
    if (identify == "mpuFB"){
      Serial.println("accelX: " + String(aX));
      Serial.println("accelY: " + String(aY));
      Serial.println("accelZ: " + String(aZ));
      aY = mpuSensor.accelY();
      return String(aY);
    
  } else{ //mpuLR 
    aX = mpuSensor.accelX();
    return String(aX);
  }
    
    
    aZ = mpuSensor.accelZ();
    // Serial.println("accelX: " + String(aX));
    // Serial.println("accelY: " + String(aY));
    // Serial.println("accelZ: " + String(aZ));
    return "Accel" + String(aX) + " " + String(aY) + " " + String(aZ);
  } else {
    Serial.println("Cannod read accel values " + String(result));
    Serial.println("");
  }

  
}
String callDistFB(String identify){
  int trig = 0;
  int echo = 0;
  if (identify == "distF"){
    trig = 32;
    echo = 33;
    
  } else{ //distB
    trig = 25;
    echo = 26;
  }

  long duration;
  int distance;

  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  duration = pulseIn(echo, HIGH);

  distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.println(distance);
  //return String(distance);
}
void MovingSequence(String instrArray[], int instrArrayLength, int offsetCountForArray){
  
  for (int i = 0; i< instrArrayLength;i++) {

    String currentInstruction = instrArray[i];

    int startStr = currentInstruction.indexOf("(") + 1;
    int endStr = currentInstruction.indexOf(")");
    String insideBracketsValue = currentInstruction.substring(startStr, endStr);

    int orderNumber = (currentInstruction.substring(0, 1)).toInt();

    if (orderNumber != offsetCountForArray) {
      continue; // Skip the rest of this iteration
    }
    switch (currentInstruction.charAt(currentInstruction.indexOf(" ")+1)) {//currentInstruction.charAt(0)
      //distF, distB,mpuFB, mpuLR
      case 'D':{ // Delay
          Serial.println(currentInstruction);
          delay(insideBracketsValue.toInt());
          break;}
      case 'I':{ // Int Assign
          int nameStart = currentInstruction.indexOf(' ', currentInstruction.indexOf(' ') + 1) + 1;
          int nameEnd = currentInstruction.indexOf('(', nameStart);
          String name = currentInstruction.substring(nameStart, nameEnd); 
          varNameArray[varCurrentIndex] = name;
          varValueArray[varCurrentIndex] = insideBracketsValue.toInt();
          varCurrentIndex++;
          break;}
      case 'O': {//Int Operation

          int nameStart = currentInstruction.indexOf(' ', currentInstruction.indexOf(' ') + 1) + 1;
          int nameEnd = currentInstruction.indexOf('(', nameStart);
          String name = currentInstruction.substring(nameStart, nameEnd); 

          int posOfVar = 0;
          int arraySize = sizeof(varNameArray) / sizeof(varNameArray[0]);

          for(int i =0; i< arraySize; i++){
            if (varNameArray[i] == name){
              posOfVar = i;
            }
          }
          Serial.print("posOfVar");
          Serial.println(posOfVar);
          if (insideBracketsValue.indexOf("+") == 0){
            insideBracketsValue.remove(0,1);
            varValueArray[posOfVar] = varValueArray[posOfVar] + insideBracketsValue.toInt();
          } else if (insideBracketsValue.indexOf("-") == 0) {
            insideBracketsValue.remove(0,1);
            varValueArray[posOfVar] = varValueArray[posOfVar] - insideBracketsValue.toInt();
          } else if (insideBracketsValue.indexOf("/") == 0) {
            insideBracketsValue.remove(0,1);
            varValueArray[posOfVar] = round(varValueArray[posOfVar] / insideBracketsValue.toInt());
          } else {
            insideBracketsValue.remove(0,1);
            varValueArray[posOfVar] = varValueArray[posOfVar] * insideBracketsValue.toInt();
          }
          
          break;
          }
      case 'F':{ //For
          int loopCount;
          if (isDigit(insideBracketsValue[0])){
            loopCount = insideBracketsValue.toInt();
          }
          else {
            for (int i = 0 ; i < varCurrentIndex ; i++){
              if (varNameArray[i] == insideBracketsValue){
                loopCount = varValueArray[i];
              }
            }
          }
          String loopToRepeat[50];
          int numOfElements = 0;
          //for (int j = 0; j < )
          while (offsetCountForArray+1 <= ((instrArray[i+1+numOfElements]).substring(0, 1)).toInt()) {
            loopToRepeat[numOfElements] = instrArray[i+1+numOfElements];
            numOfElements++;
            if (numOfElements >= 50){
              break;
            }
          }
          for (int loopIndex = 0; loopIndex < loopCount; loopIndex++) {
              // MovingSequence(String instrArray[], int instrArrayLength, bool moveIndentedCode, int offsetCountForArray)
              MovingSequence(loopToRepeat, numOfElements, offsetCountForArray+1);
          }
          break;}
      case 'W':{ //While
          int leftOperand, rightOperand;
          char operatorSymbol;

          int operatorIndex = -1;
          if (insideBracketsValue.indexOf('>') != -1) {
            operatorIndex = insideBracketsValue.indexOf('>');
            operatorSymbol = '>';
          } else if (insideBracketsValue.indexOf('<') != -1) {
            operatorIndex = insideBracketsValue.indexOf('<');
            operatorSymbol = '<';
          } else if (insideBracketsValue.indexOf('=') != -1) {
            operatorIndex = insideBracketsValue.indexOf('=');
            operatorSymbol = '=';
          }
          //left side var


          String loopToRepeat[50];
          int numOfElements = 0;
          //for (int j = 0; j < )
          while (offsetCountForArray+1 <= ((instrArray[i+1+numOfElements]).substring(0, 1)).toInt()) {
            loopToRepeat[numOfElements] = instrArray[i+1+numOfElements];
            numOfElements++;
            if (numOfElements >= 50){
              break;
            }
          }




          bool result = true;
          
          while (result){
            
            MovingSequence(loopToRepeat, numOfElements, offsetCountForArray+1);

            if (isDigit(insideBracketsValue.charAt(0))){
              leftOperand = insideBracketsValue.substring(0, operatorIndex).toInt();
            }
            else {
              for (int i = 0 ; i < varCurrentIndex ; i++){
                if (varNameArray[i] == insideBracketsValue.substring(0, operatorIndex)){
                  leftOperand = varValueArray[i];
                }
              }
            }
            //right side var
            if (isDigit(insideBracketsValue.charAt(operatorIndex + 1))){
              rightOperand = insideBracketsValue.substring(operatorIndex + 1).toInt();
            }
            else {
              for (int i = 0 ; i < varCurrentIndex ; i++){
                if (varNameArray[i] == insideBracketsValue.substring(operatorIndex + 1)){
                  rightOperand = varValueArray[i];
                }
              }
            }

            

            if (operatorSymbol == '>') {
              result = leftOperand > rightOperand;
            } else if (operatorSymbol == '<') {
              result = leftOperand < rightOperand;
            } else if (operatorSymbol == '=') {
              result = leftOperand == rightOperand;
            } else{
              result = false;
            }
          }
          break;}
      case '$':{ // If, Elif, Else
          int leftOperand, rightOperand;
          char operatorSymbol;
          
          int operatorIndex = -1;
          if (insideBracketsValue.indexOf('>') != -1) {
            operatorIndex = insideBracketsValue.indexOf('>');
            operatorSymbol = '>';
          } else if (insideBracketsValue.indexOf('<') != -1) {
            operatorIndex = insideBracketsValue.indexOf('<');
            operatorSymbol = '<';
          } else if (insideBracketsValue.indexOf('=') != -1) {
            operatorIndex = insideBracketsValue.indexOf('=');
            operatorSymbol = '=';
          }

          //left side var
          if (isDigit(insideBracketsValue.charAt(0))){
            leftOperand = insideBracketsValue.substring(0, operatorIndex).toInt();
          }
          else {
            for (int i = 0 ; i < varCurrentIndex ; i++){
              if (varNameArray[i] == insideBracketsValue.substring(0, operatorIndex)){
                if (insideBracketsValue.substring(0, operatorIndex) == "distF"){
                  callDistFB("distF");
                }else if (insideBracketsValue.substring(0, operatorIndex) == "distB"){
                  callDistFB("distB");
                }else if (insideBracketsValue.substring(0, operatorIndex) == "mpuFB"){

                }else if (insideBracketsValue.substring(0, operatorIndex) == "mpuLR"){

                }else{
                  leftOperand = varValueArray[i];
                }
                
                
              }
            }
          }
          //right side var
          if (isDigit(insideBracketsValue.charAt(operatorIndex + 1))){
            rightOperand = insideBracketsValue.substring(operatorIndex + 1).toInt();
          }
          else {
            for (int i = 0 ; i < varCurrentIndex ; i++){
              if (varNameArray[i] == insideBracketsValue.substring(operatorIndex + 1)){
                rightOperand = varValueArray[i];
              }
            }
          }
            bool result = false;
            if (operatorSymbol == '>') {
              result = leftOperand > rightOperand;
            } else if (operatorSymbol == '<') {
              result = leftOperand < rightOperand;
            } else if (operatorSymbol == '=') {
              result = leftOperand == rightOperand;
            } else {
              result = false;
            }
          String loopToRepeat[50];
          int numOfElements = 0;
          //for (int j = 0; j < )
          while (offsetCountForArray+1 <= ((instrArray[i+1+numOfElements]).substring(0, 1)).toInt()) {
            loopToRepeat[numOfElements] = instrArray[i+1+numOfElements];
            numOfElements++;
            if (numOfElements >= 50){
              break;
            }
          }
              
          if (currentInstruction.indexOf("$IF") != -1) {
            Serial.println("IFSTAT");
            canBeTrue = true;
            if (result){
              canBeTrue = false;
              MovingSequence(loopToRepeat, numOfElements, offsetCountForArray+1);
            }

          } else if (currentInstruction.indexOf("$ELIF") != -1) {
            Serial.println("ELIFSTAT");
            if (!canBeTrue){
              break;
            }
            if (result){
              canBeTrue = false;
              MovingSequence(loopToRepeat, numOfElements, offsetCountForArray+1);
            }
          } else{ // ELSE
            Serial.println("ELSESTAT");
            if (!canBeTrue){
              break;
            } else {
              MovingSequence(loopToRepeat, numOfElements, offsetCountForArray+1);
            }
          }
          break;}
      default:{ // Basic Instruction
          Serial.println(currentInstruction);
          int instructionIndividual[14] = {0}; //later 13
          int numCount = 0;
          while(true)//wholeData.length() > 0
          {
            int indexIndiv = currentInstruction.indexOf(" ");

            if (indexIndiv == -1) {
              instructionIndividual[numCount++] = currentInstruction.toInt();
              break;
            }
            else {
              instructionIndividual[numCount++] = (currentInstruction.substring(0,indexIndiv)).toInt();
              currentInstruction = currentInstruction.substring(indexIndiv+1);
            }
          }
          //hybanie serv
          while (numOfCorrectPos < 12){
            for (int j = 1; j < 13; j++) {
              if (lastPosArray[j-1] == instructionIndividual[j] && lastPosArrayMoved[j-1]==0){
                lastPosArrayMoved[j-1]=1;
                numOfCorrectPos++;
              } else if (lastPosArray[j-1] == instructionIndividual[j] && lastPosArrayMoved[j]==1){
                continue;
              }
              if (lastPosArray[j-1] > instructionIndividual[j]){
                lastPosArray[j-1]--;
              }
              if (lastPosArray[j-1] < instructionIndividual[j]){
                lastPosArray[j-1]++;
              }
              //pwm.setPWM(j-1, 0, angleToPulse(instructionIndividual[j]));
              pwm.setPWM(j-1, 0, angleToPulse(lastPosArray[j-1]));
              
            }
            delay(delayBetweenEachStep);
            }
            for (int g = 0; g < 12; g++){
              lastPosArrayMoved[g]=0;
            }
            //delay(instructionIndividual[13]);
            numOfCorrectPos=0;
            break;
          }
          
    }
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(32, OUTPUT);
  pinMode(33, INPUT);
  pinMode(25, OUTPUT);
  pinMode(26, INPUT);
  const char* ssid;
  const char* password;

  if (0 == 1) { //lan
    ssid = "UPC4138307";
    password = "pa6r8frrcRzc";

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");

      if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;) {}
      }

      display.clearDisplay();
      if (dots.length() >= 20){
        dots = "";
      }
      dots = dots + ". ";
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 20);
      display.println("Connecting");


      display.setTextSize(1);
      display.setCursor(0, 40);
      display.println(dots);
      display.drawLine(0, 51, 128, 51, WHITE);

      display.display();
    }
    myIP = WiFi.localIP();
    wifiServer.begin();
    // 
  }
  else{ //own network
    ssid = "QuadrupedRobot";
    password = "admin371";

    if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
    }
    myIP = WiFi.softAPIP();

    
    networkServer.begin();
  }
  //pesnicka
  for (int thisNote = 0; melody[thisNote]!=-1; thisNote++) {

    int noteDuration = musicSpeed*noteDurations[thisNote];
    tone(19, melody[thisNote],noteDuration*.95);
    Serial.println(melody[thisNote]);

    delay(noteDuration);

    noTone(5);
  }
  Wire.begin(SDA_PIN, SCL_PIN);

  mpuSensor.setWire(&Wire);
  mpuSensor.beginAccel();
  
  pwm.begin();
  pwm.setPWMFreq(60);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {}
    }

  PrintIP("N/C HIP:",myIP.toString());

  for (int j = 0; j < 12; j++) {
            pwm.setPWM(j, 0, angleToPulse(lastPosArray[j]));
        }
  delay(1000);
  
}
void loop() {
  // WiFiClient client;

  if (wifiServer) {
    client = wifiServer.available();
  }
  else if (networkServer) {
    client = networkServer.available();
  } 
  
  if (client) {
    for (int i = 0; i < 7; i++) {
      varNameArray[i] = "";  // Clear each element in the array
      varValueArray[i] = 0;  // Optionally reset the corresponding value array
    }
    varCurrentIndex = 0;  // Reset the index
    // delay(100);
    // callAccel("mpuFB");
    // callDistFB("distF");
    // callDistFB("distB");
    // delay(100);
    PrintIP("Connected","CIP: " +client.remoteIP().toString()); //display
    Serial.println("Client connected"+ client.remoteIP().toString());

    while (client.connected()) { 
      if (client.available()) {
        String data = client.readStringUntil('\n');
        Serial.println("Received data:" + data);
        if (data.indexOf("A") == 0){
          int lastAIndex = 0;
          for (int i = data.length() - 1; i >= 0; i--) {
            if (data[i] == 'A') {
              lastAIndex = i;
              break;
            }
          }
          String aSeqString = data.substring(lastAIndex+1);

          const int MAX_VALUES = 2;
          String items[MAX_VALUES];
          int startIndex = 0;

          for(int i = 0 ; i < MAX_VALUES ; i++){
            int index = aSeqString.indexOf(" ");
            items[i] = aSeqString.substring(startIndex, index);
            aSeqString = aSeqString.substring(index+1);
          }
          //pwm.setPWM(items[0].toInt(), 0, angleToPulse(items[1].toInt()));
          
          while (lastPosArray[items[0].toInt()] != items[1].toInt()){
            if (lastPosArray[items[0].toInt()] > items[1].toInt()){
              lastPosArray[items[0].toInt()]--;
            }
            if (lastPosArray[items[0].toInt()] < items[1].toInt()){
              lastPosArray[items[0].toInt()]++;
            }
            //pwm.setPWM(j-1, 0, angleToPulse(instructionIndividual[j]));
            pwm.setPWM(items[0].toInt(), 0, angleToPulse(lastPosArray[items[0].toInt()]));

            delay(delayBetweenEachStep);
          }
        }
        else if (data.indexOf("B") == 0){
          int lastBIndex = 0;

          for (int i = data.length() - 1; i >= 0; i--) {
            if (data[i] == 'B') {
              lastBIndex = i;
              break; // Exit after finding the last "B" (last b-sequence)
            }
          }

          String bSeqString = data.substring(lastBIndex+1);

          const int MAX_VALUES = 4;
          String items[MAX_VALUES];
          int startIndex = 0;

          for(int i = 0 ; i < MAX_VALUES ; i++){
            int index = bSeqString.indexOf(" ");
            items[i] = bSeqString.substring(startIndex, index);
            bSeqString = bSeqString.substring(index+1);
          }
          // pwm.setPWM(items[0].toInt(), 0, angleToPulse(items[1].toInt()) );
          // pwm.setPWM(items[2].toInt(), 0, angleToPulse(items[3].toInt()) );
          
          while (numOfCorrectPos < 2){
            for (int i = 0; i < 3; i+=2){
              if (lastPosArray[items[i].toInt()] == items[i+1].toInt() && lastPosArrayMoved[items[i].toInt()]==0){
                lastPosArrayMoved[items[i].toInt()]=1;
                numOfCorrectPos++;
              } else if (lastPosArray[items[i].toInt()] == items[i+1].toInt() && lastPosArrayMoved[items[i].toInt()]==1){
                continue;
              }
              if (lastPosArray[items[i].toInt()] > items[i+1].toInt()){
                lastPosArray[items[i].toInt()]--;
              }
              if (lastPosArray[items[i].toInt()] < items[i+1].toInt()){
                lastPosArray[items[i].toInt()]++;
              }
              //pwm.setPWM(j-1, 0, angleToPulse(instructionIndividual[j]));
              pwm.setPWM(items[i].toInt(), 0, angleToPulse(lastPosArray[items[i].toInt()]));

              
            }
            delay(delayBetweenEachStep);
          }
            for (int g = 0; g < 12; g++){
              lastPosArrayMoved[g]=0;
            }
            //delay(instructionIndividual[13]);
            numOfCorrectPos=0;
        }
        else if (data.indexOf("C") == 0){
          int lastCIndex = 0;
          for (int i = data.length() - 1; i >= 0; i--) {
            if (data[i] == 'C') {
              lastCIndex = i;
              break; // Exit after finding the first "B" (last c-sequence)
            }
          }

          String cSeqString = data.substring(lastCIndex+1);
          
          const int MAX_VALUES = 8;
          String items[MAX_VALUES];
          int startIndex = 0;

          for(int i = 0 ; i < MAX_VALUES ; i++)
          {
            int index = cSeqString.indexOf(" ");
            items[i] = cSeqString.substring(startIndex, index);
            cSeqString = cSeqString.substring(index+1);
          }
          // pwm.setPWM(items[0].toInt(), 0, angleToPulse(items[1].toInt()) );
          // pwm.setPWM(items[2].toInt(), 0, angleToPulse(items[3].toInt()) );
          // pwm.setPWM(items[4].toInt(), 0, angleToPulse(items[5].toInt()) );
          // pwm.setPWM(items[6].toInt(), 0, angleToPulse(items[7].toInt()) );
          
          while (numOfCorrectPos < 4){
            for (int i = 0; i < 7; i+=2){
              if (lastPosArray[items[i].toInt()] == items[i+1].toInt() && lastPosArrayMoved[items[i].toInt()]==0){
                lastPosArrayMoved[items[i].toInt()]=1;
                numOfCorrectPos++;
              } else if (lastPosArray[items[i].toInt()] == items[i+1].toInt() && lastPosArrayMoved[items[i].toInt()]==1){
                continue;
              }
              if (lastPosArray[items[i].toInt()] > items[i+1].toInt()){
                lastPosArray[items[i].toInt()]--;
              }
              if (lastPosArray[items[i].toInt()] < items[i+1].toInt()){
                lastPosArray[items[i].toInt()]++;
              }
              //pwm.setPWM(j-1, 0, angleToPulse(instructionIndividual[j]));
              pwm.setPWM(items[i].toInt(), 0, angleToPulse(lastPosArray[items[i].toInt()]));

              
            }
            delay(delayBetweenEachStep);
          }
            for (int g = 0; g < 12; g++){
              lastPosArrayMoved[g]=0;
            }
            //delay(instructionIndividual[13]);
            numOfCorrectPos=0;
        }
        else if (data.indexOf("D") == 0){
          int lastDIndex = 0;
          for (int i = data.length() - 1; i >= 0; i--) {
            if (data[i] == 'D') {
              lastDIndex = i;
              break; // Exit after finding the first "B" (last c-sequence)
            }
          }

          String dSeqString = data.substring(lastDIndex+1);
          
          const int MAX_VALUES = 16;
          String items[MAX_VALUES];
          int startIndex = 0;

          for(int i = 0 ; i < MAX_VALUES ; i++)
          {
            int index = dSeqString.indexOf(" ");
            items[i] = dSeqString.substring(startIndex, index);
            dSeqString = dSeqString.substring(index+1);
          }
          // pwm.setPWM(items[0].toInt(), 0, angleToPulse(items[1].toInt()) );
          // pwm.setPWM(items[2].toInt(), 0, angleToPulse(items[3].toInt()) );
          // pwm.setPWM(items[4].toInt(), 0, angleToPulse(items[5].toInt()) );
          // pwm.setPWM(items[6].toInt(), 0, angleToPulse(items[7].toInt()) );
          
          // pwm.setPWM(items[8].toInt(), 0, angleToPulse(items[9].toInt()) );
          // pwm.setPWM(items[10].toInt(), 0, angleToPulse(items[11].toInt()) );
          // pwm.setPWM(items[12].toInt(), 0, angleToPulse(items[13].toInt()) );
          // pwm.setPWM(items[14].toInt(), 0, angleToPulse(items[15].toInt()) );
          while (numOfCorrectPos < 8){
            for (int i = 0; i < 15; i+=2){
              if (lastPosArray[items[i].toInt()] == items[i+1].toInt() && lastPosArrayMoved[items[i].toInt()]==0){
                lastPosArrayMoved[items[i].toInt()]=1;
                numOfCorrectPos++;
              } else if (lastPosArray[items[i].toInt()] == items[i+1].toInt() && lastPosArrayMoved[items[i].toInt()]==1){
                continue;
              }
              if (lastPosArray[items[i].toInt()] > items[i+1].toInt()){
                lastPosArray[items[i].toInt()]--;
              }
              if (lastPosArray[items[i].toInt()] < items[i+1].toInt()){
                lastPosArray[items[i].toInt()]++;
              }
              //pwm.setPWM(j-1, 0, angleToPulse(instructionIndividual[j]));
              pwm.setPWM(items[i].toInt(), 0, angleToPulse(lastPosArray[items[i].toInt()]));

              
            }
            delay(delayBetweenEachStep);
          }
            for (int g = 0; g < 12; g++){
              lastPosArrayMoved[g]=0;
            }
            //delay(instructionIndividual[13]);
            numOfCorrectPos=0;
        }
        else if (data.indexOf("*") == 0){
          String wholeData = String(data);
          wholeData.remove(0,1);
          String instructionsArray[200];
          int instructionCount = 0;
          //rozdelovanie instrukcii do jednotlivych
          while(wholeData.length() > 0) {
            wholeData.remove(0,1);
            int index = wholeData.indexOf("#");

            if (index == -1){
              instructionsArray[instructionCount++] = wholeData;
              break;
            }
            else{
              instructionsArray[instructionCount++] = wholeData.substring(0,index);
              wholeData = wholeData.substring(index);
            }
          }
          //int arrayLength = sizeof(instructionsArray) / sizeof(instructionsArray[0]);
          MovingSequence(instructionsArray,instructionCount, 0);
        }
        else
        {
          Serial.println("it doesnt contain any of those characters above");
        }
        //Serial.println(callAccel());
        //Serial.println(callDistFB("distF"));
        
      }
    } 
    client.stop();
    PrintIP("NCIP: ", myIP.toString());
  }
} 
