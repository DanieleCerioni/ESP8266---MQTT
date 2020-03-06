#include <Servo.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//DHT11 temperatura/umidità
#define DHTTYPE 11
//definisco il pin a cui è collegato
#define DHTPIN 5
#include <DHT.h>
#include <DHT_U.h>

//potenziometro
#define sensorPin A0

//buzzer
#define BUZZERPIN 4
char pot[4];

//potenziometro
const int analogPin = A0;
int sensorValue = 0;
int outputValue = 0;

//servo
Servo myservo;

//led
int led=16;

//caratteristiche rete
const char* ssid = "GruppoPSC";   //wi-fi name
const char* password = "protectionUPSC270";   //wifi password
const char* mqtt_server = "10.88.10.92";        //ip del broker
const int mqttPort = 1883;                  //port mqtt
//const char* mqttUser = "otfxknod";           //Userid
//const char* mqttPassword = "nSuUc1dDLygF";   //User Psw


//istanzio un'oggetto DHT per la corretta gestione del sensore
DHT dht(DHTPIN,DHTTYPE);

//variabili temperatura
int t=0;
int h=0;
//varibili per timer
unsigned long previousMillis = 0;
const long interval = 10000;

char temperature_c[10];
char humid[10];

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];        //per pubblicare messaggi
int value = 0;
char tempString[20];     //per pubblicare dati




void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(BUZZERPIN, OUTPUT);
  digitalWrite(BUZZERPIN, LOW);
  myservo.attach(14);
  myservo.write(0); 
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqttPort);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Mqtt Broker");
  Serial.println(mqtt_server);
  tone_connection();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
  String inMsg; 
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    inMsg = (char*)payload;
  }
  Serial.println();

  if(String(topic) == "inboundTopic"){
    Serial.print("Changing led state");
  // Switch on the LED if an 1 was received as first character
  if (inMsg == "1") {   
    digitalWrite(led, HIGH);   
  } else if(inMsg == "0"){
    digitalWrite(led, LOW);  // Turn the LED off by making the voltage HIGH
  } else if (inMsg == "a"){
    myservo.write(0);
    delay(50);
  }else if (inMsg == "b"){
    myservo.write(90);
    delay(50);
  } else if (inMsg == "c"){
    myservo.write(179);
    delay(50);
  }
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("inboundTopic", "Sono connesso");     
      // ... and resubscribe
      client.subscribe("inboundTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    ++value;
/*   esempio pubblicazione messaggio 
   snprintf (msg, 50, "Invio Sequenza Valori", value);
   Serial.print("Publish message: ");                                       Invio messaggi
    Serial.println(msg);
    client.publish("outTopic", msg);
*/
    temp(); 
    potentiometer_knob();   
    Serial.print("Publish temperature: ");
    Serial.println(t);
    client.publish("tempTopic", temperature_c);
    Serial.print("Publish humidity: ");
    Serial.println(h);
    client.publish("humTopic", humid);
    Serial.print("Publish potentiometer value: ");
    Serial.println(outputValue);
    client.publish("potTopic", pot);
  }
}

void temp(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // salva quando avviene l'ultimo aggiornamento della t/h
    previousMillis = currentMillis;
    // legge la temperatura in celsius
    int newT = dht.readTemperature();
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      //Serial.println(t);
      //Serial.print("°C");
      dtostrf(t,4,0,temperature_c);
    }
    //legge umidità
    int newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      // Serial.println(h);
      //Serial.println("%");
      dtostrf(h,2,0,humid);      
    }
  }  
 }

 void tone_connection(){
    tone(BUZZERPIN, 500);
    delay(1000);  
    tone(BUZZERPIN, 1000);
    delay(1000);
    digitalWrite(BUZZERPIN, LOW);
 }

 void potentiometer_knob(){
  sensorValue = analogRead(analogPin);
    // map it to the range of the PWM out
  outputValue = map(sensorValue, 0, 1024, 0, 255);
   // print the readings in the Serial Monitor
  dtostrf(outputValue,3,3,pot);
 }
