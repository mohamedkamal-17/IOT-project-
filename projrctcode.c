/* ====================== Includes Section Start ====================== */
#include <Servo.h>
#include <Keypad.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
/* ======================  Includes Section End  ====================== */


/* ======================= Macro Section Start ======================= */
#define HomeDoorServoPin 9   // Define the pin for the servo motor(PWM Pin)
#define BuzzerPin A0        // (PWM Pin)
#define InIrDoorPin A2      // Define the pin for the IR sensor
#define FanPin 13
#define R 10
#define B 11
#define G 12
#define button A1

/* =======================  Macro Section End  ======================= */


/* ================== Global Variables Section Start ================== */
float humidityDHT22;
float temperatureDHT22;
int buttonPushed = 0;
int buttonValue = 0;

bool InIrDoorValue;   // To stor the value of InIR
const byte ROWS = 4;  //four rows
const byte COLS = 3;  //three columns
int DoorState = 0;    // door is closed
int SystemState = 0;  // System is off
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

byte rowPins[ROWS] = { 2, 3, 4, 5 };  //connect to the row pinouts of the keypad
byte colPins[COLS] = { 6, 7, 8 };     //connect to the column pinouts of the keypad

char StorePass[] = "12345";
int NumberOfPass = sizeof(StorePass) - 1;
char EnteredPass[sizeof(StorePass) - 1];
int NumberOfEnteredPass = 0;
int NumberOfCorrectNumbers = 0;
/* ==================  Global Variables Section End  ================== */


/* ================= Global Decleration Section Start ================= */
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 16, 2);
// ===================================================================================================

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN A5     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

// ===================================================================================================

Servo HomeDoorServo;          // Create a servo object to control the servo motor
/* =================  Global Decleration Section End  ================= */

int cnt =0; 


/* ============== void setup Section Start ============== */
void setup() {

  /* ============== pinMode Section Start ============== */
  pinMode(BuzzerPin, OUTPUT);
  pinMode(FanPin, OUTPUT);
  pinMode(InIrDoorPin, INPUT_PULLUP);  // Set IR sensor pin as input
                                /* ==============  pinMode Section End  ============== */

  /* ============== Program Initialization Section Start ============== */
  lcd.init();
  lcd.backlight();
  lcd.print("Enter the Pass :");  // Print a message to the LCD.

  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;


  Serial.begin(9500);
  HomeDoorServo.attach(HomeDoorServoPin);  // Attach the servo to its pin
  HomeDoorServo.write(0);                  // Rotate the servo motor 0 degrees
  /* ==============  Program Initialization Section End  ============== */
}
/* ==============  void setup Section End  ============== */

void loop() {
  Blue();
  while (SystemState == 1) {
    IR();
    DHT();
    Fan();
    TempAlarm();
  }

  EnterCheckPass();
}



/* ==================== Sub-Program Section Start ==================== */
void EnterCheckPass() {
  char key = keypad.getKey();  // to get and stor numbers from keypad

  if (key >= '0' && key != '#') {

    lcd.setCursor(0, 0);  // to set in 1st row
    lcd.print("Press # to check");
    lcd.setCursor(NumberOfEnteredPass, 1);  // to set in 2nd row
    // lcd.print("*"); // to print * for each number that the user enter
    lcd.print(key);  // to print * for each number that the user enter

    NumbersTone();

    EnteredPass[NumberOfEnteredPass] = key;  // to stor each number in EnteredPass array
    NumberOfEnteredPass++;                   // increas the index of EnteredPass array

  }

  else if (key == '#') {
    if (NumberOfEnteredPass == NumberOfPass)  // to check if the user entered count of numbers the same as numbers of stored pass
    {
      for (int j = 0; j < NumberOfPass; j++) {
        if (EnteredPass[j] == StorePass[j]) NumberOfCorrectNumbers++;  // to check each number entered with the stored value
      }

      if (NumberOfCorrectNumbers == NumberOfPass)  // to check if all numbers is true
      {
        DoorState = 1;    // door is open
        SystemState = 1;  // system is on

        CorrectPass();
        OpenHomeDoor();

      }

      else {
        WrongPass();
      }
    }

    else {
      WrongPass();
    }
  }
}


void WrongPass() {
  Red();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wrong Pass !!     ");
  WrongPassAlarm();
  NumberOfEnteredPass = 0;
  NumberOfCorrectNumbers = 0;
  lcd.setCursor(0, 0);
  lcd.print("Enter the Pass :");
  lcd.setCursor(0, 1);
}

void CorrectPass() {
  Green();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Correct Pass    ");
  delay(2000);
  NumberOfEnteredPass = 0;
  NumberOfCorrectNumbers = 0;
  lcd.clear();
}


void OpenHomeDoor() {
  for (int i = 0; i <= 100; i++) {
    HomeDoorServo.write(i);
    delay(30);
  }
  // Rotate the servo motor 100 degrees to open the door slowly
}

void CloseHomeDoor() {
  for (int i = 100; i >= 0; i--) {
    HomeDoorServo.write(i);
    delay(30);
  }
  // Rotate the servo motor 100 degrees to close the door slowly
}

void WrongPassAlarm() {
  for (int i = 1; i <= 12; i++) {
    tone(BuzzerPin, 150, 100);
    delay(300);
    noTone(BuzzerPin);
    delay(100);
  }
}

void NumbersTone() {
  digitalWrite(BuzzerPin, 1);
  delay(100);
  digitalWrite(BuzzerPin, 0);
}

void TempAlarm() {
  buttonValue = digitalRead(button);
  if (buttonValue == 1) {
    buttonPushed = 1;
    noTone(BuzzerPin);
  }
  if (buttonPushed == 0) {

    if (temperatureDHT22 > 35) {
      tone(BuzzerPin, 250, 500);
      delay(500);
      noTone(BuzzerPin);
      delay(100);
    }
  }
}

void DHT() {

  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;

  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    temperatureDHT22 =event.temperature - 560;
  }
  // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    humidityDHT22 = event.relative_humidity;
  }

  lcd.setCursor(0, 0);
  lcd.print("Temperature:");
  lcd.setCursor(12, 0);
  lcd.print(temperatureDHT22);
  Serial.print("Temperature:  ");
  Serial.println(temperatureDHT22);
  lcd.setCursor(0, 1);
  lcd.print("Humidity:");
  lcd.setCursor(10, 1);
  lcd.print(humidityDHT22);
}

void IR() {
  InIrDoorValue = digitalRead(InIrDoorPin);

  if (InIrDoorValue == 0 && DoorState == 0) {
    OpenHomeDoor();
    DoorState = 1;
    delay(10000);
    CloseHomeDoor();
    DoorState = 0;
  }

  if (InIrDoorValue == 0 && DoorState == 1) {
    CloseHomeDoor();
    DoorState = 0;
    delay(4000);
  }
}

void Fan() {
  // temperatureDHT22 = 30;
  if (temperatureDHT22 > 25) {
    digitalWrite(FanPin, 1);
  } else {
    digitalWrite(FanPin, 0);
  }
}

// -----------------------------------------------------------------
// -----------------------------------------------------------------
// -----------------------------------------------------------------
// -----------------------------------------------------------------

void Red() {
  analogWrite(R, 255);
  analogWrite(G, 0);
  analogWrite(B, 0);
}

void Green() {
  analogWrite(R, 0);
  analogWrite(G, 255);
  analogWrite(B, 0);
}

void Blue() {
  analogWrite(R, 0);
  analogWrite(G, 0);
  analogWrite(B, 255);
}
/* ====================  Sub-Program Section End  ==================== */


/**
