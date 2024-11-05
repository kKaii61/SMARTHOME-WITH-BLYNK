/* Make With Love By Kai*/

// Defines
#define BLYNK_TEMPLATE_ID "your-id"
#define BLYNK_TEMPLATE_NAME "your-template-name"
#define BLYNK_AUTH_TOKEN "auth-token-here"
#define BLYNK_PRINT SwSerial
#define DHTPIN 8
#define DHTTYPE DHT22
#define SENSOR 12

// SoftwareSerial have to be on top
#include <SoftwareSerial.h>
SoftwareSerial SwSerial(A0, A1); // RX, TX
// Includes
#include <DHT.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Wire.h>
#include "DFRobot_LCD.h"
#include <BlynkSimpleStream.h>

// Blynk timer
BlynkTimer timer;
// Temperature sensor init
DHT dht(DHTPIN, DHTTYPE);
// Define Sevo Motor
Servo myservo;
int pos = 0;
// Door Screen
DFRobot_LCD lcd(16, 2);
// Temperature Screen
LiquidCrystal_I2C lcd2(0x27, 16, 2);
const int gauge_size_chars = 16; // width of the gauge in number of characters
char gauge_string[gauge_size_chars + 1];
// Define Pin
int speakerPin = 7;
int buttonPin = 10;
int fanPin = 11;
int lightPin = 13;
const int vibrationPin = A3;
int sensorValue = 0;
const int threshold = 500;
// Variables declare
bool doorUnlock = false;
// Keyboard Init
const byte rows = 4;
const byte cols = 3;
char key[rows][cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};
byte rowPins[rows] = {15, 16, 2, 3};
byte colPins[cols] = {4, 5, 6};
Keypad keypad = Keypad(makeKeymap(key), rowPins, colPins, rows, cols);

// Default Password
char *password = "0123";
int currentposition = 0;

unsigned long prevDHTMillis = 0; // Store the last time the DHT was updated
const long dhtInterval = 1500;
unsigned long prevSensorMillis = 0; // Store the last time the IRSensor was updated
const long sensorInterval = 1000;
unsigned long prevVibrationMillis = 0; // Store the last time the VibrationSensor was updated
const long vsensorInterval = 5000;
unsigned long prevKeyMillis = 0;
const long keyInterval = 50; // Check keypad every 100ms
unsigned long blynkCheckTime = 0;
const long blynkInterval = 10;  // Run Blynk tasks every 10ms


bool danger = false;

// -- Blynk sensor -- //
void sendSensor()
{
    float h = dht.readHumidity();
    float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
    if (isnan(h) || isnan(t))
    {
        SwSerial.println("Failed to read from DHT sensor!");
        return;
    }
    // You can send any value at any time.
    // Please don't send more that 10 values per second.
    Blynk.virtualWrite(V5, t);
    Blynk.virtualWrite(V6, h);
}

// -- MAIN -- //
void (*resetFunc)(void) = 0; // Reset funtion
void setup()
{
    pinMode(buttonPin, INPUT_PULLUP); // SETUP button
    pinMode(speakerPin, OUTPUT);      // SETUP buzzer
    pinMode(fanPin, OUTPUT);          // SETUP fan
    pinMode(SENSOR, INPUT);           // SETUP IRSensor
    pinMode(lightPin, OUTPUT);        // SETUP DoorLight
    pinMode(vibrationPin, INPUT);     // SETUP VibrationSensor

    dht.begin(); // DHTSensor Init

    // Debug console
    SwSerial.begin(115200);

    // Blynk will work through Serial
    // Do not read or write this serial manually in your sketch
    Serial.begin(9600);
    Blynk.begin(Serial, BLYNK_AUTH_TOKEN);

    // DoorScreen booting/testing
    lcd.init();
    lcd.setRGB(255, 0, 0);
    lcd.print("Starting");

    // TempScreen booting/testing
    lcd2.init();
    lcd2.backlight();
    lcd2.print("Starting");

    myservo.attach(9); // Servo motor connection
    myservo.write(450);
    timer.setInterval(1000L, sendSensor);
}
 char code;
void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - blynkCheckTime >= blynkInterval) {
        blynkCheckTime = currentMillis;
        Blynk.run();
    }
    timer.run();
    
    
   
    // Check keypad input periodically
    if (currentMillis - prevKeyMillis >= keyInterval) {
        prevKeyMillis = currentMillis;
        checkKeypad();
    }

    // Checking vibration every 5 secs
    if (currentMillis - prevVibrationMillis >= vsensorInterval)
    {
        prevVibrationMillis = currentMillis;
        // sensorValue = analogRead(vibrationPin);
        if (digitalRead(vibrationPin) == HIGH)
        {
            danger = true;
            dangerMode();
            lcd2.setCursor(0, 0);
            lcd2.print("EARTHQUAKE ALERT!");
            lcd2.setCursor(0, 1);
            lcd2.print("EARTHQUAKE ALERT!");

            lcd.clear();
            lcd.print("EARTHQUAKE ALERT!");
        }
        else if (digitalRead(vibrationPin) == LOW && danger == true)
        {
            danger = false;
            resetFunc();
        }
        else
        {
        }
    }

    // Checking temperature and humidimity every 1.5 secs
    if (currentMillis - prevDHTMillis >= dhtInterval && danger == false)
    {
        prevDHTMillis = currentMillis;
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        // TempScreen output
        lcd2.setCursor(0, 0);
        lcd2.print("Temp: ");
        lcd2.print(t);
        lcd2.print("C");
        lcd2.setCursor(0, 1);
        lcd2.print("Humid: ");
        lcd2.print(h);
        lcd2.print("%");
        // Fan condition
        // Should have check there's anyone inside the home to open the fan
        if (round(t) >= 28)
        {
            digitalWrite(fanPin, HIGH);
        }
        else
        {
            digitalWrite(fanPin, LOW);
        }
    }

    // Check if there any person
    if (currentMillis - prevSensorMillis >= sensorInterval)
    {
        prevSensorMillis = currentMillis;
        if (digitalRead(SENSOR))
        {
            digitalWrite(lightPin, HIGH);
        }
        else
        {
            digitalWrite(lightPin, LOW);
        }
        // Turn off light if danger
        if (danger == true)
        {
            digitalWrite(lightPin, LOW);
        }
    }

    // BUTTON TO UNLOCK
    if (digitalRead(buttonPin) == LOW)
    {
        doorUnlock = true;
        clearscreen();
        unlockdoor();
    }

    // If no input from keypad
    // if (currentposition == 0)
    // {
    //   displayscreen();
    // }
}

void checkKeypad(){
  code = keypad.getKey();
  if (code != NO_KEY) {
    handleKeypadInput(code);
  }
}

void reset() {
    currentposition = 0;
    displayscreen();
}

void handleKeypadInput(char code) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PASSWORD:");

    lcd.setCursor(7, 1);
    for (int i = 0; i < currentposition; ++i) {
        lcd.print("*");
    }

    if (code == password[currentposition]) {
        ++currentposition;
        if (currentposition == 4) { // Assuming password length is 4
            doorUnlock = true;
            unlockdoor();
            reset();
        }
    } else {
        doorUnlock = false;
        incorrect();
        reset();
    }
}


//------------------ Function 1 - DANGERMODE --------------//

void dangerMode()
{
    if (danger == true)
    {
        for (pos = 180; pos >= 0; pos -= 5) // open the door immediately
        {
            myservo.write(pos);
            // delay(1);
        }
        digitalWrite(fanPin, LOW);
        tone(speakerPin, 1000);
    }
}

//------------------ Function 2 - Open the door --------------//
unsigned long lightOnTime = 0; // To store the time when the light is turned on
const long lightDuration = 7000;
bool lightOn = false;

void unlockdoor()
{
    unsigned long curtMillis = millis();
    if (doorUnlock == true && !lightOn)
    {                                 // If the door is unlocked and the light is off
        digitalWrite(lightPin, HIGH); // Turn the light on
        lightOnTime = curtMillis;     // Save the time when the light was turned on
        lightOn = true;               // Set the flag indicating the light is on
    }
    if (lightOn && (curtMillis - lightOnTime >= lightDuration))
    {
        digitalWrite(lightPin, LOW); // Turn the light off
        lightOn = false;             // Reset the flag
    }
    lcd.setCursor(0, 0);
    lcd.println(" ");
    lcd.setCursor(1, 0);
    lcd.print("Access Granted");
    lcd.setCursor(4, 1);
    lcd.println("WELCOME!!");
    lcd.setCursor(15, 1);
    lcd.println(" ");
    lcd.setCursor(16, 1);
    lcd.println(" ");
    lcd.setCursor(14, 1);
    lcd.println(" ");
    lcd.setCursor(13, 1);
    lcd.println(" ");

    processDoor();
    lcd.clear();
    displayscreen();
}

// --------- EXTRA FUNCTIONS ----------------- //
void processDoor()
{
    for (pos = 180; pos >= 0; pos -= 5) // open the door
    {
        myservo.write(pos);
        // delay(5);
    }
    doorUnlock = true;
    counterbeep();
    doorUnlock = false;
}

//--------------------Function 3- Wrong code--------------//

void incorrect()
{
    // delay(500);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("CODE");
    lcd.setCursor(6, 0);
    lcd.print("INCORRECT");
    lcd.setCursor(15, 1);
    lcd.println(" ");
    lcd.setCursor(4, 1);
    lcd.println("GET AWAY!!!");

    lcd.setCursor(13, 1);
    lcd.println(" ");
    // delay(3000);
    lcd.clear();
    displayscreen();
}

//-------Function 4 - CLEAR THE DOORSCREEN--------------------/

void clearscreen()
{
    lcd.setCursor(0, 0);
    lcd.println(" ");
    lcd.setCursor(0, 1);
    lcd.println(" ");
    lcd.setCursor(0, 2);
    lcd.println(" ");
    lcd.setCursor(0, 3);
    lcd.println(" ");
}

//------------Function 5 - DISPLAY DOORSCREEN FUNCTION--------------------//

void displayscreen()
{
    lcd.setCursor(0, 0);
    lcd.println("*ENTER THE CODE*");
    lcd.setCursor(1, 1);
    lcd.println("TO OPEN DOOR!!");
}

//--------------Function 6 - Count down of door locking ------------------//

const int countdown[] = {5, 4, 3, 2, 1}; // Array for countdown numbers

void counterbeep()
{
    // Countdown and display message
    for (int i = 0; i < 5; i++)
    {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.println("GET IN WITHIN:");
        lcd.setCursor(4, 1);
        lcd.print(countdown[i]); // Display countdown number
        // delay(100);
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.println("GET IN WITHIN:");
        // delay(100);  // Delay between countdown steps
    }
    doorUnlock = true;
    // Re-locking sequence
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("RE-LOCKING");

    // Close the door in steps and emit sound
    moveServoWithSound(0, 60, 200); // First movement (0 to 60) with 200 Hz tone
    lcd.setCursor(12, 0);
    lcd.print(".");

    moveServoWithSound(60, 120, 500); // Second movement (60 to 120) with 500 Hz tone
    lcd.setCursor(13, 0);
    lcd.print(".");

    moveServoWithSound(120, 180, 1000); // Final movement (120 to 180) with 1000 Hz tone
    lcd.setCursor(14, 0);
    lcd.print(".");

    // delay(400);
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("LOCKED!");
    // delay(440);
}

// Function to move the servo and emit sound
void moveServoWithSound(int startPos, int endPos, int toneFreq)
{
    for (int pos = startPos; pos <= endPos; pos += 5)
    {
        myservo.write(pos); // Move servo in steps of 5 degrees
       // delay(12);             // Delay between steps
    }
    currentposition = endPos; // Update the servo position

    // Emit a sound based on the given frequency
    tone(speakerPin, toneFreq);
    // delay(100);
    noTone(speakerPin);
}
//--------------Function 8 - FAN on ------------------//
void fanOn()
{
    digitalWrite(fanPin, HIGH);
    // delay(1000);
}

/* Make With Love By Kai*/