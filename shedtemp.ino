#include <ESP8266WiFi.h>       // Wi-Fi (single ssid)
#include <ESP8266mDNS.h>       // mDNS for discovery
#include <ESP8266WebServer.h>  // Web server
#include <OneWire.h>           // One Wire Sensors
#include <DallasTemperature.h> // Temperature conversion

#include "credentials.h" // WiFi Network credentials

// IO Pin definitions:
// D0  = GPIO16;   D1  = GPIO5;    D2  = GPIO4;    D3  = GPIO0;
// D4  = GPIO2;    D5  = GPIO14;   D6  = GPIO12;   D7  = GPIO13;
// D8  = GPIO15;   D9  = GPIO3;    D10 = GPIO1;
//
#define PROBE_TEMP 2     // I2C Bus for the temperature probes

#define TEMPERATURE_PRECISION 8

// Set up the temperature probes
OneWire one_wire(PROBE_TEMP);
DallasTemperature sensors(&one_wire);
DeviceAddress probes[20];
int num_probes = 0;
unsigned long lastTempRequest = 0;
float temperature = 0.0;

// Setup the web server
ESP8266WebServer server(80);  // Create a webserver object that listens for HTTP request on port 80

void setup() {
  Serial.begin(115200);  // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  // Set up the sensors
  sensors.begin();
  // sensors.setResolution(TEMPERATURE_PRECISION);

  // Find our sensors, use num_probes as a handy counter.
  Serial.println("Searching for probes...");
  one_wire.reset_search();
  while (one_wire.search(probes[num_probes])) {
    Serial.print("+");
    num_probes++;
  }
  sensors.setWaitForConversion(false);

  sensors.requestTemperatures(); 
  Serial.println("\nDone");

  float tempC = sensors.getTempCByIndex(0);

  // Disable AP mode
  WiFi.mode(WIFI_STA);
  WiFi.softAPdisconnect(true);

  // Connect to the network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  // Serial debug info
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address:\t");
  Serial.println(WiFi.macAddress());

  // Convert "00:01:02:03:04:05" to 000102030405,
  String hostname = WiFi.macAddress().substring(0,2) +
    WiFi.macAddress().substring(3,5) +
    WiFi.macAddress().substring(6,8) +
    WiFi.macAddress().substring(9,11) +
    WiFi.macAddress().substring(12,12) +
    WiFi.macAddress().substring(15,17);
    
  Serial.print("MAC address converted:\t");
  Serial.println(hostname);

  // Start the mDNS responder for <hostname>.local
  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
  }
  else {
    Serial.println("mDNS responder started OK");
  }
  // Make us discoverable
  if (MDNS.addService("temp-probe", "tcp", 80)) {
    Serial.println("mDNS SVC added");
  }
  else {
    Serial.println("mDNS SVC failed");
  }
  
  // function prototypes for HTTP handlers
  void handleRoot();
  void handleNotFound();

  // HTTP Endpoints
  server.on("/", handleRoot);
  server.on("/metrics", handlePrometheus);
  // 404 Handler
  server.onNotFound(handleNotFound);

  // Start the web server
  server.begin();
  Serial.print("HTTP server started, listening on http://");
  Serial.print(hostname);
  Serial.println(".local:80");
}

void loop(void){
  // Update mDNS advertisments
  MDNS.update();
  
  // Handle HTTP requests
  server.handleClient();

  // Poll the temperature sensors every 5 seconds
  if (millis() - lastTempRequest >= 5000) // waited long enough??
    {
    sensors.requestTemperatures(); 
    lastTempRequest = millis(); 
    }
}

void handleRoot() {
  server.send(200, "text/html", "Temperature data available at <a href=\"/metrics\">/metrics</a>");
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}

// Prometheus metrics endpoint
void handlePrometheus(){
  // See https://prometheus.io/docs/instrumenting/exposition_formats/
  String payload;
  payload += "# HELP Temperature probes, in degrees C\n";
  payload += "# TYPE temperature gauge\n";

  // add the temp to the metrics output  
  for (int i = 0; i < num_probes; i++) {
    payload += "temperature{probeid=\"";
    payload += probeID(probes[i]);
    payload += "\"} ";
    payload += String(sensors.getTempC(probes[i]), DEC) + "\n";

    Serial.print(probeID(probes[i]) + ": ");
    Serial.println(sensors.getTempC(probes[i]));
  }

  server.send(200, "text/plain; version=0.0.4", payload);
}

String probeID(DeviceAddress device_address) {
  String buffer;
  // Device address is 8 bytes, iterate over them.
  for (byte i = 0; i < 8; i++) {
    if (device_address[i] < 16) {
      buffer += 0;
    }
    buffer += String(device_address[i],HEX);
  }
  return buffer;
}
