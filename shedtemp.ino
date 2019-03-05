#include <ESP8266WiFi.h> // Wi-Fi (single ssid)
#include <ESP8266mDNS.h> // mDNS
#include <ESP8266WebServer.h> // Web server
#include <ezTime.h> // NTP Support

#include "credentials.h" // WiFi Network credentials

// Setup the web server
ESP8266WebServer server(80);  // Create a webserver object that listens for HTTP request on port 80

void setup() {
  Serial.begin(115200);  // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
	        
  WiFi.begin(ssid, password);  // Connect to the network
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

  // Start the mDNS responder for <hostname>.local - see credentials.h
  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");

  // NTP Sync
  setServer(ntpServer);
  waitForSync();
  Serial.println("UTC: " + UTC.dateTime());
  
  // function prototypes for HTTP handlers
  void handleRoot();
  void handleNotFound();
  // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/", handleRoot);
  server.on("/metrics", handlePrometheus);
  server.onNotFound(handleNotFound); // 404 Handler
  // Start the web server
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();  // Listen for HTTP requests from clients
}

void handleRoot() {
  server.send(200, "text/plain", "Hello world!  The time is "+UTC.dateTime(ISO8601));  // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

// Prometheus metrics endpoint
void handlePrometheus(){
  // See https://prometheus.io/docs/instrumenting/exposition_formats/
  String payload;
  payload += "# HELP Temperature probes, in degrees C\n";
  payload += "# TYPE temperature gauge\n";
  payload += "temperature{probeid=\"1\"} 12.5\n";
  payload += "temperature{probeid=\"2\"} 12.5\n";

  payload += "# HELP Humidity probes\n";
  payload += "# TYPE humidity gauge\n";
  payload += "humidity{probeid=\"3\"} 75\n";

  server.send(200, "text/plain; version=0.0.4", payload);
}
