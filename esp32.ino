#define BLYNK_TEMPLATE_ID "TMPL6vLXhjgbd"
#define BLYNK_TEMPLATE_NAME "Test2"
#define BLYNK_AUTH_TOKEN "Ew1Xv6OxvjPvyvtg_uqUhlfbQQE8TVgL"

#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <InfluxDbClient.h>

DHT dht(5, DHT11);
unsigned long startTime;

#define INFLUXDB_URL "http://192.168.0.121:8086"
#define INFLUXDB_TOKEN "sKrQSAbkAoeVhRbrjTbdlWWNq2wsAAtjbwFbWEmFFzwxJyU2z47a2_zmg9odTBctDNlwQaoJLGqOKXTH58_VSg=="
#define INFLUXDB_ORG "cairo"
#define INFLUXDB_BUCKET "bucket1"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point sensor("dht_sensor");

WebServer server(80);

unsigned long previousMillis = 0;
const long interval = 10000;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  dht.begin();

  WiFi.begin("Cairo-Dlink", "cairobawah123");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

  Blynk.begin(BLYNK_AUTH_TOKEN, "Cairo-Dlink", "cairobawah123");
  pinMode(2, OUTPUT);

  sensor.addTag("device", "ESP32");
  if (client.validateConnection()) {
      Serial.println("Connected to InfluxDB");
  } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
  }

  server.on("/getdata", getData);  // Route for handling root
  server.on("/ledon", ledOn);
  server.on("/ledoff", ledOff);
  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();

  server.handleClient();
    
  unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
    
    Blynk.virtualWrite(V0, getElapsedTime());
    Blynk.virtualWrite(V2, getTemperature());
    Blynk.virtualWrite(V3, getHumidity());
    int ledState = getLedState();
    Serial.print("LED state: ");
    Serial.println(ledState);

    Serial.print("Temp: ");
    Serial.println(getTemperature());
    Serial.print("Hum: ");
    Serial.println(getHumidity());
    Serial.print("Time: ");
    Serial.println(getElapsedTime());

    Serial.println("Data written to Blynk");

    sensor.clearFields();
    sensor.addField("temperature", getTemperature());
    sensor.addField("humidity", getHumidity());
    sensor.addField("elapsed_time", getElapsedTime());

    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    } else {
      Serial.println("Data written to InfluxDB");
    }
  }
}

float getTemperature() {
    float temp = dht.readTemperature();
    if (isnan(temp)) {
        Serial.println("Failed to read temperature!");
    }
    return temp;
}

float getHumidity() {
    float hum = dht.readHumidity();
    if (isnan(hum)) {
        Serial.println("Failed to read humidity!");
    }
    return hum;
}

float getElapsedTime() {
    return (millis() - startTime) / 3600000.0;
}

int getLedState() {
    return digitalRead(2);
}

BLYNK_WRITE(V1) {
    int ledState = param.asInt();
    digitalWrite(2, ledState);
    Serial.print("LED state changed to: ");
    Serial.println(ledState);
}

void getData() {
    String message = "ESP32 Sensor Data:\n";
    message += "Temperature: " + String(getTemperature()) + "°C\n";
    message += "Humidity: " + String(getHumidity()) + "%\n";
    message += "Elapsed Time: " + String(getElapsedTime()) + " hrs\n";
    message += "LED State: " + String(getLedState()) + "\n";
    server.send(200, "text/plain; charset=utf-8", message);
}

void ledOn() {
    digitalWrite(2, HIGH);
    server.send(200, "text/plain", "Pin is ON");
}

void ledOff() {
    digitalWrite(2, LOW);
    server.send(200, "text/plain", "Pin is OFF");
}
