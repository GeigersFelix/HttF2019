#include <WiFi.h>
#include <PubSubClient.h>
#include <LedControl.h>

// Update these with values suitable for your network.

const char* ssid = "EULE-Gast";
const char* password = "@EULE_Zukunft!";
const char* mqtt_server = "192.168.1.89";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

int taster=26;
int tv=0;
int richtung_alt=1;

LedControl lc=LedControl(12,14,15,1);

byte pfeilhoch[8]=
{
  B00011000,
  B00111100,
  B01111110,
  B11111111,
  B00011000,
  B00011000,
  B00011000,
  B00011000
};

byte pfeilraus[8]=
{
  B00001000,
  B00001100,
  B00001110,
  B11111111,
  B11111111,
  B00001110,
  B00001100,
  B00001000
};

byte rettungsgasse[8]=
{
  B00000111,
  B00000011,
  B11100101,
  B11001000,
  B10100000,
  B00010000,
  B00000000,
  B00000000
};

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

  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  //String payloadS(payload);
  String topicS(topic);
  char cpayload[20];
  for (int i = 0; i < length; i++) {
    Serial.print(payload[i]);
    cpayload[i]=payload[i];
  }

  String payloadS(cpayload);
  
  if(topicS.equals("Schild2"))            //topic, das vom vorherigen Schild beschrieben wird
  {
    //payloadS.toInt();
    Serial.println();
    Serial.print("Received: ");
    Serial.println(payloadS);
    
    if(payloadS=="1")
    {
      richtung_alt=1;
      Serial.print("1 erkannt");
    }
    else if(payloadS=="0")
    {
      richtung_alt=2;
      Serial.print("0 erkannt");
    }
    else
    {
      Serial.print("Fehler");
      Serial.print(payloadS);
      
    }
  }
  
  Serial.println();
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
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("Schild2");                             //Topic, die überwacht wird
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(taster, INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(digitalRead(taster) == HIGH)
  {
    tv++;
    //Serial.println("Taster HIGH");
  }
  else
  {
    tv=0;
  }

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    /*
    ++value;
    snprintf (msg, 50, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
    */    
    
    if(tv>1000)   //Wenn Sensor Stau erkennt
    {
      snprintf (msg, 50, "1");
      Serial.print("Sende Nachricht: ");
      Serial.println(msg);
      client.publish("Schild1", msg);  //Schildname
      
      for(int i=0; i<8; i++)
      {
        lc.setRow(0, i, pfeilraus[i]);
      }
    }
    else  //wenn kein Stau
    {
      snprintf (msg, 50, "0");
      Serial.print("Sende Nachricht: ");
      Serial.println(msg);
      client.publish("Schild1", msg);  //Schildname
      
      switch(richtung_alt)
      {
        case 1:
          for(int i=0; i<8; i++)
          {
            lc.setRow(0, i, pfeilhoch[i]);
          }
          break;
        case 2:
          for(int i=0; i<8; i++)
          {
            lc.setRow(0, i, pfeilraus[i]);
          }
          break;
      }
    }
  }
}
