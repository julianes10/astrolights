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

const char inputBootStringFLS[] PROGMEM ={":LP0080:LT0030:LMK:LCFF,00,00"};

const char inputBootStringSLS[] PROGMEM ={":MP0080:MT0030:MMK:MC00,00,FF"};


Ultrasonic ultrasonic(11, 12,20000UL);
int distance;
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


//------------------------------------------------
//--- GLOBAL FUNCTIONS ---------------------------
//------------------------------------------------



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


//------------------------------------------------

void cbUltrasonic(void)
{
  distance = ultrasonic.read();
  Serial.println(F("DEBUG: Distance CM:"));
  Serial.println(distance);
}



//------------------------------------------------

void setup() { 

  // Serial to debug AND comunication protocolo with PI              
  Serial.begin(9600);
  Serial.println(F("Setup... 'came on, be my baby, came on'"));
  GLBserialInputString[0]=0;

  GLBptrStateFunc = STATE_init;
  Serial.println(F("STATE INIT"));


  l.customProtocolId('M');
  FastLED.addLeds<WS2812B,DATA_PIN_LS_RIGHT,GRB>(RightLS, NUM_LEDS);
  FastLED.addLeds<WS2812B,DATA_PIN_LS_LEFT,GRB>(LeftLS, NUM_LEDS);

  distance=200;
  timerUltrasonic.setInterval(200,cbUltrasonic);
 
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
  lights.f->processCommands(GLBauxString);
  lights.s->processCommands(GLBauxString);
}

//-------------------------------------------------
void STATE_init(void)
{

  Serial.println(F("DEBUG: inputBootStringFLS..."));
  strcpy_P(GLBauxString,(char*)inputBootStringFLS);
  lights.f->processCommands(GLBauxString);
  strcpy_P(GLBauxString,(char*)inputBootStringSLS);
  lights.s->processCommands(GLBauxString);


  GLBptrStateFunc=STATE_welcome;
  Serial.println(F("STATE INIT -> WELCOME"));
}
//-------------------------------------------------
void STATE_welcome(void)
{
  if (r->isIdle()) {
    GLBptrStateFunc=STATE_idle;
    Serial.println(F("STATE WELCOME -> IDLE"));
  }
}
//-------------------------------------------------
void STATE_idle(void)
{
  if (GLBserialInputStringReady){
    processSerialInputString();

  }
}

const char nearf[]      PROGMEM ={":LP0080:LT0020:LMA:LC"};
const char nears[]      PROGMEM ={":MP0080:MT0020:MMA:MC"};
const char veryNearf[]  PROGMEM ={":LP0080:LT0020:LMN"};
const char veryNears[]  PROGMEM ={":MP0080:MT0020:MMN"};
//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------
void loop() { 

  //------------- INPUT REFRESHING ----------------
  // distance is readed when timer expired
  timerUltrasonic.run();

  //--------- TIME TO THINK MY FRIEND -------------
  // State machine as main controller execution
  GLBptrStateFunc();


  // ------------- OUTPUT REFRESHING ---------------
  char aux[20];
  char aux2[10];
  if (r.isIdle() && l.isIdle())
  {
    if (distance < 15)
    {
      strcpy_P(aux,(char*)veryNearf);
      f->processCommands(aux);
      strcpy_P(aux,(char*)veryNears);
      s->processCommands(aux);
    }
  }
  //setFront();//TESTT PPPPPPPPPPPPPPPPPPPPPPPPPPPP TODO
  r.refresh();
  l.refresh();
  FastLED.show();
}

