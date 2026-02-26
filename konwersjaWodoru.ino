#include <Adafruit_MAX31856.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

#define buttonMinus 6 //pin do zmniejszania temperatury
#define buttonPlus 7 //pin do zwiększenia temperatury

#define serialClock 13 //musi być ten sam co dla karty sd
#define chipSelect 2 //cs przetwornika max31856
#define diMosi 11 //termopara SDI
#define doMiso 12 //termopara SDO

#define heater 4 //pin do przekaźnika do grzałki
#define heaterLED 5 //pin który pokazuje kiedy jest włączona grzałka

#define sdCardCS 9 //pin cs do karty sd
String fileName="KonWod"; //maksymalnie 8 znaków (nazwa+numer=8)
bool useSDcard=true; //daj na true jeśli chcesz użyć zapisywania pomiarów

const int buttonStep=1;
int setTemperature=20;
double temperature;

unsigned long lastReadTime=0;
unsigned long readInterval=1000;
unsigned long lastClickTime=0;
unsigned long clickInterval=50; //można zmienić jeśli się chce szybciej/wolniej zmieniać temperaturę

Adafruit_MAX31856 thermocouple=Adafruit_MAX31856(chipSelect);
LiquidCrystal_I2C lcd(0x26,16,2);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  if(!thermocouple.begin()){
    Serial.println("Nie znaleziono termopary!");
  }
  Serial.println("Projekt \"Konwersji Wodoru\" koła naukowego \"Grzała\"");

  thermocouple.setThermocoupleType(MAX31856_TCTYPE_K);
  lcd.init();
  lcd.backlight();
  pinMode(buttonMinus,INPUT_PULLUP);
  pinMode(buttonPlus,INPUT_PULLUP);
  pinMode(heater,OUTPUT);
  pinMode(heaterLED,OUTPUT);
  digitalWrite(heater,LOW);
  digitalWrite(heaterLED,LOW);

  pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH);
  pinMode(sdCardCS, OUTPUT);
  digitalWrite(sdCardCS, HIGH);

  if(useSDcard){ //czy karta SD działa
    if(!SD.begin(sdCardCS)){
      Serial.println("Błąd inicjalizacji karty!");
      while(1); //zatrzymaj program
    }
    Serial.println("Karta gotowa!");
    String tempFileName="";
    int fileNumber=1;
    while(1){
      tempFileName=fileName+String(fileNumber); 
      if(!SD.exists(tempFileName+".TXT")){
        fileName=tempFileName;
        break;
      }
      fileNumber++;
    }
  }
}

void loop() {
  if((millis()-lastClickTime)>clickInterval){ //zmiana temperatury
    if(digitalRead(buttonPlus)==LOW){
      setTemperature+=buttonStep;
    }
    if(digitalRead(buttonMinus)==LOW){
      setTemperature-=buttonStep;
    }
    lcd.setCursor(0,0);
    lcd.print("Tmp zd ");
    lcd.print(setTemperature);
    lcd.print("*C");
    lastClickTime=millis();
  }
  
  if((millis()-lastReadTime)>=readInterval){ //odczyt temperatury
    temperature=thermocouple.readThermocoupleTemperature();
    lcd.setCursor(0,1);
    lcd.print("Ak tmp ");
    lcd.print(temperature);
    lcd.print("*C");
    lastReadTime=millis();
    if(useSDcard){ //zapis na kaercie SD
      File myFile=SD.open(fileName+".TXT",FILE_WRITE);
      if(myFile){
        myFile.print(millis());
        myFile.print(" ");
        myFile.println(temperature);
        myFile.close();
        Serial.print("Zapisano temperature: ");
        Serial.println(temperature);
      }else{
          Serial.print("Błąd otwarcia pliku ");
          Serial.println(fileName);
      }
    }
    if(temperature<setTemperature){ //logika do sterowania temperaturą
        digitalWrite(heater,HIGH);
        digitalWrite(heaterLED,HIGH);
    }else{
        digitalWrite(heater,LOW);
        digitalWrite(heaterLED,LOW);
    }
  }
  
}
