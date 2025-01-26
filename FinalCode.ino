#include <DHT.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>


#define PIN_SPI_CS 5 // The ESP32 pin GPIO5 FOR SD CARD MODULE
#define tempSensorPin 15
#define mSensor1 32 
#define mSensor2 33
#define pump1 25
#define pump2 14
#define dry 2680
#define wet 960

AsyncWebServer server(80);
WebSocketsServer websockets(81);

const char* ssid       = "Word.exe";
const char* password   = "Cat@12345";


//Object Declaration

LiquidCrystal_I2C lcd(0x27,16,2); 

DHT tempSensor(tempSensorPin,DHT11);

                                   //FILE HANDLER FOR SD CARD



//Variables used in Code
uint8_t serverStatus=0;

float tempInC,tempInF,humidity;
int moisValue1,moisValue2;
int moisValue1_Percent,moisValue2_Percent;
uint8_t pumpOneState=0,pumpTwoState=0,autoMode=0;



unsigned long TempReadinterval=1500,TempPrevTime=0,currentTime;
unsigned long HumidityReadinterval=2500,HumidityPrevTime=0;
unsigned long TempDisplayInterval=3000,TempDisplayPrevTime=0;
unsigned long MoistureReadinterval=100,MoisturePrevTime=0;
unsigned long MoistureDisplayInterval=1000,MoistureDisplayPrevTime=0;
unsigned long wifiConnectInterval=1000,wifiConnectPrevTime=0;
unsigned long sendDataInterval=100,sendDataPrevTime=0;
bool sd_status=false;

uint8_t count=0;                                    //To Display IP Adress only once





//Byte Symbol Representation declarations

byte degreeSymbol[8]= { B00000, B00111, B00101, B00111, B00000, B00000, B00000, B00000 };
byte noWifiSymbol[8] = { B10001, B01010, B00100, B01010, B10001, B00100, B00100, B00100 };
byte wifiConnectedSymbol[8] = { B00000, B00000, B00001, B00001, B00101, B00101, B10101, B10101 };
byte moistureTankZero[8]= { B00100, B01010, B10001, B10001, B10001, B10001, B01110, B00000 };
byte moistureTank20[8]={ B00100, B01010, B10001, B10001, B10001, B11111, B01110, B00000 };
byte moistureTank40[8]={ B00100, B01010, B10001, B10001, B11111, B11111, B01110, B00000 };
byte moistureTank60[8]={ B00100, B01010, B10001, B11111, B11111, B11111, B01110, B00000 };
byte moistureTank80[8]= { B00100, B01010, B11111, B11111, B11111, B11111, B01110, B00000 };
byte moistureTank100[8]={ B00100, B01110, B11111, B11111, B11111, B11111, B01110, B00000 };
byte moistureTankError[8]={ B00000, B00000, B01010, B00100, B00100, B01010, B00000, B00000 };
byte sdDetected[8] = { B00000, B00000, B00000, B11000, B10110, B10010, B10010, B11110 };
byte sdNotDetected[8] = { B00101, B00010, B00101, B11000, B10110, B10010, B10010, B11110 };

void notFound(AsyncWebServerRequest * request)
{
  request->send(404,"text/plain","Tu abhi bhi gaya nhi chutiye !!!");
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) 
{
  switch (type) 
  {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        websockets.sendTXT(num, "Connected from server");
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        String message = String((char *)(payload));

        // Check if the payload is "ping"
        if (message == "ping") {
          Serial.println("Responded Pong");
          websockets.sendTXT(num, "pong");
          return; // No further processing needed for plain text
        }

        // If not "ping", try parsing it as JSON
        DynamicJsonDocument doc(200);
        DeserializationError error = deserializeJson(doc, message);
        if (error) {
          Serial.print("Deserialize Json() failed: ");
          Serial.println(error.c_str());
          return;
        }

        // Parse JSON values
        autoMode = doc["autoSwitch"];
        if (autoMode == 0) {
          pumpOneState = doc["pump1"];
          pumpTwoState = doc["pump2"];
        }
        activatePump();
      }
      break;
    }
}

void send_sensor()
{ 
    if (isnan(tempInC) || isnan(humidity) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // JSON_Data = {"temp":tempInC,"hum":humidity,"moisOne":moisValue1_Percent,"moisTwo":moisValue2_Percent,"pump1":pumpOneState,,"pump2":pumpTwoState,"autoSwitch":autoMode}
  String JSON_Data = "{\"temp\":";
         JSON_Data += tempInC;
         JSON_Data += ",\"hum\":";
         JSON_Data += humidity;
         JSON_Data += ",\"moisOne\":";
         JSON_Data += moisValue1_Percent;
         JSON_Data += ",\"moisTwo\":";
         JSON_Data += moisValue2_Percent;
         JSON_Data += ",\"pump1\":";
         JSON_Data += pumpOneState;
         JSON_Data += ",\"pump2\":";
         JSON_Data += pumpTwoState;
         JSON_Data += ",\"autoSwitch\":";
         JSON_Data += autoMode;
         JSON_Data += "}";
  // Serial.println(JSON_Data);     
  websockets.broadcastTXT(JSON_Data);
}



void connectWifi()
{
  if((unsigned long)(currentTime-wifiConnectPrevTime)>=wifiConnectInterval){                                                  
  if(WiFi.status()!=WL_CONNECTED)
  {
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    wifiConnectPrevTime=currentTime;
  }
  
    sd_status=SD.begin(PIN_SPI_CS);
    if(sd_status)
    {
      if(!SD.exists("/index.html"))
      {
      sd_status=false;
      }
    }
}
}

void activatePump()
{
  if(autoMode==1)
  {
    if(moisValue1_Percent<10)
        pumpOneState=1;
    else
      pumpOneState=0;
    if(moisValue2_Percent<10)
      pumpTwoState=1;
    else
      pumpTwoState=0;
  }
  digitalWrite(pump1,pumpOneState);
  digitalWrite(pump2,pumpTwoState);
}

void clearScr()
{
   if(moisValue1_Percent<10)
    {   
          for(int j=11;j<=12;j++)
          { 
             lcd.setCursor(j,0);
             lcd.print(" ");
          }
    }
    else if(moisValue1_Percent>10 && moisValue1_Percent<100)
    {   
          for(int j=12;j<=12;j++)
          {
            lcd.setCursor(j,0);
            lcd.print(" ");
          }
    }
   if(moisValue2_Percent<10)
    {   
          for(int j=11;j<=12;j++)
          {
            lcd.setCursor(j,1);
            lcd.print(" ");
          }
    }
    else if(moisValue2_Percent>10 && moisValue2_Percent<100)
    {   
          for(int j=12;j<=12;j++)
          {
            lcd.setCursor(j,1);
            lcd.print(" ");
          }
    }
}
void creatCharacters()
{
  if(moisValue1_Percent<=0)
  {
    lcd.createChar(4,moistureTankZero);
  }
  else if(moisValue1_Percent>0 && moisValue1_Percent<=20)
  {
    lcd.createChar(4,moistureTank20);
  }
  else if(moisValue1_Percent>20 && moisValue1_Percent<=40)
  {
    lcd.createChar(4,moistureTank40);
  }
    else if(moisValue1_Percent>40 && moisValue1_Percent<=60)
  {
    lcd.createChar(4,moistureTank60);
  }
    else if(moisValue1_Percent>60 && moisValue1_Percent<=80)
  {
    lcd.createChar(4,moistureTank80);
  }
    else if(moisValue1_Percent>80 && moisValue1_Percent<=100)
  {
    lcd.createChar(4,moistureTank100);
  }
  else
  {
    lcd.createChar(4,moistureTankError);
  }


  if(moisValue2_Percent<=0)
  {
     lcd.createChar(5,moistureTankZero);
  }
  else if(moisValue2_Percent>0 && moisValue2_Percent<=20)
  {
    lcd.createChar(5,moistureTank20);
  }
   else if(moisValue2_Percent>20 && moisValue2_Percent<=40)
  {
    lcd.createChar(5,moistureTank40);
  }
   else if(moisValue2_Percent>40 && moisValue2_Percent<=60)
  {
    lcd.createChar(5,moistureTank60);
  }
   else if(moisValue2_Percent>60 && moisValue2_Percent<=80)
  {
    lcd.createChar(5,moistureTank80);
  }
   else if(moisValue2_Percent>80 && moisValue2_Percent<=100)
  {
    lcd.createChar(5,moistureTank100);
  }
  else
  {
    lcd.createChar(5,moistureTankError);
  }


  if(WiFi.status()!=WL_CONNECTED)
  {
    lcd.createChar(1,noWifiSymbol);
  }
  else if(WiFi.status()==WL_CONNECTED)
  {
    lcd.createChar(1,wifiConnectedSymbol);
  }

  if(sd_status)
  {
    lcd.createChar(6,sdDetected);
  }
  else
  {
    lcd.createChar(6,sdNotDetected);
  }
}

void readSensorData()
{
  //Read DHT11 Temperature Sensor
  if((unsigned long)(currentTime-TempPrevTime)>=TempReadinterval)
    {
      tempInC=tempSensor.readTemperature();
      tempInF=(tempInC*1.8)+32;
      TempPrevTime=currentTime;
    }
  //Read DHT11 Humidity Sensor
  if((unsigned long)(currentTime-HumidityPrevTime)>=HumidityReadinterval)
    {
      humidity=tempSensor.readHumidity();
      HumidityPrevTime=currentTime;
    }
  //Read Moisture Sensor
  if((unsigned long)(currentTime-MoisturePrevTime)>=MoistureReadinterval)
    {
      moisValue1 = analogRead(mSensor1);
      moisValue1_Percent=map(moisValue1,wet,dry, 100, 0);
      moisValue2 = analogRead(mSensor2);
      moisValue2_Percent=map(moisValue2,wet,dry, 100, 0);
      MoisturePrevTime=currentTime;
      activatePump();
    }
}



void lcdDisplay()
{
  //_________________________°C________________
  lcd.setCursor(0,0);
  lcd.print(tempInC);
  lcd.setCursor(5,0);
  lcd.write(0); //degree Sysmbol
  lcd.print("c"); //Celcius Symbol
  //_________________________°F_________________
  lcd.setCursor(0,1);
  lcd.print(humidity);
  lcd.setCursor(5,1);
  // lcd.write(0); //degree symbol
  lcd.print("%"); //Fahrenheit Symbol


  
  //__________________________Moisture Tank #1________________
  lcd.setCursor(8,0);
  lcd.write(4); //empty tank symbol

  //__________________________Moisture Tank #2________________
  lcd.setCursor(8,1);
  lcd.write(5); //empty tank symbol


   if((unsigned long)(currentTime-MoistureDisplayPrevTime)>=MoistureDisplayInterval)
  {
  clearScr();
  lcd.setCursor(9,0);
  lcd.print(moisValue1_Percent);
  lcd.print("%");
  //__________________________Moisture Tank #2________________
  lcd.setCursor(9,1);
  lcd.print(moisValue2_Percent);
  lcd.print("%");
  MoistureDisplayPrevTime=currentTime;
  }
  if(WiFi.status()==WL_CONNECTED)
    {
    lcd.setCursor(15,0);
    lcd.write(1);
    }
  else
    {
    lcd.setCursor(15,0);
    lcd.write(1);
    }

  if(sd_status)
  {
    lcd.setCursor(15,1);   //DISPLAY THE SD CONNECTED ICON
    lcd.write(6);
  }
  else {
    lcd.setCursor(15,1);
    lcd.write(6);          //DISPLAY THE SD NOT CONNECTED ICON
  }
}

void startServer()
{
   if(WiFi.status()==WL_CONNECTED && SD.exists("/index.html")==true && serverStatus==0)
  { 

    Serial.println("Server Started");
                  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/index.html", "text/html");
  });
  //testing start
//   server.on("/get-sensor-data", HTTP_GET, [](AsyncWebServerRequest *request){
    
//      String JSON_Data = "{\"temp\":";
//          JSON_Data += tempInC;
//          JSON_Data += ",\"hum\":";
//          JSON_Data += humidity;
//          JSON_Data += ",\"moisOne\":";
//          JSON_Data += moisValue1_Percent;
//          JSON_Data += ",\"moisTwo\":";
//          JSON_Data += moisValue2_Percent;
//          JSON_Data += "}";
//     request->send(200, "application/json", JSON_Data);
// });

  //testing end
                    server.serveStatic("/", SD, "/");
                    server.onNotFound(notFound);
                    server.begin();
                    websockets.begin();
                    websockets.onEvent(webSocketEvent);
                    serverStatus=1;
  }  
}

void setup() {

  Serial.begin(115200);
  //_______________________________________PUMPS__________________________________[+]
  pinMode(pump1,OUTPUT);
  pinMode(pump2,OUTPUT);
  digitalWrite(pump1,LOW);
  digitalWrite(pump2,LOW);
   //______________________________________________________________________________[-]



  //_______________________________________DHT11__________________________________[+]
  tempSensor.begin();
  //______________________________________________________________________________[-]


  // intitialize();
  analogSetAttenuation(ADC_11db);
  //________________________________________LCD INIT_____________________________________[+]
  lcd.init();
  lcd.backlight();



   //________________________________________LCD INTRO_____________________________________[+]
  lcd.setCursor(3,0);
  lcd.print("Smart Soil");
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print("Irrig. Sys. v1.0");
  delay(3000);
  lcd.clear();
  //________________________________________________________________________________[-]



  //________________________________________LCD CUSTOM CHARACTERS___________________[+]
  lcd.createChar(0,degreeSymbol);
  //_________________________________________________________________________________[-]
  connectWifi();


  //________________________________________SD CARD___________________[+]

 
}

void loop() 
{
currentTime=millis();
if(WiFi.status()!=WL_CONNECTED)
{
  serverStatus=0;
connectWifi();
}
                            if(count<1 && WiFi.status()==WL_CONNECTED)
                            {
                              count++;
                              lcd.clear();
                              lcd.setCursor(3,0);
                              lcd.print("Web Server:");
                              lcd.setCursor(3,1);
                              lcd.print(WiFi.localIP());
                              delay(3000);
                              lcd.clear();
                            }
readSensorData();
creatCharacters();
lcdDisplay();
if(!serverStatus)
startServer();


if((unsigned long)(currentTime-sendDataPrevTime)>=sendDataInterval){
  if(serverStatus)
  send_sensor();
  sendDataPrevTime=currentTime;
}
websockets.loop();


// displayTempValues();
}

