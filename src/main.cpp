#include <WiFiNINA.h>
#include <Arduino_LSM6DS3.h>

// Wi-Fi Configuration
const char *ssid = "ESP32S3_AP";      // Replace with your ESP32-S3 AP SSID
const char *password = "123456789";   // Replace with your ESP32-S3 AP password
const char *serverIP = "192.168.4.1"; // Replace with the ESP32-S3's IP address

WiFiClient client;

// Function Prototypes
void connectToWiFi();
void sendCommand(String command);
String lastReceivedString = "off";

void setup()
{
  Serial.begin(115200);

  // Initialize the accelerometer
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }

  // Connect to Wi-Fi
  connectToWiFi();
}

void loop()
{
  // Read accelerometer data
  float x, y, z;
  while (IMU.accelerationAvailable())
  {
    IMU.readAcceleration(x, y, z);

    // Determine the tilt direction
    String command = "off"; // Default command
    if (x < -0.5)
    {
      command = "up";
    }
    else if (x > 0.5)
    {
      command = "down";
    }
    else if (y < -0.5)
    {
      command = "right";
    }
    else if (y > 0.5)
    {
      command = "left";
    }

    if (lastReceivedString != command)
    {
      // Send the command to the ESP32-S3
      sendCommand(command);
      lastReceivedString = command;
      // Print the accelerometer data and command for debugging
      Serial.print("X: ");
      Serial.print(x);
      Serial.print(" | Y: ");
      Serial.print(y);
      Serial.print(" | Command: ");
      Serial.println(command);
    }
  }
}

void connectToWiFi()
{
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.begin(ssid, password) != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to Wi-Fi!");
}

void sendCommand(String command)
{
  if (client.connect(serverIP, 80))
  {
    // Send an HTTP GET request
    client.print("GET /");
    client.print(command);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(serverIP);
    client.println("Connection: close");
    client.println();

    // Wait for the response (optional)
    while (client.connected())
    {
      if (client.available())
      {
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }
    }

    client.stop(); // Close the connection
  }
  else
  {
    Serial.println("Failed to connect to ESP32-S3!");
  }
}
