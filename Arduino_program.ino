#include <SPI.h>
#include <Wire.h>

#include <DHT.h>
#define DHTPIN 42
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temp;
float hum;


//led paske
#include <FastLED.h>
#define NUM_LEDS 24
#define DATA_PIN_LED 5
CRGB leds[NUM_LEDS];


#include <Servo.h>
Servo myServo;


#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 12 
#define CLK_PIN   52
#define DATA_PIN  51
#define CS_PIN    40
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);


#include <MQ135.h>
#define pinAMQ A8
#define pinDMQ 32
float ppm;
MQ135 senzorMQ = MQ135(pinAMQ);

float reading;
#define LIGHTSENSORPIN A9



#include <LCDWIKI_SPI.h> 
#include <LCDWIKI_GUI.h>
// nastavení pinů displeje TFT
#define MODEL ST7796S
#define CS   A5    
#define CD   A3
#define RST  A4
#define LED  A0  
// konstruktor TFT displeje / objektu
LCDWIKI_SPI display(MODEL,CS,CD,RST,LED);


// oled displej
#include <U8glib.h>
#define SCK 13
#define SDA 11
#define RES 10
#define DC 9
#define CS 8
// konstruktor OLED displeje
U8GLIB_SH1106_128X64 oled(SCK, SDA, CS, DC, RES);


// tlacitka up down left right
#define buttonDn 36
#define buttonUp 38
#define buttonLeft 41
#define buttonRight 39


// ultrazvukove cidlo
#define trig 22
#define echo 23
long odezva;
long vzdalenost;


#define IRsenzor 24
#define PIRsenzor 25


//rgb led
#define redLed 4
#define greenLed 6
#define blueLed 7


//promenne pro "debouncing tlacitek"
bool buttonUpClicked = true;
bool buttonDnClicked = true;
bool buttonLeftClicked = true;
bool buttonRightClicked = true;

//souradnice pohybliveho ramecku
int xCorSelectBoxStart = 30;
int yCorSelectBoxStart = 320/2 - 130;
int xCorSelectBoxEnd = 130;
int yCorSelectBoxEnd = 320/2 - 10;

//minuly stav souradnic pohybliveho ramecku na premazani
int xCorSelectBoxStartLast;
int yCorSelectBoxStartLast;
int xCorSelectBoxEndLast;
int yCorSelectBoxEndLast;

bool displayChanged = false;
// index který slouží k orientaci v navigačním menu
int menuIndex = 10; 

// promenne pro funkci millis(), misto delay
int period = 1000;
unsigned long time_now = 0;

String datumcas = "";
String localIP = "";
String RGBvalueServer = "";
String motorValueServer = "";
int motorValue = 0;

int hodnota;

// funkce pro TFT displej
void show_string(uint8_t *string,int16_t x,int16_t y,uint8_t textSize,uint16_t textColor, uint16_t backgroundColor,boolean mode)
{
    display.Set_Text_Mode(mode);
    display.Set_Text_Size(textSize);
    display.Set_Text_colour(textColor);
    display.Set_Text_Back_colour(backgroundColor);
    display.Print(string,x,y);
}

void clear_screen(void)
{
   display.Fill_Screen(0x0000);
}

// funkce vykreslující OLED
void clearOLED() {
  oled.firstPage();
  do {
  } while ( oled.nextPage() );
}

void draw() {
  //predchozi
  oled.drawStr(5, 10, "*");
  switch(menuIndex){
    case 10:
      oled.drawStr(18, 15, "vzdalenost [cm]");
      oled.setFont(u8g_font_fub30);
      oled.drawStr(44, 60, String(vzdalenost).c_str());
      oled.setFont(u8g_font_7x14);
      
      break;
    case 11:
      oled.drawStr(10, 15, "passive infrared");
      if(digitalRead(PIRsenzor) == HIGH){
        oled.setFont(u8g_font_fub30);
        oled.drawStr(5,60,"pohyb!");
        oled.setFont(u8g_font_7x14);
      }
      break;
    case 12:
      oled.drawStr(10, 15, "infrared senzor");
      if(digitalRead(IRsenzor) == HIGH){
        oled.setFont(u8g_font_fub20);
        oled.drawStr(5,40,"prekazka!");
        oled.setFont(u8g_font_7x14);
      }
      break;
    case 13:
      oled.drawStr(10,15,"IP adresa serveru");
      oled.setFont(u8g_font_fub11);
      oled.drawStr(10,50,localIP.c_str());
      oled.setFont(u8g_font_7x14);

      break;
    case 20:
      oled.drawStr(24, 15, "teplota [C]");
      oled.setFont(u8g_font_fub30);
      oled.drawStr(14,60,String(temp).c_str());
      oled.setFont(u8g_font_7x14);
      
      break;
    case 21:
     
      oled.drawStr(24, 15, "vlhkost [%]");
      oled.setFont(u8g_font_fub30);
      oled.drawStr(14,60,String(hum).c_str());
      oled.setFont(u8g_font_7x14);
      break;
    case 22:

      if(millis() >= time_now + period){
        time_now += period;
        reading = analogRead(LIGHTSENSORPIN); //Read light level
      }
      oled.drawStr(24, 15, "osvit");
      oled.setFont(u8g_font_fub30);
      oled.drawStr(14,60,String(reading).c_str());
      oled.setFont(u8g_font_7x14);

      break;
    case 23:
      
      oled.drawStr(10, 15, "koncentrace [ppm]");
      if(millis >= time_now + period){
        time_now += period;
        ppm = senzorMQ.getPPM();
      }
      oled.setFont(u8g_font_fub30);
      oled.drawStr(30,60,String(ppm).c_str());
      oled.setFont(u8g_font_7x14);
      break;
  }
}

void setRGBLEDcolor(int Red,int Green,int Blue){
  analogWrite(redLed,Red);
  analogWrite(greenLed,Green);
  analogWrite(blueLed,Blue);
}


void setup() {

  // sériový monitor
  Serial.begin(9600);
  //komunikace s ESP8266
  Serial3.begin(115200);

  SPI.begin();

//led páske
  
  FastLED.addLeds<NEOPIXEL, DATA_PIN_LED>(leds, NUM_LEDS);
  FastLED.setBrightness(100);

  P.begin();
  P.setIntensity(10);

  myServo.attach(3);
  myServo.write(40);

  dht.begin();

  // nastavení OLED displeje
  oled.setFont(u8g_font_7x14);
  clearOLED();

  // nastavení TFT displeje
  display.Init_LCD();
  display.Fill_Screen(0x0000);
  display.Set_Rotation(1);

  // čidlo vzdálenosti HC-SR04
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  // ostatní senzory
  pinMode(IRsenzor, INPUT);
  pinMode(PIRsenzor, INPUT);
  
  //tlacitka
  pinMode(buttonDn,INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonLeft, INPUT_PULLUP);
  pinMode(buttonRight, INPUT_PULLUP);

  //dopsat co to je za piny
  pinMode(CS_PIN, OUTPUT);
  pinMode(DHTPIN, OUTPUT);

  pinMode(LIGHTSENSORPIN, INPUT); 
  
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  pinMode(24, INPUT);
 
  
  
}

void loop() {
  
  // načtení hodnot z DHT11 pro zobrazení na web serveru
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  Serial3.print(temp);
  
  //reset LEDek
  for(int i = 0; i< 24;i++){
    leds[i] = CRGB::Black;
    
  }
  
 
  
  
  if(P.displayAnimate()){
    P.displayText(datumcas.c_str(), PA_CENTER, 1, 100, PA_NO_EFFECT,PA_NO_EFFECT);
  }

  // příjem dat z onboard ESP8266 / web serveru
  String ESPdata = "";
  while (Serial3.available() > 0) {  //když je dostupný serial3 (esp)
    char inByte = Serial3.read(); 

    if(inByte != ""){
      ESPdata += String(inByte);
    }
    
  }
  if(ESPdata != ""){
    Serial.println("ESP8266 DATA:");                  
	  Serial.println(ESPdata + "*");

    // horejsi kod odkomentovat kdyz debugujeme data z web serveru
    
  }
  if(ESPdata.indexOf("+") >= 0){
    localIP = ESPdata.substring(2,15);
    Serial.print("lokal IP je: ");
    Serial.println(localIP);
  }
  // kontrolovat je zda obsahuje dany text, moc velky datovy tok
  else if(ESPdata.indexOf("tlacOFF") >= 0){
    Serial.println("tlacoff test");
    
  }
  else if(ESPdata.indexOf("tlacON") >= 0){
    Serial.println("tlacontest");
  
  }
  else if(ESPdata.indexOf("*") >= 0){
    
    P.print(ESPdata.substring(1));
    delay(1000);

  }
  else if(ESPdata.indexOf("!") >= 0){
    RGBvalueServer = ESPdata.substring(1);

    int firstIndex = RGBvalueServer.indexOf("-");
    int secondIndex = RGBvalueServer.indexOf(":");

    int redValueServer = RGBvalueServer.substring(0,firstIndex).toInt();
    int greenValueServer = RGBvalueServer.substring(firstIndex + 1,secondIndex).toInt();
    int blueValueServer = RGBvalueServer.substring(secondIndex + 1).toInt();

    Serial.print("RGB value: ");
    Serial.print(redValueServer);
    Serial.print(greenValueServer);
    Serial.println(blueValueServer);

    setRGBLEDcolor(redValueServer,greenValueServer,blueValueServer);
    
  }
  else if(ESPdata.indexOf(";") >= 0){
    motorValueServer = ESPdata.substring(1);
    motorValue = motorValueServer.toInt();
    myServo.write(motorValue);
    
  }
  //kontorlvoat zda obsahuje jako zde nize
  else if(ESPdata.indexOf("_") >= 0){
    datumcas = ESPdata.substring(1,16); // zmena z 1,9
    
  }
  ESPdata = "";

  

  

  
  xCorSelectBoxStartLast = xCorSelectBoxStart;
  yCorSelectBoxStartLast = yCorSelectBoxStart;
  xCorSelectBoxEndLast = xCorSelectBoxEnd;
  yCorSelectBoxEndLast = yCorSelectBoxEnd;


  //handlery tlacitek
  if((digitalRead(buttonUp) == LOW) && (buttonUpClicked == true)){
    if((menuIndex % 100)/10 == 2){
      menuIndex -=10;
      yCorSelectBoxStart -= 140;
      yCorSelectBoxEnd -=140;
      displayChanged = true;
    }
    buttonUpClicked = false;
  }
  if((digitalRead(buttonDn) == LOW) && (buttonDnClicked == true)){
    if((menuIndex % 100)/10 == 1){
      menuIndex +=10;
      yCorSelectBoxStart += 140;
      yCorSelectBoxEnd +=140;
      displayChanged = true;
    }
    buttonDnClicked = false;
  }
  if((digitalRead(buttonLeft) == LOW) && (buttonLeftClicked == true)){
    if((menuIndex % 10)/1 == 3 || (menuIndex % 10)/1 == 2 || (menuIndex % 10)/1 == 1){
      menuIndex -= 1;
      xCorSelectBoxStart -= 100;
      xCorSelectBoxEnd -= 100;
      displayChanged = true;
    }
    buttonLeftClicked = false;
  }
  if((digitalRead(buttonRight) == LOW) && (buttonRightClicked == true)){
    if((menuIndex % 10)/1 == 0 || (menuIndex % 10)/1 == 1 || (menuIndex % 10)/1 == 2){
      menuIndex += 1;
      xCorSelectBoxStart += 100;
      xCorSelectBoxEnd += 100;
      displayChanged = true;
    }
    buttonRightClicked = false;
  }
  if ((digitalRead(buttonUp) == HIGH) && (buttonUpClicked == false)) {
    buttonUpClicked = true;
  }
  if ((digitalRead(buttonDn) == HIGH) && (buttonDnClicked == false)) {
    buttonDnClicked = true;
  }
  if ((digitalRead(buttonLeft) == HIGH) && (buttonLeftClicked == false)) {
    buttonLeftClicked = true;
  }
  if ((digitalRead(buttonRight) == HIGH) && (buttonRightClicked == false)) {
    buttonRightClicked = true;
  }

  
  //switch statement na vykonani vybrane akce
  switch(menuIndex){
    case 10:
      digitalWrite(trig, LOW);
      delayMicroseconds(2);
      digitalWrite(trig, HIGH);
      delayMicroseconds(5);
      digitalWrite(trig, LOW);
      odezva = pulseIn(echo, HIGH);
      vzdalenost = odezva / 58.31;
      Serial.println(vzdalenost);
      setRGBLEDcolor(0,250,0);
      Serial.println("test");
      
      hodnota = map(vzdalenost,0,70,0,24);
      for(int i = 0; i< hodnota;i++){
  
      leds[i] = CRGB::Blue;
      }
      
      break;
    case 11:
      if(digitalRead(PIRsenzor) == HIGH){
        Serial.println("detekován pohyb");
        for(int i = 0;i<24;i++){
          leds[i] = CRGB::Red;
        }
      }
      else{
        for(int i = 0;i<24;i++){
          leds[i] = CRGB::Black;
        }
      }
      break;
    case 12:
      if(digitalRead(IRsenzor) == LOW){
        Serial.println("detekována překážka");
        myServo.write(180);
      }
      else{
        myServo.write(90);
      }
      break;
    case 13:
      Serial.println("Server");
      setRGBLEDcolor(250,0,0);
      break;
    case 20:
      temp = dht.readTemperature();
      Serial.print("Teplota: ");
      Serial.println(temp);
      break;
    case 21:
      hum = dht.readHumidity();
      Serial.print("Vlhkost: ");
      Serial.println(hum);

      
      break;
    case 22:
      
      reading = analogRead(LIGHTSENSORPIN); 
      
      Serial.println(reading);


      break;
    case 23:
      
        ppm = senzorMQ.getPPM();
        Serial.print("Koncentrace plynů: ");
        Serial.println(ppm);
      
      break;
    default:
      Serial.println(menuIndex);
      break;
  }

  

  // cara pres pul obrazovky
  display.Set_Draw_color(0xFFFF);
  display.Draw_Rectangle(30,320/2,450,320/2);
  //premazani puvodniho ramecku (jen pokud nastala nejaka zmena obrazu, bylo by zbytecne a pomale premazavat displej po kazdem projiti smycky loop)
  if(displayChanged){
    display.Set_Draw_color(0x0000);
    display.Draw_Rectangle(xCorSelectBoxStartLast,yCorSelectBoxStartLast,xCorSelectBoxEndLast,yCorSelectBoxEndLast);
    displayChanged = false;
  }
  //vykresleni noveho ramecku
  display.Set_Draw_color(0xFFFF);
  display.Draw_Rectangle(xCorSelectBoxStart,yCorSelectBoxStart,xCorSelectBoxEnd,yCorSelectBoxEnd);
  // názvy políček
  show_string("dist", 40, 320/2 - 80, 3, 0xFFFF, 0x0000, 1);
  show_string("PIR", 140, 320/2 - 80, 3, 0xFFFF, 0x0000, 1);
  show_string("IR", 240, 320/2 - 80, 3, 0xFFFF, 0x0000, 1);
  show_string("Server", 340, 320/2 - 80, 3, 0xFFFF, 0x0000, 1);
  //---------------------------------------------------
  show_string("temp", 40, 320/2 + 60, 3, 0xFFFF, 0x0000, 1);
  show_string("hum", 140, 320/2 + 60, 3, 0xFFFF, 0x0000, 1);
  show_string("Lx", 240, 320/2 + 60, 3, 0xFFFF, 0x0000, 1);
  show_string("AirQ", 340, 320/2 + 60, 3, 0xFFFF, 0x0000, 1);


  // OLED vykreslení (musí být na konci programu)
  oled.firstPage();
  do {
    draw();
  } while (oled.nextPage());
  
  FastLED.show();
  
