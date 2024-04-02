// ESP8266 kód pro webový server přes který lze ovládat panel

// knihovny
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80); 


//získávání času
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>   

WiFiUDP ntpUDP;
//                              server       offset  update
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 1000);

char Time[ ] = "00:00:00";
char Date[ ] = "00.00.2000";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;

int period = 1000;
unsigned long time_now = 0;


//html + css kód (webová stránka)
#include "index.h"

//SSID, heslo
const char* ssid = "Holeckovi";
const char* password = "314159265";

String textosend_string;
String textosendSpeed_string;
String textosendRed_string;
String textosendGreen_string;
String textosendBlue_string;
String buttonON;
String buttonOFF;

String teplota = "";
String teplota2 = "";

String getPage()
{
  String s = MAIN_page;
  return s;
  // stránka musí být bez escape charů , tj. "\"\", zpetny lomitka, nevim proc, ma to neco spolecnyho se stringem
}


void handleNotFound()
{
  server.send(200, "text/html", getPage()); //Send web page 
}

void handleSubmit()
{
  //Text to show
  if(server.hasArg("textosend"))
  {
    textosend_string = server.arg("textosend");
    Serial.print("*" + textosend_string);
  }
  if(server.hasArg("textosendSpeed")){
    textosendSpeed_string = server.arg("textosendSpeed");
    Serial.print(";" + textosendSpeed_string);
  }
  if(server.hasArg("ledRed") || server.hasArg("ledGreen") || server.hasArg("ledBlue")){
    textosendRed_string = server.arg("ledRed");
    textosendGreen_string = server.arg("ledGreen");
    textosendBlue_string = server.arg("ledBlue");

    Serial.print("!" + textosendRed_string + "-" + textosendGreen_string + ":" + textosendBlue_string);
  }
    
  
  if (server.hasArg("button1ON"))
  {
    Serial.print("tlacON");
      /*
      buttonON = server.arg("button1ON");
      Serial.print("Thereceived button is: ");
      Serial.println(buttonON);
      */
  } 
  if (server.hasArg("button1OFF"))
  {
    Serial.print("tlacOFF");
  }      
  
    
  server.send(200, "text/html", getPage()); //Send web page       //Response to the HTTP request
}  

void handleRoot() {
  if (server.args() ) 
  {
    handleSubmit();
  } 
  else 
  {
    server.send(200, "text/html", getPage()); //Send web page 
  }
}

void handleTemp(){
  
  while(Serial.available() > 0){
    char inByte = Serial.read();
      teplota += String(inByte);
  }
  teplota = teplota.substring(0,5);

  server.send(200, "text/plane", teplota);

  teplota = "";
}

void handleTime(){
  String timeDate = String(Time) + " " + String(Date);
  server.send(200, "text/plane", timeDate);
}


void setup(void){
  // zahájení sériové komunikace
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);     
  Serial.println("");

  // čekání na připojení k wifi
  Serial.print("připojuji k síti: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // zobrazí se když se ESP připojí k Access pointu + zobrazí IP adresu přidělenou routerem
  /*
  Serial.println("");
  Serial.print("připojeno k: ");
  Serial.println(ssid);
  */
  Serial.print("+");
  Serial.println(WiFi.localIP());  
  server.begin();
  // handlery + subrutiny
  server.on("/", handleRoot);      
  server.on("/readTemp", handleTemp);
  server.on("/getTime", handleTime);
  server.onNotFound(handleNotFound);

  server.begin();
  timeClient.begin();

  
}

void loop(void){

  timeClient.update();

  unsigned long unix_epoch = timeClient.getEpochTime();    // unixový čas(epoch time) z ntp serveru
 
  second_ = second(unix_epoch);

  // čas se posílá každých 500ms
  if(millis() >= time_now + period){
        time_now += period;
          minute_ = minute(unix_epoch);
          hour_   = hour(unix_epoch);
          day_    = day(unix_epoch);
          month_  = month(unix_epoch);
          year_   = year(unix_epoch);
      
          Time[7] = second_ % 10 + 48;
          Time[6] = second_ / 10 + 48;
          Time[4]  = minute_ % 10 + 48;
          Time[3]  = minute_ / 10 + 48;
          Time[1]  = hour_   % 10 + 48;
          Time[0]  = hour_   / 10 + 48;
      
          Date[0]  = day_   / 10 + 48;
          Date[1]  = day_   % 10 + 48;
          Date[3]  = month_  / 10 + 48;
          Date[4]  = month_  % 10 + 48;
          Date[8] = (year_   / 10) % 10 + 48;
          Date[9] = year_   % 10 % 10 + 48;
      
          // podtrzitko pro rozliseni data u prijmu v Arduinu (viz * pro input text field na serveru)
          Serial.print("_");
          Serial.print(String(Time));
          Serial.print(" ");
          Serial.println(String(Date));
        
        
    }
  //kontrol zda se něco na web serveru nestalo
  server.handleClient();     
}