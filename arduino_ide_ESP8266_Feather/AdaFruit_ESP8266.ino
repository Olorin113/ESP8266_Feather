//  ----------------------------------
// Autheur : Rodolphe rondeaux
//  ----------------------------------
// DATE : 29/09/2017
//  ----------------------------------
// Historique :
// Version 1.0  - 29/09/2017 - Creation du programme
// Version 1.1  - 09/10/2017 - Deepsleep=0 si batterie <= 3V 
// Version 1.2  - 10/10/2017 - IDX déclaré en auto ou en manuel au  début du programme car plusieurs ESP oont la même adresse MAC. (bug de programmation...)
// Version 1.2a - 11/10/2017 - pin0 à low au début pour viter que la led rouge ne s'allume
// Version 1.3  - 13/10/2017 - Double lecture du DHT22, pour vider le cache
// Version 1.3a - 16/10/2017 - résolution du bug de plusieurs adress mac
// Version 1.4  - 22/11/2017 - ajout GPIO12 pour timer tlp5110
//  ----------------------------------
// N°Module : Version installée
// IDX 4 : v1.4
// IDX 10: v1.3a
// IDX 11: v1.3a
// IDX 12: v1.3a
// IDX 13: v1.4 max adc 665mv 
// IDX 14: v1.3a max adc  
//  ----------------------------------

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
extern "C" {
#include "user_interface.h"
}
#include <Arduino.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "DHT.h"

// MODE DEBUG 
// 1 = ACTIVEE (sur port serie 9600bauds)
// 0 = DESACTIVE 
#define DEBUG 1

//Version du Programme
#define Version "1.4"

// ESP DEEP SLEEP CONFIGURE IN SECOND
//const int sleepTimeS =2;
const int sleepTimeS =1190;

#define DATA_DHT22 2 // what pin we're connected to IO2 (pin 11 on board) for DHT22, don't forget to set POWER_DHT22
#define POWER_DHT22 15 // what pin we're connected +3.3v DHT22: IO0 (pin 12 on board), don't forget to set DATA_DHT22
#define GATE_MOSFET 5 // what pin we're connected to the GAte pin mosfet : IO5 (pin 14 on board). for voltage devider measure
#define DONE_TPL5110 16 // what pin we're connected to the DONE pin of nanopower TPL5110 : IO12 (pin 4 on the board)

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301

// ----------
// Initialize DHT sensor.
// ----------
DHT dht(DATA_DHT22, DHTTYPE);

// ------------ 
// CONFIG DOMOTICZ
// ------------ 
const int Rx_By_1=192;
const int Rx_By_2=168;
const int Rx_By_3=43;
const char * domoticz_server="192.168.43.1";
IPAddress SubNet(255,255,255,0);
IPAddress GW(Rx_By_1,Rx_By_2,Rx_By_3,1);
const int domoticz_port=8080;

bool RetourCnxDomoticz=false;

#include "Wifi_Framework.h"
#include "Domoticz_Framework.h"
#include "mac_idx.h"

// MODE SECURE
// 1 = ACTIVEE (SSL/HTTPS)
// 0 = DESACTIVE (HTTP)
#define SSL_MODE 0

// ----------
//  CONFIG WIFI
// ----------
const char* ssid = "RasberryRd";
const char* password = "";
WiFiServer server(80);
// ------------ ----- ------------
// -------- SUB PROGRAMS ---------
// ------------ ----- ------------

int Get_Battery_level(){
  //Activer le pont diviser de tension à travers un MOSFET branché sur un I/O.
  //

  // récupérer la tension sur l'entrée analogique
  uint32 VCC_A0=analogRead(A0);
#if defined(DEBUG) && DEBUG == 1       
  Serial.print("Voltage A0 : ");
  Serial.print(VCC_A0);
  Serial.println("mV");
//  while (true)
 // {
 // delay(100000);
 // }
#endif  
  //Calculer le niveau de batterie : R1=1.2Mohm R2=220Kohm 
  // 0.635V=100% 0.498V=0%
  // 635-498=137
  //max 628 => idx4
  //min 290 => idx4
  //100 * (VCC-635) /137
  int max_bat=630;
  int min_bat=450;
  int Battery_Level=0;

  //Si tension suppérieur au 105% du max
  if (VCC_A0 > (max_bat*1.05) )
  {
#if defined(DEBUG) && DEBUG == 1     
    Serial.print(VCC_A0);
    Serial.print(" : upper than 100% =>");
    Serial.println(max_bat);    
#endif    
    return(100);
  }
  //Si tension inférieur au 95% du min
  else if (VCC_A0 < (min_bat*0.95) )
  { 
#if defined(DEBUG) && DEBUG == 1     
    Serial.print(VCC_A0);
    Serial.print(" : less than 95% of min_bat:");
    Serial.println(min_bat);
    Serial.println("return battery_level=0");   
#endif
    return(0);
  }
  else
  {
     Battery_Level=((VCC_A0-min_bat)*100)/(max_bat-min_bat);
#if defined(DEBUG) && DEBUG == 1     
     Serial.print(Battery_Level);
     Serial.println("% Battery");
#endif     
     return(Battery_Level);
  }
  
}


//----------------------------
// SENT TO DOMOTICZ
//----------------------------
// desc : Envoi des données relevées vers domoticz
//----------------------------
// IN :
//    temp : température en format string avec 1 chiffre derrière la virgule (eg:"28.2")
//    humidity : température en format string avec 1 chiffre derrière la virgule (eg:"28.2")
//    VCC : valeurr de l'entrée analogique ADC A0 (0 à 1024) pour connaître la tension de la batterie

// OUT: 
//  true : ennvoie à domoticz OK
//  true : ennvoie à domoticz KO
//----------------------------


bool SendToDomoticz(String temp,String humidity,int Battery_level){

  //On allume le WIFI car désactivé après le deepsleep  
 
  WiFi.forceSleepWake();
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFiClient client;


//GET MAC AADRESS to determine IDX of domoticz
  unsigned char mac[6];
  
  //WiFi.softAPmacAddress(mac); Autre version pour récupérer l'adress mac != de la version ci-dessous.
  WiFi.macAddress(mac);  
  int IDX = GetIdx(mac);
  
  IPAddress IP(192,168,43,IDX);
#if defined(DEBUG) && DEBUG == 1       
  Serial.print("Connect to WIFI");  
#endif 
  
  bool retour=Wifi_Connection(ssid,password,IDX);
  if (retour==false)
  {
    
#if defined(DEBUG) && DEBUG == 1         
     Serial.println("KO - Connect to WIFI");
#endif 

    return(false);
  }

#if defined(DEBUG) && DEBUG == 1       
  Serial.println("check if wifi server is available");
#endif  

  if (client.connect(domoticz_server,domoticz_port)) 
  {
    UpdateCapteurDomoticz(client,SSL_MODE,domoticz_server,domoticz_port,IDX,temp,humidity,Battery_level);
    client.stop();
  }
  //On éteint le wifi:
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);   
 
  return(true);
}

// ------------ ----- ------------
// ------------ SETUP ------------
// ------------ ----- ------------
void setup() {
  // Activation du watchdog
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);

#if defined(DEBUG) && DEBUG == 1   
  Serial.begin(74880);
  Serial.print("Version  ");    
  Serial.println(Version);    
  Serial.println("Read DHT sensor!");
#endif
  //GPIO 12 active bas
  pinMode(DONE_TPL5110, OUTPUT);
  digitalWrite(DONE_TPL5110,LOW);
          
  //pinMode(0, OUTPUT); // Activer la sortie sur pin 12 - I/O 0 pour éviter que la led rouge ne s'allume
  //digitalWrite(0,LOW);

  // Activer les MOSFET pour les lectures du pont diviseur de tension
  pinMode(GATE_MOSFET, OUTPUT);
  digitalWrite(GATE_MOSFET,HIGH);
  // Activer l'I/O pour l'alim du dht22
  pinMode(POWER_DHT22, OUTPUT);
  digitalWrite(POWER_DHT22,HIGH);


  
  delay(100);
  // récupérer la tension de l'esp8266  
  int Battery_Level=Get_Battery_level();  

  // Désactiver les MOSFET pour les lectures du pont diviseur de tension
  digitalWrite(GATE_MOSFET,LOW);

  int retry = 0;
  while(retry<2){

    // reset Watchdog
    ESP.wdtFeed();
    retry++;  
        
   delay(500);

   // LLire 2x et ne prendre que la 2ème lecture, pour vider le cache et avoir la nouvelle valeur

    dht.readHumidity();
    dht.readTemperature();
    dht.readTemperature(true);
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float humidity=dht.readHumidity();
    // Read temperature as Celsius (the default)
    float temp=dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f=dht.readTemperature(true);
     
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp) ) {
      
#if defined(DEBUG) && DEBUG == 1        
      Serial.println("Failed to read from DHT sensor! (wait 1s)");
#endif       
   
      temp=0;
      f=0;
      humidity=0;
      delay(1000);
      
#if defined(DEBUG) && DEBUG == 1        
      Serial.println("Retry to read from DHT sensor!");
#endif         
    }
    else if (retry < 2)
    {
      digitalWrite(POWER_DHT22,LOW);
      
        
      //Si  la temperature est correctement lu, on met le retry a 3, on envoie l'info a domotics, et on sort de la boucle
      retry=3;
      
      //Connvertion float to String de l'humidité et temperature
      char buff[10];
      dtostrf(temp, 4, 2, buff);      
      String StrTemp=buff;
      dtostrf(humidity, 4, 2, buff);
      String StrHumidity=buff;
      
      RetourCnxDomoticz=SendToDomoticz(StrTemp,StrHumidity,Battery_Level);
          
#if defined(DEBUG) && DEBUG == 1

      // Compute heat index in Fahrenheit (the default)
      float hif = dht.computeHeatIndex(f, humidity);
      // Compute heat index in Celsius (isFahreheit = false)
      float hic = dht.computeHeatIndex(temp, humidity, false);

      Serial.print("Humidity: ");
      Serial.print(StrHumidity);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(StrTemp);
      Serial.print(" *C ");
      Serial.print(f);
      Serial.print(" *F\t");
      Serial.print("Heat index: ");
      Serial.print(hic);
      Serial.print(" *C ");
      Serial.print(hif);
      Serial.println(" *F");
#endif
    }
  }

#if defined(DEBUG) && DEBUG == 1    
 // Si l'envoie de la temperature a domoticz a echouee, on sort une erreur
  if (RetourCnxDomoticz==false)
  {
      Serial.println("Join Domoticz KO");
  }   
  Serial.println("deep sleep");

#endif      
    
//  if (Battery_Level == 0 )
//  {
#if defined(DEBUG) && DEBUG == 1    
 //   Serial.println("WARNING = BATTERY ULTRA LOW => DEEPSLEEP 0 ");    
#endif        
 //   ESP.deepSleep(0,WAKE_RF_DEFAULT);
 // }
 // else
 // {
    // ESP.deepSleep(sleepTimeS * 1000000,WAKE_RF_DEFAULT);
    digitalWrite(DONE_TPL5110,HIGH);
    delay(2000);
    digitalWrite(DONE_TPL5110,LOW);
 // }
}


// ------------ ----- ------------
// ------------  LOOP ------------
// ------------ ----- ------------
void loop() {Serial.println("loop"); delay(2000);    digitalWrite(DONE_TPL5110,HIGH);
    delay(1000);
    digitalWrite(DONE_TPL5110,LOW);
    }
