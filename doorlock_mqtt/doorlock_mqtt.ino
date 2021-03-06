//#ifdef ESP32
//#include <WiFi.h>
//#else
//#include <ESP8266WiFi.h>
//#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <Keypad.h>
//#include <WiFi.h>
#include <PubSubClient.h>

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4// three columns#define relay 26
#define buzzer 27
#define debug(x)
#define debugln(x)
const int relay = 26;
byte pin_column[COLUMN_NUM] = {23, 22, 3, 21}; // GIOP18, GIOP5, GIOP17, GIOP16 connect to the row pins
byte pin_rows[ROW_NUM] = {19, 18, 5,17};  // GIOP4, GIOP0, GIOP2 connect to the column pins


char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3','A'},
  {'4', '5', '6','B'},
  {'7', '8', '9','C'},
  {'*', '0', '#','D'}
};

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

const String password = "7777"; // change your password here
String input_password;


const char* ssid = "";
const char* passwd = "";
const char* mqtt_server = "";

// Initialize Telegram BOT
#define BOTtoken ""  // your Bot Token (Get from Botfather)
//Enter either group ID or the chat ID
#define CHAT_ID ""
WiFiClientSecure botclient;
UniversalTelegramBot bot(BOTtoken, botclient);
// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
bool relayState = LOW;
bool buzzerState = LOW;
// Handle new receive messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
      }
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/open to open door \n";
      welcome += "/close to close door \n";
      welcome += "/state to request current door state \n";
      bot.sendMessage(chat_id, welcome, "");
    }
    if (text == "/open") {
      bot.sendMessage(chat_id, "door state set to open", "");
      relayState = HIGH;
      digitalWrite(relay, relayState);
      delay(5000);
      relayState = LOW;
      digitalWrite(relay, relayState);
    }
    if (text == "/close") {
      bot.sendMessage(chat_id, "door state set to close", "");
      relayState = LOW;
      digitalWrite(relay, relayState);
    }

    if (text == "/state") {
      if (digitalRead(relay)){
        bot.sendMessage(chat_id, "door is open", "");}
      else
        //bot.sendMessage(chat_id, "blueLED is OFF", "");
       {
        bot.sendMessage(chat_id, "door is closed", "");
       }

    }
}
}

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passwd);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // open the lock if an 1 was received as first character

 if ((char)payload[0] == '0') {

    digitalWrite(relay, LOW);
   digitalWrite(buzzer,HIGH);
    client.publish("outDoor","closed");
   delay(100);
    digitalWrite(buzzer,LOW);
    delay(100);
    digitalWrite(buzzer,HIGH);
    delay(100);
    digitalWrite(buzzer,LOW);

  }

  else if ((char)payload[0] == '1'){
   digitalWrite(relay,HIGH);
   digitalWrite(buzzer, HIGH);
   client.publish("outDoor","opened");
   delay(1000);
   digitalWrite(buzzer, LOW);
   delay(5000);
   digitalWrite(relay,LOW);


  }
   else {
    digitalWrite(relay, LOW);
    digitalWrite(buzzer, LOW);
     client.publish("outDoor","closed");
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
      client.publish("outDoor", "Start ");
      // ... and resubscribe
      client.subscribe("inDoor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup(){
  Serial.begin(9600);
  //#ifdef ESP8266
    botclient.setInsecure();
 //#endif
  input_password.reserve(32); // maximum input characters is 33, change if needed
   configTime(0, 0, "pool.ntp.org");
   debugln("\n\nStarting program");
   setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


   pinMode(relay,OUTPUT);
   pinMode(buzzer,OUTPUT);
   //digitalWrite(relay,LOW);
}
 // WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);

 // while (WiFi.status() != WL_CONNECTED) {
    //delay(1000);
    //Serial.println("Connecting to WiFi..");
  //}

void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.println(key);

    if (key == '*') {
      input_password = ""; // clear input password
    } else if (key == '#') {
      if (password == input_password) {
        Serial.println("The password is correct, ACCESS GRANTED!");
        digitalWrite(relay,HIGH);
        delay(3000);
        digitalWrite(relay,LOW);
        client.publish("outDoor","opened");

      } else {
        Serial.println("The password is incorrect, ACCESS DENIED!");
        digitalWrite(relay, LOW);
         digitalWrite(buzzer, HIGH);
         delay(500);
         digitalWrite(buzzer,LOW);
         client.publish("outDoor","closed");
      }

      input_password = ""; // clear input password
    } else {
      input_password += key; // append new character to input password string
    }
  }

if (!client.connected())  // Reconnect if connection is lost
  {
    reconnect();
  }
  client.loop();
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

}