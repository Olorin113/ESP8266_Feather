//----------------------------
// MAC ADRESS TO STRING FORMAT
//----------------------------
// desc
//----------------------------
// IN : MAC address (ref of table)
// OUT: MAC address in format string
//----------------------------

String macToStr(const uint8_t* mac){
  
  String result;
  for (int i = 0; i < 6; ++i) {    
    if (mac[i] < 16)
    {        
        result += "0";
    }
    result += String(mac[i], 16);
    if (i < 5)
        result += ':';    
  }
  result.toUpperCase();
  return result;
}


//----------------------------
// GETIDX
//----------------------------
// desc : Get idx domoticz from  mac address
//----------------------------
// IN : MAC address (ref of table)
// OUT: IDX selon Domoticz 
//      IP du module
//----------------------------

int GetIdx(const uint8_t* mac){

  int IDX=0;
  String clientMac ="";
  clientMac = macToStr(mac);
 
  if (clientMac == "5C:CF:7F:F0:B1:0F")
  {
        IDX=11;
  }      
  else if (clientMac == "5C:CF:7F:EF:C1:BF")
  {
        IDX=4;
  }
  else if (clientMac == "5C:CF:7F:A4:3E:32")
  {
        IDX=10;
  }
  else if (clientMac == "5C:CF:7F:EF:B4:CB")
  {
        IDX=12;
  }  
  else if (clientMac == "5C:CF:7F:F0:B2:47")
  {
        IDX=13;
  }
  else if (clientMac == "5C:CF:7F:EF:C0:C0")
  {
        IDX=14;
  }
#if defined(DEBUG) && DEBUG == 1 
  Serial.print("IDX: ");
  Serial.print(IDX);
  Serial.print(" - MAC: ");
  Serial.println(clientMac);
#endif   
  return(IDX);
}


