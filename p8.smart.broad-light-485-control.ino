#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Connection infomation
char* ssid = "Spark";
char* password = "laputalpt";
char* chip_id = "p8_404_lc_0001";
int bps = 9600;

#define mqtt_server  "pipa.joinp8.com"
#define mqtt_server_port 1883
#define mqtt_user "p8iot"
#define mqtt_password "fd3sak2v6"

WiFiClient espClient;
PubSubClient client(espClient);

String hexToString(uint8_t value) {
  return value < 0x0F ? "0" + String(value, HEX) : String(value, HEX);
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_server_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(chip_id, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(chip_id);
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


String commnicate485(uint8_t code[], size_t length) {
  uint8_t result[20];
  Serial.write(code, length);
  Serial.readBytes(result, 20);
  String value = "";
  for (int i = 0; i < 20; i++) {
    value += hexToString(result[i]);
  }
  value.trim();
  return value;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String payloadData = "";
  for (int i = 0; i < length; i++) {
    payloadData += String((char)payload[i]);
  }
  //the message received.
  String message = payloadData.substring(0, payloadData.lastIndexOf("@"));
  //the message who send it.
  String sender = payloadData.substring(payloadData.lastIndexOf("@") + 1, payloadData.length()) + " set device";

  //Serial.println(message);
  //Serial.println(sender);

  uint8_t code[message.length()];
  for (int i = 0; i < message.length(); i = i + 2) {
    String temp1 = String((char)message[i]);
    String temp2 = String((char)message[i + 1]);
    code[i / 2] = strtoul((temp1 + temp2).c_str(), 0, 16);
  }

  //显示code的内容
  //for (int i = 0; i < 20; i++) {
  //  Serial.print(code[i], HEX);
  //  Serial.print(" ");
  //}

  //获取485的feedback
  String response = commnicate485(code, message.length() / 2);
  String result = "{\"type\": \"BroadLC485\",\"data\":\"";
  result.concat(response);
  result.concat("\"}");
  Serial.println(result.c_str());
  client.publish(sender.c_str(), result.c_str());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
