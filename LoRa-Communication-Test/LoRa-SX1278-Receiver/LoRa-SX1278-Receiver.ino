#include <LoRa.h>

// Pines RECEPTOR
const int loraRST = 15;
const int loraDI0 = 2;  // DIO0
const int loraNSS = 5;
const int loraMOSI = 18;
const int loraMISO = 22;
const int loraSCK = 23;

int SyncWord = 0x22;
int lastReceivedValue = 0; // (Solo para test)
int currentValue = 0;

void setup() {
  Serial.begin(115200);
  
  while (!Serial);
  Serial.println("Esperando datos...");

  // SPI
  SPI.begin(loraSCK, loraMISO, loraMOSI, loraNSS);
  
  // pines para LoRa
  LoRa.setPins(loraNSS, loraRST, loraDI0);

  // LoRa en 433MHz
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  // Parámetros LoRa
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(62.5E3);
  LoRa.setCodingRate4(8);
  LoRa.setSyncWord(SyncWord);
  
  Serial.println("LoRa init succeeded.");
  Serial.println();
}

void loop() { 
  // Recibir un paquete
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) { 
    String receivedData = "";
    
    // Leer paquete
    while (LoRa.available()) {
      char inChar = (char)LoRa.read();
      receivedData += inChar;
    }
    
    // Convercion a número
    currentValue = receivedData.toInt();
    
    // Para un valor nuevo
    if (currentValue != lastReceivedValue) {
      lastReceivedValue = currentValue;
      
      // Mostrar
      Serial.print("Nuevo dato recibido: ");
      Serial.println(currentValue);
      
      // Información de señal (test)
      Serial.print("RSSI: ");
      Serial.print(LoRa.packetRssi());
      Serial.print(" dBm, SNR: ");
      Serial.print(LoRa.packetSnr());
      Serial.println(" dB");
      Serial.println("-------------------");
      Serial.println();
      Serial.println("Esperando siguiente dato...");
    }
  }
  
  delay(50); // Pequeño delay para evitar sobrecarga
}