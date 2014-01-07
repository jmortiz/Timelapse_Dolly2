#include <math.h>
#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#include "dollyStepper.h"
#include "defines.h"

DollyStepper stepper(DIR_PIN, STEP_PIN,STEPPER_ENABLE_PIN);
boolean dolly_is_home = false;
boolean dolly_is_away = false;
boolean from_pause = false;
unsigned char program_state = STATE_INIT;
unsigned char new_program_state = STATE_INIT;
LCDKeypad lcd;
unsigned int photo_interval = MIN_INTERVAL;
unsigned int total_time = 2000;
unsigned int min_total_time = 2000;
unsigned int max_total_time = 2000;
unsigned int n_photos=0;
unsigned int photos_taken=0;
double deltax;
unsigned long time_last_photo;
unsigned char photos_per_interval=1;

//Up and Down arrows
byte c_up[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
};

byte c_down[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
};

void setup() {
  Serial.begin(115200);
  Serial.println("Ready");
  
  //configure stepper
  stepper.set_steps_per_mm(STEPS_PER_MM);
  stepper.set_freq(400);
  
  //end stops
  pinMode(HOME_PIN,INPUT);
  attachInterrupt(0,detected_end_stop_home,CHANGE);
  if(digitalRead(HOME_PIN) == LOW) //pressed
    dolly_is_home = true;
  pinMode(AWAY_PIN,INPUT);
  attachInterrupt(1,detected_end_stop_away,CHANGE);
  if(digitalRead(AWAY_PIN) == LOW) //pressed
    dolly_is_away = true;
  digitalWrite(STEPPER_ENABLE_PIN,HIGH);
  
  //camera control pins
  pinMode(FOCUS_PIN,OUTPUT);
  pinMode(SHUTTER_PIN,OUTPUT);
  digitalWrite(FOCUS_PIN,LOW);
  digitalWrite(SHUTTER_PIN,LOW);
  /*
  digitalWrite(FOCUS_PIN,HIGH);
  delay(1000);
  digitalWrite(SHUTTER_PIN,HIGH);
  delay(500);
  digitalWrite(FOCUS_PIN,LOW);
  digitalWrite(SHUTTER_PIN,LOW);*/
  
  //LCD
  lcd.createChar(2,c_up);
  lcd.createChar(3,c_down);
  lcd.begin(16, 2);
  lcd.clear();
  
  //calibration
  if(lcd.button()==KEYPAD_SELECT)
  {
    lcd.print("Calibration");
    lcd.setCursor(0,1);
    lcd.print("10000 steps");
    stepper.step(10000);
    waitButton();
    lcd.clear();
  }
  
  //Welcome message
  lcd.print("Timelapse Dolly");
  lcd.setCursor(0,1);
  lcd.print("v0.1");
  delay(3000);
  lcd.clear();
}

void loop() {
  int buttonPressed;
  double temp;
  unsigned long time = millis();
  int i;
  //execute actions of state
  //change program state if change detected
  switch(program_state)
  {
    case STATE_INIT:
      if(dolly_is_home)
        new_program_state = STATE_ASK_INTERVAL;
      else
        new_program_state = STATE_ASK_HOME;
      break;
    case STATE_ASK_HOME:
      do
      {
        buttonPressed=waitButton();
      }
      while(!(buttonPressed==KEYPAD_UP || buttonPressed==KEYPAD_DOWN));
      if(buttonPressed == KEYPAD_UP) //YES, go home
        new_program_state = STATE_GO_HOME;
      else if(buttonPressed == KEYPAD_DOWN) //NO
        new_program_state = STATE_ASK_INTERVAL;
      break;
    case STATE_GO_HOME:
      if(dolly_is_home)
      {
        new_program_state = STATE_ASK_INTERVAL;
        //stepper.set_freq(300);
      }
      break;
    case STATE_ASK_INTERVAL:
      do
      {
        buttonPressed=waitButton();
        if(buttonPressed == KEYPAD_UP)
        {
          if(photo_interval < MAX_INTERVAL)
          {
            photo_interval++;
            lcd.setCursor(0,1);
            lcd.print(String(photo_interval)+" s ");
            lcd.write(2);
            lcd.write(' ');
            lcd.write(3);
            lcd.print("  ");
          }
        }
        else if(buttonPressed == KEYPAD_DOWN)
        {
          if(photo_interval > MIN_INTERVAL)
          {
            photo_interval--;
            lcd.setCursor(0,1);
            lcd.print(String(photo_interval)+" s ");
            lcd.write(2);
            lcd.write(' ');
            lcd.write(3);
            lcd.print("  ");
          }
        }
      }
      while(!(buttonPressed==KEYPAD_SELECT));
      new_program_state = STATE_ASK_PHOTOS_PER_INTERVAL;
      break;
      
    case STATE_ASK_PHOTOS_PER_INTERVAL:
      do
      {
        buttonPressed=waitButton();
        if(buttonPressed == KEYPAD_UP)
        {
          photos_per_interval = 3;
          lcd.setCursor(0,1);
          lcd.print(String(photos_per_interval) + " ");
          lcd.write(2);
          lcd.write(' ');
          lcd.write(3);
          lcd.print("  ");
        }
        else if(buttonPressed == KEYPAD_DOWN)
        {
          photos_per_interval = 1;
          lcd.setCursor(0,1);
          lcd.print(String(photos_per_interval) + " ");
          lcd.write(2);
          lcd.write(' ');
          lcd.write(3);
          lcd.print("  ");
        }
      }
      while(!(buttonPressed==KEYPAD_SELECT));
      new_program_state = STATE_ASK_TOTAL_TIME;
      break;
    case STATE_ASK_TOTAL_TIME:
      do
      {
        buttonPressed=waitButton();
        if(buttonPressed == KEYPAD_UP)
        {
          if(total_time < max_total_time)
          {
            total_time += 60;
            lcd.setCursor(0,1);
            lcd.print(String(total_time/60)+" min ");
            lcd.write(2);
            lcd.write(' ');
            lcd.write(3);
            lcd.print("  ");
          }
        }
        else if(buttonPressed == KEYPAD_DOWN)
        {
          if(total_time > min_total_time)
          {
            total_time -= 60;
            lcd.setCursor(0,1);
            lcd.print(String(total_time/60)+" min ");
            lcd.write(2);
            lcd.write(' ');
            lcd.write(3);
            lcd.print("  ");
          }
        }
      }
      while(!(buttonPressed==KEYPAD_SELECT));
      new_program_state = STATE_ASK_START;
      break;
    case STATE_ASK_START:
      do
      {
        buttonPressed=waitButton();
      }
      while(!(buttonPressed==KEYPAD_UP || buttonPressed==KEYPAD_DOWN));
      if(buttonPressed == KEYPAD_UP) //YES, start
        new_program_state = STATE_TIMELAPSE;
      else if(buttonPressed == KEYPAD_DOWN) //NO
        new_program_state = STATE_ASK_INTERVAL;
      break;
    case STATE_TIMELAPSE:
        if(time - time_last_photo >= photo_interval*1000)
        {
          delay(stepper.move_mm(deltax));
          delay(5);
          stepper.stop();
          delay(100);
          press_focus();
          //TODO: photos per interval should be configured on startup
          for(i=0; i<photos_per_interval; i++)
          {
            //Serial.println("photo"+String(photos_taken));
            take_photo();
            delay(1000);
          }
          release_focus();
          photos_taken++;
          time_last_photo = time;
          
          lcd.setCursor(0,0);
          lcd.print("Photo "+String(photos_taken)+"/"+String(n_photos));
        }
        if(photos_taken >= n_photos || dolly_is_away)
          new_program_state = STATE_ASK_HOME;
          
        if ((buttonPressed=lcd.button()) == KEYPAD_UP)
          new_program_state = STATE_PAUSE;
        else if ((buttonPressed=lcd.button()) == KEYPAD_DOWN)
          new_program_state = STATE_ASK_STOP;
        break;
    case STATE_ASK_STOP:
        do
          {
            buttonPressed=waitButton();
          }
        while(!(buttonPressed==KEYPAD_UP || buttonPressed==KEYPAD_DOWN));
          if(buttonPressed == KEYPAD_UP) //YES, Stop
          new_program_state = STATE_ASK_HOME;
          else if(buttonPressed == KEYPAD_DOWN) //NO, continue
          {
            new_program_state = STATE_TIMELAPSE;
            from_pause = true;
          }
        break;
     case STATE_PAUSE:
       do
          {
            buttonPressed=waitButton();
          }
        while(!(buttonPressed==KEYPAD_UP || buttonPressed==KEYPAD_DOWN));
          if(buttonPressed == KEYPAD_UP) //YES, Resume
          {
            new_program_state = STATE_TIMELAPSE;
            from_pause = true;
          }
          else if(buttonPressed == KEYPAD_DOWN) //NO, Stop
            new_program_state = STATE_ASK_STOP;
        break;
  }
  
  //execute on state change actions
  if(new_program_state != program_state)
  {
    switch(new_program_state)
    {
      case STATE_ASK_HOME:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Go home?");
        lcd.setCursor(0,1);
        lcd.write(2);
        lcd.print(": YES ");
        lcd.write(3);
        lcd.print(": NO");
        break;
      case STATE_GO_HOME:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Please Wait...");
        //stepper.set_freq(400);
        stepper.move(-1);
        break;
      case STATE_ASK_INTERVAL:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Photo Interval:");
        lcd.setCursor(0,1);
        lcd.print(String(photo_interval)+" s ");
        lcd.write(2);
        lcd.write(' ');
        lcd.write(3);
        lcd.write(' ');
        break;
      case STATE_ASK_PHOTOS_PER_INTERVAL:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Photos P/Inter.:");
        lcd.setCursor(0,1);
        lcd.print(String(photos_per_interval) + " ");
        lcd.write(2);
        lcd.write(' ');
        lcd.write(3);
        lcd.write(' ');
        break;
      case STATE_ASK_TOTAL_TIME:
        temp = TOTAL_LENGTH*STEPS_PER_MM*MIN_INTERVAL/((double)stepper.get_freq()*MAX_FOCUS_TIME);
        min_total_time = temp;
        temp = TOTAL_LENGTH*(double)photo_interval/MIN_DELTAX;
        max_total_time = temp;
        total_time = min_total_time;
        Serial.println("max total time: "+String(max_total_time));
        Serial.println("min total time: "+String(min_total_time));
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Total time:");
        lcd.setCursor(0,1);
        lcd.print(String(total_time/60)+" min ");
        lcd.write(2);
        lcd.write(' ');
        lcd.write(3);
        lcd.write(' ');
        break;
      case STATE_ASK_START:
        temp = (double)total_time/(double)photo_interval;
        n_photos = temp;
        deltax = TOTAL_LENGTH/temp;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("N: "+String(n_photos));
        lcd.print(" Start?");
        lcd.setCursor(0,1);
        lcd.write(2);
        lcd.print(": YES ");
        lcd.write(3);
        lcd.print(": NO");
        break;
      case STATE_TIMELAPSE:
        if (from_pause == false)
        {
          time_last_photo = time;
          photos_taken=0;
          press_focus();
          //TODO: photos per interval should be configured on startup
          for(i=0; i<photos_per_interval; i++)
          {
            //Serial.println("photo"+String(photos_taken));
            take_photo();
            delay(1000);
          }
          release_focus();
          Serial.println("1st photo");
        }
        else
        from_pause = false;
        
        lcd.clear();
        lcd.print("Photo "+String(photos_taken)+"/"+String(n_photos));
        lcd.setCursor(0,1);
        lcd.write(2);
        lcd.print(": PAUSE ");
        lcd.write(3);
        lcd.print(": STOP");
        break;
      case STATE_ASK_STOP:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Stop?");
        lcd.setCursor(0,1);
        lcd.write(2);
        lcd.print(": YES ");
        lcd.write(3);
        lcd.print(": NO");
        break;
      case STATE_PAUSE:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Resume?");
        lcd.setCursor(0,1);
        lcd.write(2);
        lcd.print(": YES ");
        lcd.write(3);
        lcd.print(": NO");
        break;
    }
  }
  program_state = new_program_state;
  
  delay(5);
}

int waitButton()
{
  int buttonPressed; 
  waitReleaseButton();
  lcd.blink();
  while((buttonPressed=lcd.button())==KEYPAD_NONE)
  {
  }
  delay(50);  
  lcd.noBlink();
  return buttonPressed;
}

void waitReleaseButton()
{
  delay(50);
  while(lcd.button()!=KEYPAD_NONE)
  {
  }
  delay(50);
}

//ISR
void detected_end_stop_home()
{
  //debounce first
  delay(100);
  if(digitalRead(HOME_PIN) == LOW) //pressed
  {
    dolly_is_home = true;
    stepper.stop();
  }
  else
    dolly_is_home = false;
}

//ISR
void detected_end_stop_away()
{
  //debounce first
  delay(100);
  if(digitalRead(AWAY_PIN) == LOW) //pressed
  {
    dolly_is_away = true;
    stepper.stop();
  }
  else
    dolly_is_away = false;
}

void press_focus()
{
  digitalWrite(FOCUS_PIN,HIGH);
  delay(500);
}

void release_focus()
{
  digitalWrite(FOCUS_PIN,LOW);
}

void take_photo()
{
  digitalWrite(SHUTTER_PIN,HIGH);
  delay(50);
  digitalWrite(SHUTTER_PIN,LOW);
  delay(50);
}

