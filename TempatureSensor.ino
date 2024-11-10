// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
#else
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>S
#endif
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"

// DHT sensor pin
#define DHTPIN 5 

// Data wire is connected to GPIO 4
#define ONE_WIRE_BUS 4

#define DHTTYPE DHT22

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Variables to store temperature values
String rootTemperatureF = "";
String rootTemperatureC = "";
String airTemperatureF = "";
String airTemperatureC = "";
String humidity = "";

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 30000;

// Replace with your network credentials
const char* ssid = "WIFIWizards";
const char* password = "$p3ll$@ndP0t10n$";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readRootTemperatureC() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);

  if(tempC == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    return "Error";
  } else {
    Serial.print("Root Temperature Celsius: ");
    Serial.println(tempC); 
  }
  return String(tempC);
}

String readRootTemperatureF() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  float tempF = sensors.getTempFByIndex(0);

  if(int(tempF) == -196){
    Serial.println("Failed to read from DS18B20 sensor");
    return "Error";
  } else {
    Serial.print("Root Temperature Fahrenheit: ");
    Serial.println(tempF);
  }
  return String(tempF);
}

String readAirTemperatureF() {
  float tempF = dht.readTemperature(true);

  if(isnan(tempF)){
    Serial.println("Failed to read from DHT sensor in Fahrenheit");
    return "Error";
  } else {
    Serial.print("Air Temperature Fahrenheit: ");
    Serial.println(tempF);
  }
  return String(tempF);
}

String readAirTemperatureC() {
  float tempC = dht.readTemperature();

  if(isnan(tempC)){
    Serial.println("Failed to read from DHT sensor in Celsius");
    return "Error";
  } else {
    Serial.print("Air Temperature Celsius: ");
    Serial.println(tempC);
  }
  return String(tempC);
}

String readAirHumidity() {
  float humidity = dht.readHumidity();

  if(isnan(humidity)){
    Serial.println("Failed to read from DHT sensor humidity");
    return "Error";
  } else {
    Serial.print("Humidity: ");
    Serial.println(humidity);
  }
  return String(humidity);
}

const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE HTML>
<html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link href="http://fonts.cdnfonts.com/css/vt323" rel="stylesheet">
      <style>

        @import url('http://fonts.cdnfonts.com/css/vt323');

        body {
            background-color: black;
            color: rgb(0,255,102);
            font-family: 'VT323', sans-serif;
        }

        .header {
            text-align: center;
        }

            .header h1 {
                font-size: 4em;
            }

        .container {
            display: flex;
            justify-content: space-evenly;
            flex-wrap: wrap;
            margin: 3px 5px;
            border: 2px solid rgb(0,255,102);
            padding: 20px 0;
        }
        .child {
            width: 300px;
            text-align: center;
            padding-top: 20px;
            font-size: 2em;

        }
        .warn-clr {
          color: yellow;
        }
         .error-clr {
          color: red;
        }

      </style>
    </head>
    <body>
      <div class="header">
        <h1>Green House</h1>
      </div>
      <div class="container">

        <div class="child">
          <h2>Air Temperature</h2>
          <p>
              <span id="airtemperaturec"></span>
              <sup class="units">&deg;C</sup>
          </p>
        </div>
        
        <div class="child">
          <h2>Humidity</h2>
          <span id="humidity"></span>
        </div>
        
        <div class="child">
          <h2>Root Temperature</h2>
            <p>
              <span id="roottemperaturec"></span>
              <sup class="units">&deg;C</sup>
            </p>
        </div>
    </body>
    <script>

    window.onload = () => {
      updateSensorData();
      setInterval(() => updateSensorData(), 10000);
    }

    async function updateSensorData() {

      const C_LOWER_IDEAL = 20;
      const C_UPPER_IDEAL = 25;
      const C_LOWER_BOUND = 15;
      const C_UPPER_BOUND = 30;

      const HUMIDITY_LOWER_BOUND = 30;
      const HUMIDITY_UPPER_BOUND = 80;
      const HUMIDITY_LOWER_IDEAL = 40;   
      const HUMIDITY_UPPER_IDEAL = 70;

      const response = await fetch("/sensorData");
      let obj = await response.json();
      console.log(obj);
      
      let roottemperaturec = document.getElementById("roottemperaturec");
      roottemperaturec.innerText = obj.rootTemperatureC;
      setBoundaryColors(roottemperaturec, obj.rootTemperatureC, C_LOWER_IDEAL, C_UPPER_IDEAL, C_LOWER_BOUND, C_UPPER_BOUND) 
      
      let airtemperaturec = document.getElementById("airtemperaturec");
      airtemperaturec.innerText = obj.airTemperatureC;
      setBoundaryColors(airtemperaturec, obj.airTemperatureC, C_LOWER_IDEAL, C_UPPER_IDEAL, C_LOWER_BOUND, C_UPPER_BOUND) 
      
      let humidity = document.getElementById("humidity");
      humidity.innerText = obj.humidity;
      setBoundaryColors(humidity, obj.humidity, HUMIDITY_LOWER_IDEAL, HUMIDITY_UPPER_IDEAL, HUMIDITY_LOWER_BOUND, HUMIDITY_UPPER_BOUND) 
    }

    function setBoundaryColors(elem, value, lowerIdeal, upperIdeal, lowerBound, upperBound) {
      elem.classList.remove("warn-clr");
      elem.classList.remove("error-clr");
      
      if ((value < lowerIdeal && value > lowerBound) || (value > upperIdeal && value < upperBound)) {
        elem.classList.add("warn-clr");
      } else if (value < lowerBound || value > upperBound){
        elem.classList.add("error-clr");
      }
    }
    </script>
    </html>)rawliteral";

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println();
  
  // Start up the DS18B20 library
  sensors.begin();

  // Root
  rootTemperatureC = readRootTemperatureC();
  rootTemperatureF = readRootTemperatureF();

  // Air
  airTemperatureC = readAirTemperatureC();
  airTemperatureF = readAirTemperatureF();
  humidity = readAirHumidity();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.setDebugOutput(true);
    
    switch(WiFi.status()) {
      case 0:
        Serial.printf("WL_IDLE_STATUS");
        break;
      case 1:
        Serial.printf("WL_NO_SSID_AVAIL");
        break;
      case 4:
        Serial.printf("WL_CONNECT_FAILED");
        break;
      case 6:
        Serial.printf("WL_CONNECT_WRONG_PASSWORD");
        break;
      case 7:
        Serial.printf("WL_DISCONNECTED");
        break;
    }
    Serial.println();
    WiFi.printDiag(Serial);
    Serial.println();
  }
  Serial.println();

  // Start DHT sensor
  dht.begin();
  
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
  server.on("/sensorData", HTTP_GET, [](AsyncWebServerRequest *request){

    String jsonData = "{\"rootTemperatureC\": " +  rootTemperatureC + "," 
      + "\"airTemperatureC\": " + airTemperatureC + "," 
      + "\"airTemperatureF\": " + airTemperatureF + "," 
      + "\"humidity\": " + humidity + "}";
      
    request->send_P(200, "text/plain", jsonData.c_str());
  });
  
  // Start server
  server.begin();
}
 
void loop(){
  
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
     // Root
    rootTemperatureC = readRootTemperatureC();
    rootTemperatureF = readRootTemperatureF();
  
    // Air
    airTemperatureC = readAirTemperatureC();
    airTemperatureF = readAirTemperatureF();
    humidity = readAirHumidity();
  
    lastTime = millis();
  }  
}
