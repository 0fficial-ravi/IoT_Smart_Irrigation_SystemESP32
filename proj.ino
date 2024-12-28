#include <DHT.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>


const char* ssid       = "Word.exe";
const char* password   = "Cat@12345";

#define PIN_SPI_CS 5 // The ESP32 pin GPIO5 FOR SD CARD MODULE
#define tempSensorPin 15
#define mSensor1 32 
#define mSensor2 33
#define pump1 25
#define pump2 14
#define dry 2680
#define wet 960

File myFile; //FILE HANDLER FOR SD CARD

const int lcdColumns = 16;
const int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 


DHT tempSensor(tempSensorPin,DHT11);


float tempInC,tempInF,humidity;
int moisValue1,moisValue2;
int moisValue1_Percent,moisValue2_Percent;
unsigned long TempReadinterval=1500,TempPrevTime=0,currentTime;
unsigned long HumidityReadinterval=2500,HumidityPrevTime=0;
unsigned long TempDisplayInterval=3000,TempDisplayPrevTime=0;
unsigned long MoistureReadinterval=100,MoisturePrevTime=0;
unsigned long MoistureDisplayInterval=1000,MoistureDisplayPrevTime=0;
unsigned long wifiConnectInterval=1000,wifiConnectPrevTime=0;
bool sd_status=false;



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


// void intitialize()
// {
// TempReadinterval=1500;
// TempPrevTime=0;

// HumidityReadinterval=2500;
// HumidityPrevTime=0;

// MoistureReadinterval=100;
// MoisturePrevTime=0;

// wifiConnectInterval=1000;
// wifiConnectPrevTime=0;

// TempDisplayInterval=3000;
// TempDisplayPrevTime=0;

// MoistureDisplayInterval=1000;
// MoistureDisplayPrevTime=0;

// tempInC=0.0;
// tempInF=0.0;
// humidity=0.0;
// moisValue1=0;
// moisValue2=0;
// moisValue1_Percent=0;
// moisValue2_Percent=0;
// }
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
      if(!SD.exists("/testStatus.txt"))
      {
      // Serial.println("File not exists");
      sd_status=false;
      }
    }
}
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
           digitalWrite(pump1,HIGH);  //ACTIVATE PUMP 1
    }
    else if(moisValue1_Percent>10 && moisValue1_Percent<100)
    {   
          for(int j=12;j<=12;j++)
          {
            lcd.setCursor(j,0);
            lcd.print(" ");
          }
          digitalWrite(pump1,LOW);  //DE-ACTIVATE PUMP 1
    }
   if(moisValue2_Percent<10)
    {   
          for(int j=11;j<=12;j++)
          {
            lcd.setCursor(j,1);
            lcd.print(" ");
          }
          digitalWrite(pump2,HIGH);   //ACTIVATE PUMP 2
    }
    else if(moisValue2_Percent>10 && moisValue2_Percent<100)
    {   
          for(int j=12;j<=12;j++)
          {
            lcd.setCursor(j,1);
            lcd.print(" ");
          }
          digitalWrite(pump2,LOW);  //DE-ACTIVATE PUMP 2
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
    }
}

// void displayTempValues()
// {
//   if((unsigned long)(currentTime-TempDisplayPrevTime)>=TempDisplayInterval)
//   {
//   Serial.println(".........................");
//   Serial.print("deg-C :");
//   Serial.println(tempInC);
//   Serial.print("deg-F :");
//   Serial.println(tempInF);
//   Serial.print("Humidity :");
//   Serial.print(humidity);
//   Serial.println("%");

//   Serial.print("Moisture 1 Raw :");
//   Serial.print(moisValue1);
//   Serial.print(",");
//   Serial.print(moisValue1_Percent);
//   Serial.println("%");


//   Serial.print("Moisture 2 Raw :");
//   Serial.print(moisValue2);
//   Serial.print(",");
//   Serial.print(moisValue2_Percent);
//   Serial.println("%");
 


//   TempDisplayPrevTime=currentTime;
//   }
// }

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
    lcd.setCursor(15,1);
    lcd.write(6);
  }
  else {
    lcd.setCursor(15,1);
    lcd.write(6);
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

void loop() {

currentTime=millis();
connectWifi();
readSensorData();
// displayTempValues();
creatCharacters();
lcdDisplay();
}

