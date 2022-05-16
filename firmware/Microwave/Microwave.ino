/*   Microwave Controller   */

// пины
#define SOUND_PIN D0
#define BTN_START D5
#define BTN_STOP  D6
#define BTN_DOOR  D7
#define RELE D1               			// Relay on D1 pin

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
  pinMode(RELE, OUTPUT);
  pinMode(SOUND_PIN, OUTPUT);
  
  digitalWrite(RELE, LOW);          	// Relay Off on Startup
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
      digitalWrite(RELE, LOW);      	// Relay Off
      TM1637KL.point(POINT_OFF);
      ShowTimer();
    } else {
      newTime =  millis();          	// Get current time
      deltaTime = deltamills(oldTime, newTime); // Разница времени простоя
      if (deltaTime > 1000) {       	// Если разница времени 1 секунда
        Timer--;	
        TM1637KL.point(POINT_ON);
        ShowTimer();
        if (Timer < 4 ) { Beep(); }
        oldTime = millis();         	// Запоминаем время начала работы
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

// Функция обработки кнопок
void buttonTick() {
  if (!digitalRead(BTN_STOP) && !btnFlag1) {
    if (Mode == 1) {                        // Если PAUSE, то STOP
      Mode = 0;
      Timer = 0;
      TM1637KL.point(POINT_OFF);
      ShowTimer();
    }
    if (Mode == 2) {                        // Если RUN, то PAUSE
      Mode = 1;
      digitalWrite(RELE, LOW);              // Выключаем реле
    }
    btnFlag1 = true;
    Beep();
//  btnTime = millis(); // перенес в Beep()
  }
  
  if (digitalRead(BTN_STOP) && btnFlag1) {
    btnFlag1 = false;
  }
  
  if (digitalRead(BTN_DOOR)) {             
    if (!btnFlag3) {
      if (Mode == 2) {                        // Если RUN, то PAUSE
        Mode = 1;
        digitalWrite(RELE, LOW);              // Выключаем реле
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
    if (Mode == 1) {                        // Если PAUSE, то RUN 
      Mode = 2;
      digitalWrite(RELE, HIGH);              // Включаем реле
      oldTime = millis();                   // Записываем время включения
    } else {                                 // Если STOP или RUN, то RUN
      Mode = 2;
      Timer = Timer + 30;
      TM1637KL.point(POINT_ON);
      ShowTimer();
      digitalWrite(RELE, HIGH);             // Включаем реле
      oldTime = millis();                   // Записываем время включения
    }
    btnFlag2 = true;
    Beep();
//  btnTime = millis(); // перенес в Beep()
  }
  
  if (digitalRead(BTN_START) && btnFlag2) {
    btnFlag2 = false;
  }
}

// Функция вычисления разницы времени
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
  analogWrite(SOUND_PIN, 50); // включаем пьезоизлучатель 
  Sound_On = true;
  btnTime = millis(); 
}
