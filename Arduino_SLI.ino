#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
Adafruit_AlphaNum4 dispL = Adafruit_AlphaNum4();
Adafruit_AlphaNum4 dispR = Adafruit_AlphaNum4();

int gearDpin = 24;
int gearCpin = 22;
int gearLpin = 23;
int SER_Pin = 27; //pin 14 on 75HC595
int RCLK_Pin = 25; //pin 12 on 75HC595
int SRCLK_Pin = 26; //pin 11 on 75HC595

//How many of the shift registers - change this
#define number_of_74hc595s 3 

//do not touch
#define numOfRegisterPins number_of_74hc595s * 8

boolean registers[numOfRegisterPins];
char rpmHolder;

//rpm flash variables
int ledState = LOW;
long previousMillis = 0;
long interval = 500;
boolean limit = false;

//warning lights
int yelL = 16;
int yelR = 17;
int bluL = 18;
int bluR = 19;
int redL = 20;
int redR = 21;

int shiftBrightnessPin = 3;

int shiftBright00 = 239;
int shiftBright01 = 223;
int shiftBright02 = 207;
int shiftBright03 = 191;
int shiftBright04 = 175;
int shiftBright05 = 159;
int shiftBright06 = 143;
int shiftBright07 = 127;
int shiftBright08 = 111;
int shiftBright09 = 95;
int shiftBright10 = 79;
int shiftBright11 = 63;
int shiftBright12 = 47;
int shiftBright13 = 31;
int shiftBright14 = 15;
int shiftBright15 = 0;

byte dash = 0b00000010;
byte gearR = 0b00001010;
byte gearN = 0b11111100;
byte gear1 = 0b01100000;
byte gear2 = 0b11011010;
byte gear3 = 0b11110010;
byte gear4 = 0b01100110;
byte gear5 = 0b10110110;
byte gear6 = 0b10111110;
byte gear7 = 0b11100000;
byte gear8 = 0b11111110;

long revs = 0x879;
long gear = 0x897; //random value
long disR = 0x987; //random value
long disL = 0x789; //random value
long blnk = 0x0;
long decp = 0x4000;
long num0 = 0x3F;
long num1 = 0x6;
long num2 = 0xDB;
long num3 = 0xCF;
long num4 = 0xE6;
long num5 = 0xED;
long num6 = 0xFD;
long num7 = 0x7;
long num8 = 0xFF;
long num9 = 0xE7;
long plus = 0x12C0;
long mnus = 0xC0;

int MAX_CMD_LENGTH = 10;
char cmd[10];
int cmdIndex;
char incomingByte;
long digitHolder[10];
long LEDMapHolder[4];

void setup() {
     
    //Setup Serial Port with baud rate of 9600
    Serial.begin(9600);
    dispL.begin(0x70);
    dispR.begin(0x71);

    pinMode(gearDpin, OUTPUT);
    pinMode(gearCpin, OUTPUT);
    pinMode(gearLpin, OUTPUT);
    pinMode(SER_Pin, OUTPUT);
    pinMode(RCLK_Pin, OUTPUT);
    pinMode(SRCLK_Pin, OUTPUT);
    pinMode(shiftBrightnessPin, OUTPUT);
    
    analogWrite(shiftBrightnessPin, shiftBright07);
    dispL.setBrightness(0);
    dispR.setBrightness(0);
    
    dispL.writeDigitAscii(0, '<');
    dispL.writeDigitAscii(1, 'D');
    dispL.writeDigitAscii(2, 'I');
    dispL.writeDigitAscii(3, 'Y');
    dispR.writeDigitAscii(0, 'S');
    dispR.writeDigitAscii(1, 'L');
    dispR.writeDigitAscii(2, 'I');
    dispR.writeDigitAscii(3, '>');
    dispR.writeDisplay();
    dispL.writeDisplay();
    updateGearShiftRegister(dash);
    
    clearRegisters();
    writeRegisters();
    
    setRegisterPin(1, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(7, HIGH);
    setRegisterPin(9, HIGH);
    setRegisterPin(11, HIGH);
    setRegisterPin(13, HIGH);
    
    setRegisterPin(yelL, HIGH);
    setRegisterPin(yelR, HIGH);
    setRegisterPin(bluL, HIGH);
    setRegisterPin(bluR, HIGH);
    setRegisterPin(redL, HIGH);
    setRegisterPin(redR, HIGH);
    
    writeRegisters();



    
    delay(250);
    
    cmdIndex = 0;
    
}
 
void loop() {
  if (limit == true)
  {
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis > interval)
    {
      previousMillis = currentMillis;
      
      if (ledState == LOW)
      {
        ledState = HIGH;
      }
      else
      {
        ledState = LOW;
      }
      writeAllRpm(ledState);
    }
  }
    
    if (incomingByte=Serial.available()>0) {
      
      char byteIn = Serial.read();
      cmd[cmdIndex] = byteIn;
      
      if(byteIn=='\n')
      {
        
        //command finished
        cmd[cmdIndex] = '\0';
        Serial.println(cmd);
        cmdIndex = 0;
        LEDMapHolder[0] = assignChar(cmd[0]);
        rpmHolder = cmd[1];
        LEDMapHolder[1] = assignChar(cmd[1]);
        LEDMapHolder[2] = assignChar(cmd[2]);
        LEDMapHolder[3] = assignChar(cmd[3]);
        LEDMapHolder[4] = assignChar(cmd[4]);
        LEDMapHolder[5] = assignChar(cmd[5]);
        LEDMapHolder[6] = assignChar(cmd[6]);
        LEDMapHolder[7] = assignChar(cmd[7]);
        LEDMapHolder[8] = assignChar(cmd[8]);
        LEDMapHolder[9] = assignChar(cmd[9]);
        
        determineDecimalPoints();
        
        long digit1 = LEDMapHolder[1];
        long digit2 = LEDMapHolder[2];;
        long digit3 = LEDMapHolder[3];;
        long digit4 = LEDMapHolder[4];;
        
        if (LEDMapHolder[0] == disR)
        {
          displayRight(digit1, digit2, digit3, digit4);
        }
        else if (LEDMapHolder[0] == disL)
        {
          displayLeft(digit1, digit2, digit3, digit4);
        }
        else if (LEDMapHolder[0] == gear)
        {
          
          byte gearHolder = longToGear(digit1);
          updateGearShiftRegister(gearHolder);
        }
        else if (LEDMapHolder[0] == revs)
        {
          updateRpmRegister(rpmHolder);
        }
      }
      
      else
      {
        if(cmdIndex++ >= MAX_CMD_LENGTH)
        {
          cmdIndex = 0;
        }
      }
    }
    
}

void updateGearShiftRegister(byte gearInput)
{
   digitalWrite(gearLpin, LOW);
   shiftOut(gearDpin, gearCpin, LSBFIRST, gearInput);
   digitalWrite(gearLpin, HIGH);
}

long assignChar(char input)
{
  long output;
  switch (input)
  {
    case 'R':
    output = disR;
    break;
    case 'L':
    output = disL;
    break;
    case 'G':
    output = gear;
    break;
    case 'E':
    output = revs;
    break;
    case '0':
    output = num0;
    break;
    case '1':
    output = num1;
    break;
    case '2':
    output = num2;
    break;
    case '3':
    output = num3;
    break;
    case '4':
    output = num4;
    break;
    case '5':
    output = num5;
    break;
    case '6':
    output = num6;
    break;
    case '7':
    output = num7;
    break;
    case '8':
    output = num8;
    break;
    case '9':
    output = num9;
    break;
    case '+':
    output = plus;
    break;
    case '-':
    output = mnus;
    break;
    case '.':
    output = decp;
    break;
    case ' ':
    output = blnk;
    break;
  }
  return output;
}

void displayRight(long I, long II, long III, long IIII)
{
  dispR.writeDigitRaw(0, I );
  dispR.writeDigitRaw(1, II);
  dispR.writeDigitRaw(2, III);
  dispR.writeDigitRaw(3, IIII);
  dispR.writeDisplay();
}

void displayLeft(long I, long II, long III, long IIII)
{
  dispL.writeDigitRaw(0, I );
  dispL.writeDigitRaw(1, II);
  dispL.writeDigitRaw(2, III);
  dispL.writeDigitRaw(3, IIII);
  dispL.writeDisplay();
}

byte longToGear(long input)
{
  byte output = 0b0;
  if (input == num1){output = gear1;}
  else if (input == num2){output = gear2;}
  else if (input == num3){output = gear3;}
  else if (input == num4){output = gear4;}
  else if (input == num5){output = gear5;}
  else if (input == num6){output = gear6;}
  else if (input == num7){output = gear7;}
  else if (input == num8){output = gear8;}
  else if (input == num0){output = gearN;}
  else if (input == num9){output = gearR;}
  else {output = 0b10011110;}
  return output;
}

void determineDecimalPoints()
{
  if (LEDMapHolder[2] == decp)
  {
    LEDMapHolder[1] = LEDMapHolder[1] + LEDMapHolder[2];
    //shift
    LEDMapHolder[2] = LEDMapHolder[3];
    LEDMapHolder[3] = LEDMapHolder[4];
    LEDMapHolder[4] = LEDMapHolder[5];
    LEDMapHolder[5] = LEDMapHolder[6];
    LEDMapHolder[6] = LEDMapHolder[7];
    LEDMapHolder[7] = LEDMapHolder[8];
    LEDMapHolder[8] = LEDMapHolder[9];
  }
  if (LEDMapHolder[3] == decp)
  {
    LEDMapHolder[2] = LEDMapHolder[2] + LEDMapHolder[3];
    //shift
    LEDMapHolder[3] = LEDMapHolder[4];
    LEDMapHolder[4] = LEDMapHolder[5];
    LEDMapHolder[5] = LEDMapHolder[6];
    LEDMapHolder[6] = LEDMapHolder[7];
    LEDMapHolder[7] = LEDMapHolder[8];
    LEDMapHolder[8] = LEDMapHolder[9];
  }
  if (LEDMapHolder[4] == decp)
  {
    LEDMapHolder[3] = LEDMapHolder[3] + LEDMapHolder[4];
    //shift
    LEDMapHolder[4] = LEDMapHolder[5];
    LEDMapHolder[5] = LEDMapHolder[6];
    LEDMapHolder[6] = LEDMapHolder[7];
    LEDMapHolder[7] = LEDMapHolder[8];
    LEDMapHolder[8] = LEDMapHolder[9];
  }
  if (LEDMapHolder[5] == decp)
  {
    LEDMapHolder[4] = LEDMapHolder[4] + LEDMapHolder[5];
    //shift
    LEDMapHolder[5] = LEDMapHolder[6];
    LEDMapHolder[6] = LEDMapHolder[7];
    LEDMapHolder[7] = LEDMapHolder[8];
    LEDMapHolder[8] = LEDMapHolder[9];
  }
  if (LEDMapHolder[6] == decp)
  {
    LEDMapHolder[5] = LEDMapHolder[5] + LEDMapHolder[6];
    //shift
    LEDMapHolder[6] = LEDMapHolder[7];
    LEDMapHolder[7] = LEDMapHolder[8];
    LEDMapHolder[8] = LEDMapHolder[9];
  }
  if (LEDMapHolder[7] == decp)
  {
    LEDMapHolder[6] = LEDMapHolder[6] + LEDMapHolder[7];
    //shift
    LEDMapHolder[7] = LEDMapHolder[8];
    LEDMapHolder[8] = LEDMapHolder[9];
  }
  if (LEDMapHolder[8] == decp)
  {
    LEDMapHolder[7] = LEDMapHolder[7] + LEDMapHolder[8];
    //shift
    LEDMapHolder[8] = LEDMapHolder[9];
  }
  if (LEDMapHolder[9] == decp)
  {
    LEDMapHolder[8] = LEDMapHolder[8] + LEDMapHolder[9];
  }
}

//set all register pins to LOW
void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
     registers[i] = LOW;
  }
} 


//Set and display registers
//Only call AFTER all values are set how you would like (slow otherwise)
void writeRegisters(){

  digitalWrite(RCLK_Pin, LOW);

  for(int i = numOfRegisterPins - 1; i >=  0; i--){
    digitalWrite(SRCLK_Pin, LOW);

    int val = registers[i];

    digitalWrite(SER_Pin, val);
    digitalWrite(SRCLK_Pin, HIGH);

  }
  digitalWrite(RCLK_Pin, HIGH);

}

//set an individual pin HIGH or LOW
void setRegisterPin(int index, int value){
  registers[index] = value;
}

void updateRpmRegister(char input)
{
  switch(input)
  {
    case 'a':
    setRegisterPin(0, LOW);
    setRegisterPin(1, LOW);
    setRegisterPin(2, LOW);
    setRegisterPin(3, LOW);
    setRegisterPin(4, LOW);
    setRegisterPin(5, LOW);
    setRegisterPin(6, LOW);
    setRegisterPin(7, LOW);
    setRegisterPin(8, LOW);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'b':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, LOW);
    setRegisterPin(2, LOW);
    setRegisterPin(3, LOW);
    setRegisterPin(4, LOW);
    setRegisterPin(5, LOW);
    setRegisterPin(6, LOW);
    setRegisterPin(7, LOW);
    setRegisterPin(8, LOW);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'c':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, LOW);
    setRegisterPin(3, LOW);
    setRegisterPin(4, LOW);
    setRegisterPin(5, LOW);
    setRegisterPin(6, LOW);
    setRegisterPin(7, LOW);
    setRegisterPin(8, LOW);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'd':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, LOW);
    setRegisterPin(4, LOW);
    setRegisterPin(5, LOW);
    setRegisterPin(6, LOW);
    setRegisterPin(7, LOW);
    setRegisterPin(8, LOW);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'e':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, LOW);
    setRegisterPin(5, LOW);
    setRegisterPin(6, LOW);
    setRegisterPin(7, LOW);
    setRegisterPin(8, LOW);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'f':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, LOW);
    setRegisterPin(6, LOW);
    setRegisterPin(7, LOW);
    setRegisterPin(8, LOW);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'g':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, LOW);
    setRegisterPin(7, LOW);
    setRegisterPin(8, LOW);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'h':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, LOW);
    setRegisterPin(8, LOW);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'i':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, HIGH);
    setRegisterPin(8, LOW);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'j':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, HIGH);
    setRegisterPin(8, HIGH);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'k':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, HIGH);
    setRegisterPin(8, HIGH);
    setRegisterPin(9, HIGH);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'l':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, HIGH);
    setRegisterPin(8, HIGH);
    setRegisterPin(9, HIGH);
    setRegisterPin(10, HIGH);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'm':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, HIGH);
    setRegisterPin(8, HIGH);
    setRegisterPin(9, HIGH);
    setRegisterPin(10, HIGH);
    setRegisterPin(11, HIGH);
    setRegisterPin(12, LOW);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'n':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, HIGH);
    setRegisterPin(8, HIGH);
    setRegisterPin(9, HIGH);
    setRegisterPin(10, HIGH);
    setRegisterPin(11, HIGH);
    setRegisterPin(12, HIGH);
    setRegisterPin(13, LOW);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'o':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, HIGH);
    setRegisterPin(8, HIGH);
    setRegisterPin(9, HIGH);
    setRegisterPin(10, HIGH);
    setRegisterPin(11, HIGH);
    setRegisterPin(12, HIGH);
    setRegisterPin(13, HIGH);
    setRegisterPin(14, LOW);
    limit = false;
    break;
    case 'p':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, HIGH);
    setRegisterPin(8, HIGH);
    setRegisterPin(9, HIGH);
    setRegisterPin(10, HIGH);
    setRegisterPin(11, HIGH);
    setRegisterPin(12, HIGH);
    setRegisterPin(13, HIGH);
    setRegisterPin(14, HIGH);
    limit = false;
    break;
    case 'q':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, HIGH);
    setRegisterPin(3, HIGH);
    setRegisterPin(4, HIGH);
    setRegisterPin(5, HIGH);
    setRegisterPin(6, HIGH);
    setRegisterPin(7, HIGH);
    setRegisterPin(8, HIGH);
    setRegisterPin(9, HIGH);
    setRegisterPin(10, HIGH);
    setRegisterPin(11, HIGH);
    setRegisterPin(12, HIGH);
    setRegisterPin(13, HIGH);
    setRegisterPin(14, HIGH);
    ledState = HIGH;
    limit = true;
    break;
    case 'r':
    setRegisterPin(0, HIGH);
    setRegisterPin(1, HIGH);
    setRegisterPin(2, LOW);
    setRegisterPin(3, LOW);
    setRegisterPin(4, LOW);
    setRegisterPin(5, LOW);
    setRegisterPin(6, LOW);
    setRegisterPin(7, LOW);
    setRegisterPin(8, LOW);
    setRegisterPin(9, LOW);
    setRegisterPin(10, LOW);
    setRegisterPin(11, LOW);
    setRegisterPin(12, LOW);
    setRegisterPin(13, HIGH);
    setRegisterPin(14, HIGH);
    limit = false;
    break;
  }
  writeRegisters();
}

void writeAllRpm(int state)
{
  setRegisterPin(0, state);
  setRegisterPin(1, state);
  setRegisterPin(2, state);
  setRegisterPin(3, state);
  setRegisterPin(4, state);
  setRegisterPin(5, state);
  setRegisterPin(6, state);
  setRegisterPin(7, state);
  setRegisterPin(8, state);
  setRegisterPin(9, state);
  setRegisterPin(10, state);
  setRegisterPin(11, state);
  setRegisterPin(12, state);
  setRegisterPin(13, state);
  setRegisterPin(14, state);
  writeRegisters();
}

