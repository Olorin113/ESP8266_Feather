
// DESCRIPTION
//
// Met à jour le capteur via l'api de domoticz
//
// ENTREE :
//
// domoticz_server : IP du serveur Domoticz
// domoticz_port : Port du serveur Domoticz
// idx: Index du capteur à mettre à jour
//
// SORTIE
//
// 0 : Mise à jour OK
// 1 : Erreur de connection au serveur domoticz

bool UpdateCapteurDomoticz(WiFiClient client,int SSL_MODE,const char* domoticz_server,int domoticz_port,int idx,String temp,String humidity,int Battery_Level){

#if defined(DEBUG) && DEBUG == 1      
    Serial.println("UpdateCapteurDomoticz - Update DOMOTICZ");
#endif  
    client.print("GET /json.htm?type=command&param=udevice&idx=");
    client.print(idx);
    client.print("&nvalue=0&svalue=");    
    client.print(temp);
    client.print(";");


    client.print(humidity);
    client.print(";0"); //Value for HUM_STAT. Can be one of: 0=Normal, 1=Comfortable, 2=Dry, 3=Wet
    client.print("&battery=");
    client.print(Battery_Level);
    
  if (SSL_MODE==0)
  {
    client.println(" HTTP/1.1");
  }
    client.print("Host: ");
    client.print(domoticz_server);
    client.print(":");
    client.println(domoticz_port);
    client.println("User-Agent: Arduino-ethernet");
    client.println("Connection: close");
    client.println();

#if defined(DEBUG) && DEBUG == 1      
  Serial.println("UpdateCapteurDomoticz - OK");
#endif
  
  return(true);
}
