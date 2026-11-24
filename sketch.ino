

#define BLYNK_TEMPLATE_ID   "TMPL3e4SquoDN"
#define BLYNK_TEMPLATE_NAME "Wireless Sensor Mesh"
#define BLYNK_AUTH_TOKEN    "34Lmd35pJJrm4mZCjiHQXRUZPUn3Xazb"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <ThingSpeak.h>
#include <BlynkSimpleEsp32.h>

const char* ssid     = "Wokwi-GUEST";
const char* password = "";

// HiveMQ
const char* mqttServer = "broker.hivemq.com";
const int   mqttPort   = 1883;
const char* TOPIC_ALL  = "wsm_mesh/#";

// ThingSpeak
unsigned long channelID = 3508336;                      // your channel ID
const char* thingSpeakAPIKey = "839K785FKZK5XF5K";

// Google Sheets
const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbw85rjDLLydmwh86DZO6-m2zqRV-6SNgNs03Eo1_EiS09Do0OojoK-1fN_bf48OOMP9/exec";

WiFiClient espClient;
PubSubClient mqtt(espClient);
WiFiClient tsClient;
BlynkTimer timer;

float temp[3], hum[3];
bool  hasData[3] = {false, false, false};

// ---------- MQTT message received ----------
void onMessage(char* topic, byte* payload, unsigned int len) {
  char buf[32];
  len = min(len, (unsigned int)31);
  memcpy(buf, payload, len);
  buf[len] = '\0';

  int n = topic[strlen(topic) - 1] - '1';   // node1->0, node2->1, node3->2
  float t, h;
  if (n >= 0 && n < 3 && sscanf(buf, "%f,%f", &t, &h) == 2) {
    temp[n] = t;
    hum[n]  = h;
    hasData[n] = true;
    Serial.printf("Received [%s]: %.1f C, %.1f %%\n", topic, t, h);
  }
}

// ---------- MQTT connect ----------
void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to HiveMQ...");
    String id = "wsm-gateway-" + String(random(0xffff), HEX);
    if (mqtt.connect(id.c_str())) {
      mqtt.subscribe(TOPIC_ALL);
      Serial.println("connected, subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

// ---------- Upload to cloud ----------
void sendToCloud() {
  Serial.println("---- Uploading ----");

  int fieldsSet = 0;

  for (int i = 0; i < 3; i++) {
    if (!hasData[i]) continue;

    // Blynk
    Blynk.virtualWrite(i * 2, temp[i]);
    Blynk.virtualWrite(i * 2 + 1, hum[i]);

    // ThingSpeak fields
    ThingSpeak.setField(i * 2 + 1, temp[i]);
    ThingSpeak.setField(i * 2 + 2, hum[i]);

    fieldsSet++;
  }

  if (fieldsSet == 0) {
    Serial.println("No node data yet, skipping upload");
    return;
  }

  Serial.println("Blynk: sent");

  int ts = ThingSpeak.writeFields(channelID, thingSpeakAPIKey);
  Serial.print("ThingSpeak: ");
  Serial.println(ts);
    // Google Sheets
  for (int i = 0; i < 3; i++) {
    if (!hasData[i]) continue;

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(10000);

    String url = String(googleScriptURL) + "?node=node" + String(i + 1) +
                 "&temperature=" + String(temp[i], 1) +
                 "&humidity="    + String(hum[i], 1);

    http.begin(url);
    int code = http.GET();
    Serial.printf("Sheets node%d: %d (%s)\n", i + 1, code, http.errorToString(code).c_str());
    Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
    http.end();
    delay(500);
  }
  
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  Serial.println("Starting Gateway...");

  WiFi.begin(ssid, password, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  ThingSpeak.begin(tsClient);

  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setKeepAlive(60);
  mqtt.setCallback(onMessage);

  timer.setInterval(16000L, sendToCloud);
}

// ---------- Loop ----------
void loop() {
  Blynk.run();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();
  timer.run();
}


#define BLYNK_TEMPLATE_ID   "TMPL3e4SquoDN"
#define BLYNK_TEMPLATE_NAME "Wireless Sensor Mesh"
#define BLYNK_AUTH_TOKEN    "34Lmd35pJJrm4mZCjiHQXRUZPUn3Xazb"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <PubSubClient.h>
#include <ThingSpeak.h>
#include <BlynkSimpleEsp32.h>

const char* ssid     = "Wokwi-GUEST";
const char* password = "";

// HiveMQ
const char* mqttServer = "broker.hivemq.com";
const int   mqttPort   = 1883;
const char* TOPIC_ALL  = "wsm_mesh/#";

// ThingSpeak
unsigned long channelID = 3508336;                      // your channel ID
const char* thingSpeakAPIKey = "839K785FKZK5XF5K";

WiFiClient espClient;
PubSubClient mqtt(espClient);
WiFiClient tsClient;
BlynkTimer timer;

float temp[3], hum[3];
bool  hasData[3] = {false, false, false};

// ---------- MQTT message received ----------
void onMessage(char* topic, byte* payload, unsigned int len) {
  char buf[32];
  len = min(len, (unsigned int)31);
  memcpy(buf, payload, len);
  buf[len] = '\0';

  int n = topic[strlen(topic) - 1] - '1';   // node1->0, node2->1, node3->2
  float t, h;
  if (n >= 0 && n < 3 && sscanf(buf, "%f,%f", &t, &h) == 2) {
    temp[n] = t;
    hum[n]  = h;
    hasData[n] = true;
    Serial.printf("Received [%s]: %.1f C, %.1f %%\n", topic, t, h);
  }
}

// ---------- MQTT connect ----------
void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to HiveMQ...");
    String id = "wsm-gateway-" + String(random(0xffff), HEX);
    if (mqtt.connect(id.c_str())) {
      mqtt.subscribe(TOPIC_ALL);
      Serial.println("connected, subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

// ---------- Upload to cloud ----------
void sendToCloud() {
  Serial.println("---- Uploading ----");

  int fieldsSet = 0;

  for (int i = 0; i < 3; i++) {
    if (!hasData[i]) continue;

    // Blynk
    Blynk.virtualWrite(i * 2, temp[i]);
    Blynk.virtualWrite(i * 2 + 1, hum[i]);

    // ThingSpeak fields
    ThingSpeak.setField(i * 2 + 1, temp[i]);
    ThingSpeak.setField(i * 2 + 2, hum[i]);

    fieldsSet++;
  }

  if (fieldsSet == 0) {
    Serial.println("No node data yet, skipping upload");
    return;
  }

  Serial.println("Blynk: sent");

  int ts = ThingSpeak.writeFields(channelID, thingSpeakAPIKey);
  Serial.print("ThingSpeak: ");
  Serial.println(ts);
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  Serial.println("Starting Gateway...");

  WiFi.begin(ssid, password, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  ThingSpeak.begin(tsClient);

  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setKeepAlive(60);
  mqtt.setCallback(onMessage);

  timer.setInterval(16000L, sendToCloud);
}

// ---------- Loop ----------
void loop() {
  Blynk.run();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();
  timer.run();
}