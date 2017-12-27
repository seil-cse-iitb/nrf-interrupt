#include <RF24.h>
#include <nRF24L01.h>
#include <SPI.h>

// Smoothing
const int numOfReadings = 10;

int readings1[numOfReadings];
int readings2[numOfReadings];
int readings3[numOfReadings];

int total1 = 0;
int total2 = 0;
int total3 = 0;

int avergae1 = 0;
int avergae2 = 0;
int avergae3 = 0;

int readIndex = 0;

// Ultrasonic sensor pin-outs
const int tp1 = 10;
const int ep1 = 3;
const int tp2 = 4;
const int ep2 = 5;
const int tp3 = 6;
const int ep3 = 7;

// RF Pin-outs
const int pinIRQ = 2;
const int pinCE = 8;
const int pinCSN = 9;
RF24 radio(pinCE, pinCSN);

const uint64_t rAddress = 0x000052131ELL;
const uint64_t wAddress = 0x000062132FLL;

bool received = false;
// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.

bool measured  = false;

float prevSensorValue = 0.0;
float currSensorValue = 0.0;
float lowestSensorValue = 1000;

struct data_rx
{
  char dataType;
  byte dataLength;
  byte command;
} data;

struct data_tx
{
  float usData;
}sensorData;

//char tx_data[10] = "";

class Ultrasonic {
  public:
    int triggerPin;
    int echoPin;
    volatile long echoStart = 0;                         // Records start of echo pulse
    volatile long echoEnd = 0;                           // Records end of echo pulse
    volatile long echoDuration = 0;                      // Duration - difference between end and start
    volatile int triggerTimeCount = 0;                  // Count down counter to trigger pulse time

    Ultrasonic(int tp, int ep) {
      triggerPin = tp;
      echoPin = ep;
      pinMode(triggerPin, OUTPUT);
      pinMode(echoPin, INPUT);
    }
    void echo_measure()
    {
      while (!digitalRead(echoPin));
      echoStart = micros();

      while (digitalRead(echoPin));
      echoEnd = micros();
      echoDuration = echoEnd - echoStart;
    }

    float get_distance() {
      return (echoDuration / 58);
    }

    void trigger_pulse() {
      digitalWrite(this->triggerPin, HIGH);
      delayMicroseconds(50);
      digitalWrite(this->triggerPin, LOW);
    }
};

Ultrasonic us1(tp1, ep1);
Ultrasonic us2(tp2, ep2);
Ultrasonic us3(tp3, ep3);

//char data[10] = "";
/*
  void radioListen()
  {
  while (radio.available())
  {
    //radio.read(&ac_state, sizeof(ac_state));
    radio.read(&data, sizeof(data));
    received = true;
  }
  }
*/

void radioWrite()
{
  radio.stopListening();
  int failCount = 0;
  delay(20);

  //Serial.println("Inside radioWrite()");
  delay(20);
  while (!radio.write(&sensorData, sizeof(sensorData)))
  {
    if (failCount > 15)
    {
      //Serial.println("Failed");
      failCount = 0;
      break;
    }
    failCount;
  }
  radio.startListening();
}

void initRadio()
{
  radio.begin();
  radio.setAutoAck(1);
  radio.setRetries(15, 15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.maskIRQ(1, 1, 0);
  radio.setChannel(105);
  radio.openWritingPipe(wAddress);
  radio.openReadingPipe(1, rAddress);
  radio.startListening();
}

void irqFunction()
{
  while (radio.available())
  {
    radio.read(&data, sizeof(data));
  }
}

void setup() {
  // put your setup code here, to run once
  //Serial.begin(115200);
  //Serial.println("____ULTRASONIC_NRF____");
  //Serial.println();

  initRadio();

  attachInterrupt(0, irqFunction, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (data.command == 1)
  {
    us1.trigger_pulse();
    us1.echo_measure();
    //Serial.print("The distance1 is ");
    currSensorValue = us1.get_distance();
    if (currSensorValue < lowestSensorValue) lowestSensorValue = currSensorValue;
    //Serial.println();

    us2.trigger_pulse();
    us2.echo_measure();
    //Serial.print("The distance2 is ");
    currSensorValue = us2.get_distance();
    if (currSensorValue < lowestSensorValue) lowestSensorValue = currSensorValue;
    //Serial.println();

    us3.trigger_pulse();
    us3.echo_measure();
    //Serial.print("The distanc3 is ");
    currSensorValue = us3.get_distance();
    if (currSensorValue < lowestSensorValue) lowestSensorValue = currSensorValue;
    //Serial.println();
    //Serial.println();

    measured = true;
    //Serial.begin(115200);
    //Serial.println("The distance is ");
    //Serial.end();
    
    delay(10);
  }

  if (measured)
  {
    sensorData.usData = lowestSensorValue;
    
    Serial.begin(115200);
    Serial.print("The distance is ");
    Serial.println(lowestSensorValue);
    Serial.println();
    Serial.end();

    radioWrite();
    
    delay(100);
    
    lowestSensorValue = 1000;

    measured = false;
  }
}
