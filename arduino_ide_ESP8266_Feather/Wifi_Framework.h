// DESCRIPTION
//
// Etablir la connection WIFI
//
// ENTREE :
//    const char* ssid : Nom du réseau (pointeur de string)
//    const char* password : Password du réseau (pointeur de string)
//
// SORTIE :
//        true : connection OK  
//        false : connection KO

bool Wifi_Connection(const char* ssid, const char* password,int idx)
{
 IPAddress IP(Rx_By_1,Rx_By_2,Rx_By_3,100+idx); 
 
 //  connection au WIFI
  //WiFi.begin(ssid, password);
  WiFi.begin(ssid);
  WiFi.config(IP,GW,SubNet);
 // timeout = 120x500ms=1min
  int timeout=30;
  int i=0;
  // tant que le status du wifi est deconnecté on boucle pendant 1min
  while (WiFi.status() != WL_CONNECTED) {

#if defined(DEBUG) && DEBUG == 1       
    Serial.print(".");
#endif    

    delay(500);
    i=i+1;
    if (i==timeout)
    {
      // Si timeout on retourne une  erreur

#if defined(DEBUG) && DEBUG == 1   
      Serial.println("");       
      Serial.println("Wifi_Connection - ERROR to connect to ssid");
      Serial.println("Wifi-framework deep sleep");       
#endif
      ESP.deepSleep(1, RF_CAL);// sleep for 1ms and reset
      return(false);
    }
  }
 

  // Si la connection a été faite, on retourne l'IP
#if defined(DEBUG) && DEBUG == 1        
  Serial.print("Wifi_Connection - ESP IP=");  
  Serial.println(WiFi.localIP());
#endif  
  return(true);
}
