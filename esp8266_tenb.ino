#include <TM1637.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";

int8_t digits[4];;
char price[5];

#define CLK 2
#define DIO 0
TM1637 tm1637(CLK, DIO);

void setup() {
  // read last price from EEPROM and display it
  // so our display isn't blank while we're getting the new price
  EEPROM.begin(4);
  digits[0] = EEPROM.read(0);
  digits[1] = EEPROM.read(1);
  digits[2] = EEPROM.read(2);
  digits[3] = EEPROM.read(3);
  tm1637.set();
  tm1637.init();
  tm1637.display(digits);

  // spam serial port with some debug stuff
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  WiFiClientSecure client;
  Serial.println("connecting to api.iextrading.com");
  if (!client.connect("api.iextrading.com", 443)) {
    Serial.println("connection failed");
    return;
  }

  Serial.println("requesting URL: /1.0/stock/tenb/price");
  client.print("GET /1.0/stock/tenb/price HTTP/1.1\r\n" 
               "Host: api.iextrading.com\r\n" 
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  // assumes price is < $100, hopefully have to update this at some point :)
  client.readBytes(price, 5);

  digits[0] = price[0] - '0';
  digits[1] = price[1] - '0';
  digits[2] = price[3] - '0';
  digits[3] = price[4] - '0';

  EEPROM.write(0, digits[0]);
  EEPROM.write(1, digits[1]);
  EEPROM.write(2, digits[2]);
  EEPROM.write(3, digits[3]);
  EEPROM.commit();

  // turn on the : to indicate we have updated the price from the internet
  // and are no longer using the value stored in the eeprom
  tm1637.point(POINT_ON);
  tm1637.display(digits);

  Serial.println("TENB price:");
  Serial.println(price);
  Serial.println("closing connection, going into deep sleep for 10 minutes");

  // sleep for 10 minutes (600 seconds)
  ESP.deepSleep(600*1000000);
  delay(1);
}

void loop() {
}
