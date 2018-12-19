#include <SoftwareSerial.h>
#include <Ultrasonic.h>

#include "lsem.h"



//-- SOME NEEDED PROTOTYPES ---------------------------


//------------------------------------------------
//--- GLOBAL VARIABLES ---------------------------
//------------------------------------------------
char    GLBserialInputString[100]; // a string to hold incoming data
char    GLBauxString[100];
int     GLBserialIx=0;
bool    GLBserialInputStringReady = false; // whether the string is complete


//Ultrasonic ultrasonic(12, 11,20000UL);
Ultrasonic ultrasonic(12, 11);



//------------------------------------------------
//-------    TIMER CALLBACKS PROTOTYPES  ---------
//------------------------------------------------
void GLBcallbackTimeoutRight(void);
void GLBcallbackTimeoutLeft(void);
void GLBcallbackPauseRight(void);
void GLBcallbackPauseLeft(void);

//------------------------------------------------
//-------    GLOBAL VARS TO THIS MODULE  ---------
//------------------------------------------------

#define NUM_LEDS  15
#define DATA_PIN_LS_RIGHT 9
#define DATA_PIN_LS_LEFT 10
CRGB RightLS[NUM_LEDS];
CRGB LeftLS[NUM_LEDS];

LSEM r(RightLS, NUM_LEDS, GLBcallbackPauseRight, GLBcallbackTimeoutRight);
LSEM l(LeftLS,  NUM_LEDS, GLBcallbackPauseLeft,  GLBcallbackTimeoutLeft);

int GLBhelloIndex=0;
#define HELLO_MESSAGES 5
const char boot1[] = ":LP0030:LT0015:LMA:LCff,66,ff";
const char boot2[] = ":LP0030:LT0005:LMT:LC00,ff,55";
const char boot3[] = ":LP0030:LT0005:LMt:LC4d,a6,ff";
const char boot4[] = ":LP0030:LT0030:LMK:LCFF,a6,ff";
const char boot5[] = ":LP0030:LT0020:LMk:LCFF,a6,ff";

const char *bootx[] = {  boot1, boot2, boot3, boot4, boot5 };


const char boot[] = ":LP0080:LT0050:LMA:LCff,66,ff";



//------------------------------------------------
//--- GLOBAL FUNCTIONS ---------------------------
//------------------------------------------------
void playNoiseColor(int distance);
void playNoise(int distance);
void playNoisePink(int distance);
void playRolling(int distance);
void playKnightRider(int distance);

//------------------------------------------------
//-------    TIMER CALLBACKS     -----------------
//------------------------------------------------
void GLBcallbackTimeoutRight(void)
{
  //if (LSEM.getDebug()) {Serial.println(F("DEBUG: callbackTimeout"));}
  r.callbackTimeout();
}
void GLBcallbackTimeoutLeft(void)
{
  //if (LSEM.getDebug()) {Serial.println(F("DEBUG: callbackTimeout"));}
  l.callbackTimeout();
}
//------------------------------------------------
void GLBcallbackPauseRight(void)
{
  //Serial.println(F("DEBUG: callbackPause");
  r.callbackPause();
}

//------------------------------------------------
void GLBcallbackPauseLeft(void)
{
  //Serial.println(F("DEBUG: callbackPause");
  l.callbackPause();
}
//-------------------------------------------------------------

#define MAX_FUN_MODES 5
typedef void (*cbf)(int);  
cbf GLBplaySetptr[MAX_FUN_MODES] = {
  &playNoiseColor,&playRolling,&playNoisePink,&playNoise,&playKnightRider};    
uint8_t GLBplaySetptr_index=MAX_FUN_MODES-1;
//------------------------------------------------

void readUltrasonic(void)
{
 int distance = ultrasonic.read();

 Serial.print(F("Distance CM:"));
 Serial.println(distance);

 if (distance==0) return;
 if (distance>30) return;


 if ( r.isIdle() && l.isIdle() ) {  
    //Try new set
    GLBplaySetptr_index=(GLBplaySetptr_index+1)%MAX_FUN_MODES;
 }
 // else {      //keep in the set  }
 GLBplaySetptr[GLBplaySetptr_index](distance);
}



//------------------------------------------------

void playNoiseColor(int distance)
{
   char aux[50];
   char aux2[30];

   //Serial.println(F("NOISE COLOR SET"));
   if (distance > 15) return;


   int delta=(distance%10)*5;

   int filter=(distance)*3; // More distance, more filter

   int flick=25+distance*4;  // More distance, less flicker

   sprintf(aux2,":LT0050:LMn:LG%04d:LP%04d",filter,flick);

   //Color by slots
   if (distance < 10) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0xFF-delta,0,0);
   }
   else if (distance < 15) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0xFF-delta,0xFF-delta,0);
   }
   else if (distance < 20) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0,0,0xFF-delta);
   }
   else if (distance < 25) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0,0xFF-delta,0xFF-delta);
   }
   else if (distance < 30) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0xFF-delta,0xFF-delta,0xFF-delta);
   }
   r.processCommands(aux);
   l.processCommands(aux);
 

}

//----------------------------------------------------------
void playRolling(int distance)
{
   char aux[50];
   char aux2[30];

   //Serial.println(F("ROLLING SET"));


   int delta=(distance%10)*5;


   int flick=20+distance*3;  // More distance, less flicker

   sprintf(aux2,":LT0050:LMC:LP%04d",flick);

   //Color by slots
   if (distance < 10) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0xAA-delta,0,0);
   }
   else if (distance < 15) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0xBB-delta,0xCC-delta,0x22);
   }
   else if (distance < 20) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0,0xFF-delta,0);
   }
   else if (distance < 25) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0,0x66-delta,0x55-delta);
   }
   else if (distance < 30) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0x00-delta,0x10-delta,0xaa-delta);
   }
   r.processCommands(aux);
   l.processCommands(aux);
 

}

//----------------------------------------------------------
void playKnightRider(int distance)
{
   char aux[50];
   char aux2[30];

   //Serial.println(F("Knight rider SET"));


   int delta=(distance%10)*7;


   int flick=20+distance*3;  // More distance, less flicker

   sprintf(aux2,":LT0050:LMK:LP%04d",flick);

   //Color by slots
   if (distance < 10) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0,0,0xFF-delta);
   }
   else if (distance < 15) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,delta,0xFF-delta,0xFF);
   }
   else if (distance < 20) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0,0xFF-delta,0);
   }
   else if (distance < 25) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0xff,delta,delta);
   }
   else if (distance < 30) { 
     sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,0xff-delta,0xff-delta,0xff-delta);
   }
   r.processCommands(aux);
   l.processCommands(aux);
 

}


//------------------------------------------------

void playNoise(int distance)
{
   char aux[50];

   // Serial.println(F("NOISE SET"));

   int flick=100+distance*4;  // More distance, less flicker
   
   if (r.isIdle())  {
     sprintf(aux,":LT0050:LMN:LP%04d",flick);
     r.processCommands(aux);
     l.processCommands(aux);
   }
} 

//------------------------------------------------

void playNoisePink(int distance)
{
   char aux[50];
   // Serial.println(F("PINK NOISE SET"));

   int filter=(distance)*3; // More distance, more filter
   int flick=200+distance*3;  // More distance, less flicker

   sprintf(aux,":LT0050:LMn:LG%04d:LP%04d:LC9F,17,64",filter,flick);


   r.processCommands(aux);
   l.processCommands(aux);
} 



//----------------------------------------------------------
void playPing(int rr, int gg, int bb)
{
   char aux[50];
   char aux2[30];

   sprintf(aux2,":LT0030:LMK:LP0100");
   sprintf(aux,"%s:LC%02X,%02X,%02X",aux2,rr,gg,bb);
   r.processCommands(aux);
   l.processCommands(aux);
 

}


//------------------------------------------------

void setup() { 

  // Serial to debug AND comunication protocolo with PI              
  Serial.begin(9600);
  Serial.println(F("Setup... 'came on, be my baby, came on'"));
  GLBserialInputString[0]=0;

  FastLED.addLeds<WS2812B,DATA_PIN_LS_RIGHT,GRB>(RightLS, NUM_LEDS);
  FastLED.addLeds<WS2812B,DATA_PIN_LS_LEFT,GRB>(LeftLS, NUM_LEDS);


  Serial.println("Messages:");
  for (int i=0;i<HELLO_MESSAGES;i++)
  {
   Serial.println(bootx[i]);
  } 

}

//------------------------------------------------
void serialEvent() {
  while (Serial.available()) {
     char inChar = (char)Serial.read();
     if (inChar < 0x20) {
       GLBserialInputStringReady = true;
       GLBserialInputString[GLBserialIx]=0;
       GLBserialIx=0;
       return;
     }
     GLBserialInputString[GLBserialIx++]=inChar;
  }
}

//-------------------------------------------------
void processSerialInputString()
{
  strcpy(GLBauxString,GLBserialInputString);
  GLBserialInputString[0]=0;
  GLBserialInputStringReady = false;
  r.processCommands(GLBauxString);
  l.processCommands(GLBauxString);
}






//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------

unsigned long interval=500;     // the time we need to wait
unsigned long previousMillis=0;  // millis() returns an unsigned long.

unsigned long interval2=15000;     // the time we need to wait
unsigned long previousMillis2=0;  // millis() returns an unsigned long.


void loop() { 
  unsigned long currentMillis = millis(); // grab current time

  // State machine as main controller execution
  if (GLBhelloIndex < HELLO_MESSAGES)
  {
    if (r.isIdle() && l.isIdle())
    {
      Serial.print(F("Halo:"));
      Serial.println(GLBhelloIndex);
      Serial.println(bootx[GLBhelloIndex]); 
      r.processCommands((char*)bootx[GLBhelloIndex]);
      l.processCommands((char*)bootx[GLBhelloIndex]);
      GLBhelloIndex++;
    }
  }
  else
  {
    if ((unsigned long)(currentMillis - previousMillis) >= interval) {
      readUltrasonic();
      previousMillis = millis();
    }
    if (r.isIdle()) {
      if ((unsigned long)(currentMillis - previousMillis2) >= interval2) {
        playPing(random(0,255),random(0,255),random(0,255));
        previousMillis2 = millis();
      }
    }



    processSerialInputString();
  }

  r.refresh();
  l.refresh();
  FastLED.show();
}

