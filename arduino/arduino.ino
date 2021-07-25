#include <NTPClient.h>

#include <Arduino_JSON.h>

#include <ArduinoMqttClient.h>

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ESP8266_ESP12)
  #include <ESP8266WiFi.h>
#endif

#include <Arduino_MKRIoTCarrier.h>
MKRIoTCarrier carrier;

const char broker[] = "18.215.16.26";
int port = 1883;
const char topic[]  = "arduino/echo";


char ssid[] = "Fios-FNMJD";
char pass[] = "hat2500axe467irons";
  
float temperature = 0;
float humidity = 0;
float pressure = 0;

int moistPin = A5;
float moisture = 0;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

void setup() {
  while (!Serial);
  CARRIER_CASE = true;
  carrier.begin();
  carrier.display.setRotation(0);

  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print("Failed... Retrying");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  Serial.print("Subscribing to topic: ");
  Serial.println(topic);
  Serial.println();


  // subscribe to a topic
  mqttClient.subscribe(topic);

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Waiting for messages on topic: ");
  Serial.println(topic);
  Serial.println();
}

void loop() {
  Serial.println("Reading Temperature");

  int messageSize = mqttClient.parseMessage();
  if (messageSize) {
    // we received a message, print out the topic and contents
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");

    // use the Stream interface to print the contents
    while (mqttClient.available()) {
      Serial.print((char)mqttClient.read());
    }
    Serial.println();

    Serial.println();
  }
  
  delay(2000);
  temperature = carrier.Env.readTemperature(); //reads temperature
  Serial.println("Reading Humidity");
  humidity = carrier.Env.readHumidity(); //reads humidity
  
  Serial.println("Reading Pressure");
  pressure = carrier.Pressure.readPressure(); //reads pressure


  int raw_moisture = analogRead(moistPin);
 
  //map raw moisture to a scale of 0 - 100
  moisture = map(raw_moisture, 0, 1023, 100, 0);

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" Â°C");
  
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" *** ");
  
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" *** ");


  Serial.print("Moisture = ");
  Serial.print(raw_moisture);
  Serial.print("---");
  Serial.print(moisture);
  Serial.println(" *** ");

  JSONVar measurements;

  measurements["temperature"] = temperature;
  measurements["humidity"] = humidity;
  measurements["pressure"] = pressure;
  measurements["moisture"] = moisture;
  measurements["source"] = "arduino";

 
  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(topic);
  Serial.println(JSON.stringify(measurements));
  mqttClient.print(JSON.stringify(measurements));
  mqttClient.endMessage();
  
  printTemperature();
  delay(2000);
  printHumidity();
  delay(2000);
  printPressure();
  delay(2000);
  printMoisture();
}

  
void printTemperature() {
  //configuring display, setting background color, text size and text color
  carrier.display.fillScreen(ST77XX_RED); //red background
  carrier.display.setTextColor(ST77XX_WHITE); //white text
  carrier.display.setTextSize(2); //medium sized text
 
  carrier.display.setCursor(30, 110); //sets position for printing (x and y)
  carrier.display.print("Temp: ");
  carrier.display.print(temperature);
  carrier.display.println(" C");
 
}
 
void printHumidity() {
  carrier.display.fillScreen(ST77XX_BLUE);
  carrier.display.setTextColor(ST77XX_WHITE);
  carrier.display.setTextSize(2); //medium sized text
 
  carrier.display.setCursor(30, 110); //sets new position for printing (x and y)
  carrier.display.print("Humi: ");
  carrier.display.print(humidity);
  carrier.display.println(" %");
}


void printPressure() {
  carrier.display.fillScreen(ST77XX_GREEN);
  carrier.display.setTextColor(ST77XX_BLACK);
  carrier.display.setTextSize(2);
 
  carrier.display.setCursor(30, 110);
  carrier.display.print("Press: ");
  carrier.display.print(pressure);
}

void printMoisture() {
  carrier.display.fillScreen(ST77XX_YELLOW);
  carrier.display.setTextColor(ST77XX_BLACK);
  carrier.display.setTextSize(2);
 
  carrier.display.setCursor(30, 110);
  carrier.display.print("Mois: ");
  carrier.display.print(moisture);
}
