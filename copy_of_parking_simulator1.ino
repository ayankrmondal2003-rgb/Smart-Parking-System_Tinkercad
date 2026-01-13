#include <LiquidCrystal.h>
#include <Servo.h>

//SERVO OBJECTS
Servo S1;   // Entry gate
Servo S2;   // Exit gate

#define OPEN_ANGLE  90
#define CLOSE_ANGLE 0

//LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//PIR (LCD CONTROL ONLY)
#define LCD_PIR        A2   // PIR output
#define LCD_BACKLIGHT  A3   // LCD pin 15 (LED+)

//ENTRY / EXIT ULTRASONIC (3-PIN)
#define ENTRY_US 6
#define EXIT_US  13

//SLOT ULTRASONIC (HC-SR04)
#define TRIG1 7
#define ECHO1 8
#define TRIG2 A0
#define ECHO2 A1

//SLOT LED INDICATORS
#define SLOT1_LED A4
#define SLOT2_LED A5

//DISTANCE THRESHOLDS
#define SLOT_THRESHOLD   40   // cm
#define GATE_THRESHOLD   30   // cm

//FUNCTIONS

// 3-pin ultrasonic (entry & exit)
long read3PinUltrasonic(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(2);
  digitalWrite(pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin, LOW);

  pinMode(pin, INPUT);
  long duration = pulseIn(pin, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}

// 4-pin ultrasonic (slots)
long read4PinUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}

//SETUP
void setup() {

  // Servos
  S1.attach(10);
  S2.attach(9);
  S1.write(CLOSE_ANGLE);
  S2.write(CLOSE_ANGLE);

  // Ultrasonic pins
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  // PIR + LCD backlight
  pinMode(LCD_PIR, INPUT);
  pinMode(LCD_BACKLIGHT, OUTPUT);
  digitalWrite(LCD_BACKLIGHT, LOW);   // LCD OFF initially

  // Slot LEDs
  pinMode(SLOT1_LED, OUTPUT);
  pinMode(SLOT2_LED, OUTPUT);
  digitalWrite(SLOT1_LED, LOW);
  digitalWrite(SLOT2_LED, LOW);

  // LCD init
  lcd.begin(16, 2);
  lcd.print("Smart Parking");
  lcd.setCursor(0, 1);
  lcd.print("System");
  delay(2000);
  lcd.clear();

  Serial.begin(9600);
}

//LOOP
void loop() {

 //LCD POWER CONTROL (PIR ONLY)
  bool pirDetected = digitalRead(LCD_PIR);

  if (pirDetected) {
    digitalWrite(LCD_BACKLIGHT, HIGH);   // LCD ON
  } else {
    digitalWrite(LCD_BACKLIGHT, LOW);    // LCD OFF
  }

  //SLOT DETECTION
  long slot1Dist = read4PinUltrasonic(TRIG1, ECHO1);
  long slot2Dist = read4PinUltrasonic(TRIG2, ECHO2);

  lcd.setCursor(0, 0);
  if (slot1Dist > 0 && slot1Dist < SLOT_THRESHOLD) {
    lcd.print("Slot 1 = NA ");
  } else {
    lcd.print("Slot 1 = A  ");
  }

  lcd.setCursor(0, 1);
  if (slot2Dist > 0 && slot2Dist < SLOT_THRESHOLD) {
    lcd.print("Slot 2 = NA ");
  } else {
    lcd.print("Slot 2 = A  ");
  }

//SLOT LED CONTROL
  if (slot1Dist > 0 && slot1Dist < SLOT_THRESHOLD) {
    digitalWrite(SLOT1_LED, HIGH);   // Slot 1 occupied
  } else {
    digitalWrite(SLOT1_LED, LOW);    // Slot 1 empty
  }

  if (slot2Dist > 0 && slot2Dist < SLOT_THRESHOLD) {
    digitalWrite(SLOT2_LED, HIGH);   // Slot 2 occupied
  } else {
    digitalWrite(SLOT2_LED, LOW);    // Slot 2 empty
  }

  //ENTRY GATE (ULTRASONIC ONLY)
  long entryDist = read3PinUltrasonic(ENTRY_US);
  if (entryDist > 0 && entryDist < GATE_THRESHOLD) {
    S1.write(OPEN_ANGLE);
  } else {
    S1.write(CLOSE_ANGLE);
  }

  //EXIT GATE (ULTRASONIC ONLY)
  long exitDist = read3PinUltrasonic(EXIT_US);
  if (exitDist > 0 && exitDist < GATE_THRESHOLD) {
    S2.write(OPEN_ANGLE);
  } else {
    S2.write(CLOSE_ANGLE);
  }
  Serial.print("PIR: "); Serial.print(pirDetected);
  Serial.print(" | Entry: "); Serial.print(entryDist);
  Serial.print(" | Exit: "); Serial.print(exitDist);
  Serial.print(" | Slot1: "); Serial.print(slot1Dist);
  Serial.print(" | Slot2: "); Serial.println(slot2Dist);

  delay(300);
}
