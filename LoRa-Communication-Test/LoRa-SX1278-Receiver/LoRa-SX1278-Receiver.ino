#include <LoRa.h>

// pines RECEPTOR
const int loraRST = 15;
const int loraDI0 = 2;
const int loraNSS = 5;
const int loraMOSI = 18;
const int loraMISO = 22;
const int loraSCK = 23;

int SyncWord = 0x22;
String lastMessage = "";

void setup() {
  Serial.begin(115200);
  
  while (!Serial);
  
  // SPI
  SPI.begin(loraSCK, loraMISO, loraMOSI, loraNSS);
  
  // pines LoRa
  LoRa.setPins(loraNSS, loraRST, loraDI0);

  // 433mhz
  if (!LoRa.begin(433E6)) {
    Serial.println("Error de Inicializacion de LoRa");
    while (1);
  }
  
  // Configurar parámetros
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(62.5E3);
  LoRa.setCodingRate4(8);
  LoRa.setSyncWord(SyncWord);
  
  Serial.println("Receptor listo");
  Serial.println("Esperando mensaje...");
}

void loop() { 
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) { 
    String receivedData = "";
    
    while (LoRa.available()) {
      char inChar = (char)LoRa.read();
      receivedData += inChar;
    }
    
    if (receivedData != lastMessage) {
      lastMessage = receivedData;
      Serial.println(receivedData);
      Serial.println("-----------------");
      Serial.println();
    }
  }
  
  delay(50);
}