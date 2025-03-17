#include <WiFiNINA.h>
#include <Arduino_LSM6DS3.h>

// Wi-Fi Configuration
const char *ssid = "ESP32S3_AP";      // Replace with your ESP32-S3 AP SSID
const char *password = "123456789";   // Replace with your ESP32-S3 AP password
const char *serverIP = "192.168.4.1"; // Replace with the ESP32-S3's IP address

WiFiClient client;

// Function Prototypes
void connectToWiFi();
void sendCommand(const char *command);
void handleIMUData();

const char *lastSentCommand = "off";
volatile bool isSending = false; // Mutex-like variable to prevent race conditions

void setup()
{
  Serial.begin(115200);

  // Initialize the accelerometer
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ; // Halt execution if IMU initialization fails
  }

  // Connect to Wi-Fi
  connectToWiFi();
}

void loop()
{
  handleIMUData();
}

void connectToWiFi()
{
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 5)
  {
    Serial.print(".");
    delay(1000);
    retryCount++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nConnected to Wi-Fi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\nFailed to connect to Wi-Fi. Restarting...");
    NVIC_SystemReset(); // Restart the device if Wi-Fi connection fails
  }
}

void handleIMUData()
{
  float x, y, z;

  while (IMU.accelerationAvailable())
  {
    IMU.readAcceleration(x, y, z);

    // Determine the tilt direction
    const char *command = "off"; // Default command
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

    if (lastSentCommand != command)
    {
      // Send the command to the ESP32-S3
      sendCommand(command);
      lastSentCommand = command;
    }
    delay(50);
  }
}

void sendCommand(const char *command)
{
  // Check if another command is being sent
  if (isSending)
  {
    Serial.println("Command sending in progress. Skipping...");
    return;
  }

  isSending = true; // Lock the mutex

  if (client.connect(serverIP, 80))
  {
    // Send an HTTP GET request
    client.println(command);
    client.stop(); // Close the connection
    Serial.println("Sent " + String(command));
  }
  else
  {
    Serial.println("Failed to connect to ESP32-S3!");
  }

  isSending = false; // Unlock the mutex
}
