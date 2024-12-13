#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

bool status = true;
int pins[4] = {10,5,15,12};


const char* ssid = "Antonio";
const char* password = "jose1944";


ESP8266WebServer server(80);

String var1;  
String var2;

bool reverseStatus = false;

String login = "ventiladores";
String pass = "123";

String timerStart = "00:00";
String timerEnd = "01:00";

int timerHoursStart = 0;
int timerMinutesStart = 0;

int timerHoursEnd = 0;
int timerMinutesEnd = 0;

int hoursNow;
int minutesNow;

int lastMinute = 0;


const char* ntpServerName = "pool.ntp.org";
const int utcOffsetInSeconds = -3 * 3600; 


WiFiUDP ntpUDP;

#define offset -10800

NTPClient timeClient(ntpUDP, "pool.ntp.org");


void setup() {
  
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);

  timeClient.begin();
  timeClient.setTimeOffset(offset);    


  pinMode(10, OUTPUT); 
  pinMode(5, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(12, OUTPUT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  
  
  String myIP = WiFi.localIP().toString();

  


  
  server.on("/", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204); 
  });

  
  server.on("/", []() {
    if (server.method() == HTTP_GET) {
       
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

       
      Serial.println("GET");
      server.send(200, "text/html", "<h1>GET Request</h1><p>Bem-vindo ao seu servidor ESP8266 via GET!</p>");
    }else {
     
      server.send(405, "text/html", "<h1>405 Method Not Allowed</h1>");
    }
  });

server.on("/login", []() {
    if (server.method() == HTTP_GET) {
      
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

      
      if (server.hasArg("var1") && server.hasArg("var2")) {
        var1 = server.arg("var1");
        var2 = server.arg("var2");

        Serial.print("Login: ");
        Serial.print(var1);
        Serial.print(" Pass: ");
        Serial.println(var2);

        
        DynamicJsonDocument doc(1024);
        doc["var1"] = var1;
        doc["var2"] = var2;

        
        if (var1 == login && var2 == pass) {
          doc["login_s"] = "true";
        } else {
          doc["login_s"] = "false";
        }

        String response;
        serializeJson(doc, response);

        
        server.send(200, "application/json", response);

      } else {
        
        server.send(400, "application/json", "{\"error\":\"Missing parameters\"}");
      }
      
    } else if (server.method() == HTTP_OPTIONS) {
      // Lida com requisições OPTIONS para CORS
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(204);  // Resposta sem conteúdo para requisições OPTIONS
    } else {
      // Outros métodos
      server.send(405, "text/html", "<h1>405 Method Not Allowed</h1>");
    }
});

server.on("/dashboard/eletro2", HTTP_GET, []() {
     
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

    DynamicJsonDocument doc(1024);
    doc["status"] = status;

    String response;
    serializeJson(doc, response);

     
    server.send(200, "application/json", response);
});

server.on("/timers", HTTP_GET, []() {
     
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

    DynamicJsonDocument doc(1024);
    doc["start"] = timerStart;
    doc["end"] = timerEnd;

    String response;
    serializeJson(doc, response);

     
    //Serial.println("GET NOS TIMERS");
    server.send(200, "application/json", response);
});

server.on("/timers", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);  
});

server.on("/dashboard/eletro2", HTTP_POST, []() {
     
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

    if (server.hasArg("plain")) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, server.arg("plain"));

        if (error) {
             
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

         
        var1 = doc["status"].as<String>();

        if (var1 == "true"){
          status = true;
        }else{
          status = false;
        }

         
        String response = "{\"Status\":\"" + var1 + "\"}";
        
        Serial.print("Status: ");
        Serial.println(status);

        server.send(200, "application/json", response);
    } else {
         
        server.send(400, "application/json", "{\"error\":\"No JSON received\"}");
    }
});

server.on("/timers/post", HTTP_POST, []() {
     
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

    if (server.hasArg("plain")) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, server.arg("plain"));

        if (error) {
             
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        
        timerStart = doc["var1"].as<String>();
        timerEnd = doc["var2"].as<String>();

         
        String response = "{\"Start\":\"" + timerStart + "\",\"End\":\"" + timerEnd + "\"}";

        sscanf(timerStart.c_str(), "%2d:%2d", &timerHoursStart, &timerMinutesStart);
        sscanf(timerEnd.c_str(), "%2d:%2d", &timerHoursEnd, &timerMinutesEnd);
        

        Serial.print("StartTime: ");
        Serial.print(timerStart);
        Serial.print(" EndTime: ");
        Serial.println(timerEnd);

        server.send(200, "application/json", response);
    } else {
         
        server.send(400, "application/json", "{\"error\":\"No JSON received\"}");
    }
});

server.on("/timers/post", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);  
});

 
server.on("/dashboard/eletro2", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);   
});



   
  server.begin();
}

void loop() {

  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);

  hoursNow = timeClient.getHours();
  minutesNow = timeClient.getMinutes();

  if (lastMinute != minutesNow){
    Serial.print("Hours: ");
    Serial.print(hoursNow);
    Serial.print(" Minutes: ");
    Serial.println(minutesNow);

    if (hoursNow == timerHoursStart){
      if (minutesNow == timerMinutesStart){
        status = true;
        Serial.println("ligando...");
      }
    } else if (hoursNow == timerHoursEnd){
        if(minutesNow == timerMinutesEnd){
          status = false;
          Serial.println("desligando...");
      }
    }

    
    lastMinute = minutesNow;
  } 





  if (status == true){
    reverseStatus = false;
  }else {
    reverseStatus = true;
  }

 for (int i = 0; i < 4; i++){
      digitalWrite(pins[i], reverseStatus);
    }

   
  server.handleClient();
}
