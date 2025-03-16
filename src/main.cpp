#include <WiFiNINA.h>
#include <Arduino_LSM6DS3.h> // Library for the acceleration/accelerometer

// Wi-Fi credentials (match those on the ESP32-S3)
const char *ssid = "ESP32-AccessPoint"; // Wi-Fi SSID
const char *password = "password123";   // Wi-Fi password

const char *host = "192.168.4.1"; // ESP32-S3 Access Point IP
const int port = 12345;           // Port number of the server

WiFiClient client;

void connectToWiFi()
{
  Serial.println("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, password);
    int count = 0;
    while (WiFi.status() != WL_CONNECTED && count < 10)
    {
      delay(1000);
      Serial.print(".");
      count++;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Wi-Fi connected!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println("Failed to connect. Retrying...");
    }
  }

  // Connect to the ESP32-S3 server
  if (client.connect(host, port))
  {
    Serial.println("Connected to the server!");
  }
  else
  {
    Serial.println("Failed to connect to the server!");
  }
}

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial)
    ;

  // Initialize Wi-Fi connection
  connectToWiFi();

  // Initialize IMU
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }
  Serial.println("IMU initialized.");
}

void loop()
{
  // Send the pattern to the ESP32-S3
  while (client.connected() && IMU.accelerationAvailable())
  {
    // Variables to store acceleration readings
    float x, y, z;

    // Read acceleration values
    IMU.readAcceleration(x, y, z);
    String pattern;
    if (x > 0.5)
    {
      pattern = "down";
      client.println(pattern);
      client.flush();
      Serial.println("Sent: " + pattern);
    }
    else if (x < -0.5)
    {
      pattern = "up";
      client.println(pattern);
      client.flush();
      Serial.println("Sent: " + pattern);
    }
    else if (y > 0.5)
    {
      pattern = "left";
      client.println(pattern);
      client.flush();
      Serial.println("Sent: " + pattern);
    }
    else if (y < -0.5)
    {
      pattern = "right";
      client.println(pattern);
      client.flush();
      Serial.println("Sent: " + pattern);
    }
    else if (x <= 0.5 && x >= -0.5 && y <= 0.5 && y >= -0.5)
    {
      pattern = "off";
      client.println(pattern);
      client.flush();
      Serial.println("Sent: " + pattern);
    }
    delay(10);
  }
  if (!client.connected())
  {
    Serial.println("Disconnected from the server, reconnecting...");
    connectToWiFi();
  }
}
