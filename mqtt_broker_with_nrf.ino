#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

// nRF Initialization
const int pinCE = D3;
const int pinCSN = D4;
RF24 radio(pinCE, pinCSN);

const uint64_t wAddress = 0x000052131ELL;
const uint64_t rAddress = 0x000062132FLL;

bool received = false;

// Sending packet
struct data_tx
{
  char dataType = 'I';
  byte dataLength = 1;
  byte command = 1;
}data;

struct data_rx
{
  float usData;
}sensorData;

const char* ssid = "SEIL";
const char* password = "deadlock123";

const char* mqtt_server = "10.129.149.34"; // Shaunak's desktop
//const char* mqtt_server = "10.129.28.158";
const char* mqtt_username = "<MQTT_BROKER_USERNAME>";
const char* mqtt_password = "<MQTT_BROKER_PASSWORD>";

const char* mqtt_topic = "nodemcu/NRF";
const char* mqtt_topic_publish = "nodemcu/pub";
const char* mqtt_topic_command = "nodemcu/ultrasound/command";

const char* client_id = "sensorNode";

WiFiClient espClient;
PubSubClient client(espClient);

const int ledPin = D2;

int count = 0;
int counter = 0; 

void setupWifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to WiFi with IP ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message from topic [");
  Serial.print(topic);
  Serial.println("]");

  if (payload[0] == '1')
  {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED HIGH");
  }
  else if (payload[0] == '0')
  {
    digitalWrite(ledPin, LOW);
    Serial.println("LED LOW");
  }
  else
  {
    Serial.println("Bad data");
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT....");

    if (client.connect(client_id))//, mqtt_username, mqtt_password))
    {
      Serial.println("Connected");
      client.subscribe(mqtt_topic);
    }
    else
    {
      Serial.print("Failed to connect");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void initRadio()
{
  radio.begin();
  radio.setAutoAck(1);
  radio.setRetries(15, 15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(105);
  radio.openWritingPipe(wAddress);
  radio.openReadingPipe(1, rAddress);
  radio.startListening();
}

void radioListen()
{
  while(radio.available())
  {
    radio.read(&sensorData, sizeof(sensorData));
    received = true;
  }
}

void setup()
{
  Serial.begin(115200);

  initRadio();

  // setup WiFi
  setupWifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // setup OneWire bus
  //DS18B20.begin();
  //dht.begin();
  Serial.println(mqtt_topic);

  randomSeed(analogRead(0));
}

void loop()
{
  /*
  if (!client.connected())
  {
    reconnect();
  }

  client.loop();

  char test[] = {'1'};
  client.publish(mqtt_topic_publish, test );
  Serial.println("Published");
  count++;
  delay(1000);
  */

  delay(random(1000, 5000));
  Serial.println("Sending Packet");
  if(data.command == 0) data.command = 1;
  else data.command = 0;
  //data.command = random(0,1);
  Serial.print("Command is ");
  Serial.println(data.command);
  
  radio.stopListening();
  if(radio.write(&data, sizeof(data)))
  {
    Serial.println("Failed");
  }

  Serial.println();
  radio.startListening();
  
  radioListen();

  if(received)
  {
    Serial.print("Ultrasound Data is ");
    Serial.println(sensorData.usData);
    Serial.println();

    received = false;
  }
}

