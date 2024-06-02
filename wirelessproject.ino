#include "LiquidCrystal_I2C.h"
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "string.h"
#include "stdio.h"
#define REPORTING_PERIOD_MS 1000
#define LCD_ADDR 0x27
#define LEDPIN 4
#define BUZZER_PIN 12
#define SMOKE_THRESH 200
#define DISTANCE_THRESH 10
#define SMOKE_PIN A0
#define TRIG_PIN 9
#define ECHO_PIN 10

PulseOximeter pox;
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

bool isSmoke = false;
bool isPerson = false;
double o2Level = -1;
double heartRate = -1;
char heartReadingString[10];
char o2ReadingString[10];
char line1[50];
char line2[50];
uint32_t tsLastReport = 0;
float duration, distance;
double smokeReading;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  lcd.init();
  lcd.backlight();

  if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } else {
      Serial.println("SUCCESS");
  }

    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
    Serial.begin(9600);
    
}


void loop() {
  isSmoke = smoke_present();
  isPerson = personPresent();

  if(isSmoke){
    digitalWrite(BUZZER_PIN, 1);
    Serial.println("Gas Leak Detected!");
  }else{
    digitalWrite(BUZZER_PIN, 0);
  }

  if(isPerson){
    digitalWrite(LEDPIN, 1);
  }else{
    digitalWrite(LEDPIN, 0);
  }

  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      o2Level = pox.getSpO2();
      heartRate = pox.getHeartRate();

      if(heartRate > 0 || o2Level > 0){
        if(heartRate < 30){
          Serial.println("Patient is in a critical condition");
        }
        dtostrf(heartRate, 3, 0, heartReadingString);
        dtostrf(o2Level, 3, 0, o2ReadingString);
        sprintf(line1, "Heart Rate: %s BPM", heartReadingString);
        sprintf(line2, "O2 Level: %s %", o2ReadingString);
        lcd.setCursor(0, 0);
        lcd.print(line1);
        lcd.setCursor(0, 1);
        lcd.print(line2);
      }
      tsLastReport = millis();
      
  }
}

bool smoke_present(){
  double reading = analogRead(SMOKE_PIN);
  // Serial.println(reading);
  if(reading > SMOKE_THRESH){
    return true;
  }
  return false;
}

bool personPresent(){
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration*.0343)/2;

  if(distance < DISTANCE_THRESH){
    return true;
  }
  return false;
  
}

void onBeatDetected(){}