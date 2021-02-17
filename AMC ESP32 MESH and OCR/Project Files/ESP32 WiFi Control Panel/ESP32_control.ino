/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <SPIFFS.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <Hash.h>
  #include <FS.h>
#endif
#include <ESPAsyncWebServer.h>



// SSID and PASSWORD of the ESP32 access point
const char *ssid = "MyESP32AP";
const char *password = "testpassword";

AsyncWebServer server(80);

const char* PARAM_STRING = "inputString";
const char* PARAM_STRING2 = "inputString2";

//HTML webpage code that contains two textboxes, one for SSID and one for password
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function submitMessage() {
      alert("Saved value to ESP SPIFFS");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
  </script></head><body>
  <form action="/get" target="hidden-form">
    inputString (current value %inputString%): <input type="text" name="inputString">
    inputString (current value passwordfield): <input type="password" name="inputString2">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
  <iframe style="display:none" name="hidden-form"></iframe>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

//Read current value from the ESP32 file system
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
}

//Function to write the value of the text boxes to the ESP32 file system
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Replaces "current value" next to the text boxes with stored values found in the ESP32 file system
String processor(const String& var){
  //Serial.println(var);
  if(var == "inputString"){
    return readFile(SPIFFS, "/inputString.txt");
  }
  else if(var == "inputString2"){
    return readFile(SPIFFS, "/inputString2.txt");
  }
  return String();
}

void setup() {
  Serial.begin(115200);
  // Initialize SPIFFS
  #ifdef ESP32
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #else
    if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #endif
//Start the WiFi network with the provided SSID and PASSWORD
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Send web page with input fields
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputMessage2;
    const char *SSID;
    const char *WiFiPassword;
    // GET inputString value from <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_STRING)) {
      inputMessage = request->getParam(PARAM_STRING)->value();
      inputMessage2 = request->getParam(PARAM_STRING2)->value();
      //Write the values to the file system
      writeFile(SPIFFS, "/inputString.txt", inputMessage.c_str());
      writeFile(SPIFFS, "/inputString2.txt", inputMessage2.c_str());
      SSID = inputMessage.c_str();
      WiFiPassword = inputMessage2.c_str();
      //Try to connect to the WiFi with the given SSID and PASSWORD
      WiFi.mode(WIFI_STA);
      WiFi.enableSTA(true);
      WiFi.begin(SSID, WiFiPassword);
      Serial.print("Connecting to "); Serial.println(SSID);
      uint8_t i = 0;
      while (WiFi.status() != WL_CONNECTED)
      {
        Serial.print('.');
        delay(500);
     
        if ((++i % 16) == 0)
        {
          Serial.println(F(" still trying to connect"));
        }
      }
      Serial.print(F("Connected. My IP address is: "));
      Serial.println(WiFi.localIP()); 
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  // Troubleshooting section that prints out the current strings stored in the File System
  //String yourInputString = readFile(SPIFFS, "/inputString.txt");
  //Serial.print("*** Your inputString: ");
  //Serial.println(yourInputString);

  //String yourInputString2 = readFile(SPIFFS, "/inputString2.txt");
  //Serial.print("*** Your inputString: ");
  //Serial.println(yourInputString2);
  

}
