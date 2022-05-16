/*   Microwave Controller   */

// пины
#define SOUND_PIN D0
#define BTN_START D5
#define BTN_STOP  D6
#define BTN_DOOR  D7
#define RELAY D1               			// Relay on D1 pin

#include <TM1637.h>

TM1637 TM1637KL(D3, D2);				// CLK, DIO
int8_t Digits[] = {0x7f, 0x7f, 0x7f, 0x7f};
boolean btnFlag1, btnFlag2, btnFlag3, Sound_On;
unsigned long oldTime;              
unsigned long newTime;              
unsigned long btnTime;              
unsigned long deltaTime;            
int Mode = 0, Timer, Minutes, Seconds; 	// Current timer time, Mode = 0-Stop, 1-Pause, 2-Run

void setup() {
  Serial.begin(115200);
  TM1637KL.init();
  TM1637KL.set(3);                  	// Screen Brightness: 0-7
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_STOP,  INPUT_PULLUP);
  pinMode(BTN_DOOR,  INPUT_PULLUP);
  pinMode(RELAY, OUTPUT);
  pinMode(SOUND_PIN, OUTPUT);
  
  digitalWrite(RELAY, LOW);          	// Relay Off on Startup
  digitalWrite(SOUND_PIN, LOW);
  Mode = 0;
  Timer = 0;
  TM1637KL.point(POINT_OFF);
  ShowTimer();
  btnTime = millis();         			// Start time
 }

void loop() {
  newTime =  millis();          		// Get current time

  deltaTime = deltamills(btnTime, newTime); // Contact bounce elimination
  if (deltaTime > 300) { 
    if (Sound_On) { 
      analogWrite(SOUND_PIN, 0); 		// Sound Off 
      Sound_On = false;		
    }
    buttonTick(); 
  }  

  if ( Mode == 2 ) {
    if (Timer == 0){ 
      Mode = 0;
      btnTime = millis();
      digitalWrite(RELAY, LOW);      	// Relay Off
      TM1637KL.point(POINT_OFF);
      ShowTimer();
    } else {
      newTime =  millis();          	// Get current time
      deltaTime = deltamills(oldTime, newTime); // Downtime
      if (deltaTime > 1000) {       	// if downtime > 1 second
        Timer--;	
        TM1637KL.point(POINT_ON);
        ShowTimer();
        if (Timer < 4 ) { Beep(); }
        oldTime = millis();         	// Save as start time
      }  
    }
  }
}

void ShowTimer() {
  Minutes = Timer / 60;
  Seconds = Timer % 60;
  Digits[0] = Minutes / 10;
  Digits[1] = Minutes % 10;
  Digits[2] = Seconds  / 10;
  Digits[3] = Seconds  % 10;  
  if ( Digits[0] == 0 ) { Digits[0] = 0x7f; }
  if (( Digits[1] == 0 ) && ( Digits[0] == 0x7f )) { Digits[1] = 0x7f; }
  if (( Digits[2] == 0 ) && ( Digits[1] == 0x7f ) && ( Digits[0] == 0x7f )) { Digits[2] = 0x7f; }
  TM1637KL.display(Digits);
}

// Button processing function
void buttonTick() {
  if (!digitalRead(BTN_STOP) && !btnFlag1) {
    if (Mode == 1) {                        // If PAUSE, then STOP
      Mode = 0;
      Timer = 0;
      TM1637KL.point(POINT_OFF);
      ShowTimer();
    }
    if (Mode == 2) {                        // If RUN, then PAUSE
      Mode = 1;
      digitalWrite(RELAY, LOW);             // Relay Off
    }
    btnFlag1 = true;
    Beep();
  }
  
  if (digitalRead(BTN_STOP) && btnFlag1) {
    btnFlag1 = false;
  }
  
  if (digitalRead(BTN_DOOR)) {             
    if (!btnFlag3) {
      if (Mode == 2) {                        // If RUN, then PAUSE
        Mode = 1;
        digitalWrite(RELAY, LOW);             // Relay Off
      }
      btnFlag3 = true;
    }
    btnTime = millis();
    return;
  }

  if (!digitalRead(BTN_DOOR) && btnFlag3) {
    btnFlag3 = false;
    return;
  }

  if (!digitalRead(BTN_START) && !btnFlag2) {
    if (Mode == 1) {                        // If PAUSE, then RUN 
      Mode = 2;
      digitalWrite(RELAY, HIGH);            // Relay On
      oldTime = millis();                   // Save On Time
    } else {                                // If STOP or RUN, then RUN
      Mode = 2;
      Timer = Timer + 30;
      TM1637KL.point(POINT_ON);
      ShowTimer();
      digitalWrite(RELAY, HIGH);            // Relay On
      oldTime = millis();                   // Save On Time
    }
    btnFlag2 = true;
    Beep();
  }
  
  if (digitalRead(BTN_START) && btnFlag2) {
    btnFlag2 = false;
  }
}

// Time difference calculation function
unsigned long deltamills(unsigned long t_old, unsigned long t_new) {
  unsigned long delta;
  if ( t_old <= t_new ) {        
    delta = t_new - t_old;
  } else {
    delta = (4294967295 - t_old) + t_new;
  }
  return delta;
}

void Beep() {
  analogWrite(SOUND_PIN, 50); 				// turn on the piezo emitter
  Sound_On = true;
  btnTime = millis(); 
}
