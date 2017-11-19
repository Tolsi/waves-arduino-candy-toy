#include <ESP8266WiFi.h>

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#include<AccelStepper.h>
AccelStepper myStepper(8, 13, 14, 12, 16);

const int stepsPerRevolution = 4076;

const char* ssid = "";
const char* password = "";

const char* balanceJsonField = "balance";
const char* host = "nodes.wavesnodes.com";
const String address = "3P7CHn3nndASs6UqgUf9atBEgue7C4cANdY";

const unsigned long long price = 100000000;

DynamicJsonBuffer jsonBuffer;
unsigned long long balance = -1;
unsigned int candiesToGive = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  myStepper.setMaxSpeed(700.0);
  myStepper.setAcceleration(700.0);
  myStepper.setSpeed(700);
  myStepper.moveTo(myStepper.currentPosition() - stepsPerRevolution / 2);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
}


void loop()
{
  if (candiesToGive > 0) {
      if(myStepper.distanceToGo() == 0) {
          candiesToGive--;
          Serial.printf("[Candy was gived, %d left]\n", candiesToGive);
          myStepper.moveTo(myStepper.currentPosition() - stepsPerRevolution / 2);
      }
      myStepper.run();
    } else {
    WiFiClient client;
  
    Serial.printf("\n[Connecting to %s ... ", host);
    if (client.connect(host, 80))
    {
      Serial.println("connected]");
  
      Serial.println("[Sending a request]");
      client.print(String("GET /addresses/balance/" + address) + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n" +
                   "\r\n"
                  );
  
      Serial.println("[Response:]");
      while (client.connected())
      {
        if (client.available()) {
          JsonObject& root = jsonBuffer.parse(client);
          if (root.containsKey(balanceJsonField)){
            root.printTo(Serial);
            const unsigned long long newBalance = root[balanceJsonField];
            Serial.print("\nUpdated balance: ");
            printLLNumber(newBalance, 10);
            if (balance != -1) {
              int candies = (newBalance - balance) / price;
              candiesToGive += candies;
            }
            balance = newBalance;
          }
        }
      }
      client.stop();
      Serial.println("\n[Disconnected]");
    }
    else
    {
      Serial.println("connection failed!]");
      client.stop();
    }
    delay(5000);
  }
}

void printLLNumber(unsigned long long n, uint8_t base)
{
  unsigned char buf[16 * sizeof(long)]; // Assumes 8-bit chars. 
  unsigned long long i = 0;

  if (n == 0) {
    Serial.print('0');
    return;
  } 

  while (n > 0) {
    buf[i++] = n % base;
    n /= base;
  }

  for (; i > 0; i--)
    Serial.print((char) (buf[i - 1] < 10 ?
      '0' + buf[i - 1] :
      'A' + buf[i - 1] - 10));
}

