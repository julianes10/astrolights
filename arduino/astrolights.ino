#include <SoftwareSerial.h>
#include <Ultrasonic.h>

#include "lsem.h"



//-- SOME NEEDED PROTOTYPES ---------------------------

void STATE_init(void);
void STATE_welcome(void);
void STATE_idle(void);

// State pointer function
void (*GLBptrStateFunc)();


//------------------------------------------------
//--- GLOBAL VARIABLES ---------------------------
//------------------------------------------------
char    GLBserialInputString[100]; // a string to hold incoming data
char    GLBauxString[100];
int     GLBserialIx=0;
bool    GLBserialInputStringReady = false; // whether the string is complete


Ultrasonic ultrasonic(12, 11);
int latestLight_distance=0;
int distance=0;
SimpleTimer timerUltrasonic;


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
#define HELLO_MESSAGES 1
const char boot1[] = ":LP0080:LT0015:LMA:LCff,66,ff";
const char boot2[] = ":LP0080:LT0005:LMA:LC00,ff,55";
const char boot3[] = ":LP0080:LT0005:LMA:LC4d,a6,ff";
const char boot4[] = ":LP0080:LT0030:LMK:LCFF,a6,ff";
const char boot5[] = ":LP0080:LT0020:LMk:LCFF,a6,ff";

const char *bootx[] = {  boot1, boot2, boot3, boot4, boot5 };


const char boot[] = ":LP0080:LT0050:LMA:LCff,66,ff";



//------------------------------------------------
//--- GLOBAL FUNCTIONS ---------------------------
//------------------------------------------------
void playNoiseColor(int distance);


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

// State pointer function
void (*GLBplaySet)(int distance);


//------------------------------------------------

void cbUltrasonic(void)
{
 distance = ultrasonic.read();
 if (distance==0) return;
 if (distance>30) return;

 Serial.print(F("Distance CM:"));
 Serial.println(distance);


 if (r.isIdle() && l.isIdle())
 {  //Let's focus in one set
   switch(random(2))
   {
     case 0: 
          GLBplaySet=playNoiseColor;
          break;
     default:
          GLBplaySet=playNoiseColor;
          break;    
   };
 }
 GLBplaySet(distance);
}



//------------------------------------------------

void playNoiseColor(int distance)
{
 char aux[50];
 char aux2[30];

 if ((distance>0) && (distance<30))
 {
   int delta=(distance%10)*5;

   int filter=(distance)*3; // More distance, more filter

   int flick=20+distance*3;  // More distance, less flicker

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
   /*Serial.print(F("CHANGE to:"));
     Serial.print(aux);
     latestLight_distance = distance;*/
 }

}


//------------------------------------------------

void setup() { 

  // Serial to debug AND comunication protocolo with PI              
  Serial.begin(9600);
  Serial.println(F("Setup... 'came on, be my baby, came on'"));
  GLBserialInputString[0]=0;

  FastLED.addLeds<WS2812B,DATA_PIN_LS_RIGHT,GRB>(RightLS, NUM_LEDS);
  FastLED.addLeds<WS2812B,DATA_PIN_LS_LEFT,GRB>(LeftLS, NUM_LEDS);

  distance=200;
  timerUltrasonic.setInterval(200,cbUltrasonic);

  Serial.println("Messages:");
  for (int i=0;i<HELLO_MESSAGES;i++)
  {
   Serial.println(bootx[i]);
  } 

  GLBplaySet=playNoiseColor;

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
void loop() { 

  //------------- INPUT REFRESHING ----------------
  // distance is readed when timer expired
  timerUltrasonic.run();


  //--------- TIME TO THINK MY FRIEND -------------
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
    processSerialInputString();
  }

  r.refresh();
  l.refresh();
  FastLED.show();
}

