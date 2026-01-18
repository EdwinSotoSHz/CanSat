#include <LoRa.h>

// pines RECEPTOR
const int loraRST = 15;
const int loraDI0 = 2;
const int loraNSS = 5;
const int loraMOSI = 18;
const int loraMISO = 22;
const int loraSCK = 23;

int SyncWord = 0x22;
String lastReceived = "";
int packetCount = 0;

void setup() {
  Serial.begin(115200);
  
  while (!Serial);
  Serial.println("LoRa Receiver ESP32");
  Serial.println("Esperando...");
  Serial.println("--------------");

  // SPI
  SPI.begin(loraSCK, loraMISO, loraMOSI, loraNSS);
  LoRa.setPins(loraNSS, loraRST, loraDI0);

  // 433mhz
  if (!LoRa.begin(433E6)) {
    Serial.println("Error iniciando LoRa");
    while (1);
  }
  
  // Configurar parámetros
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(62.5E3);
  LoRa.setCodingRate4(8);
  LoRa.setSyncWord(SyncWord);
  
  Serial.println("Listo para recibir");
  Serial.println();
}

void loop() { 
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) { 
    packetCount++;
    String receivedData = "";
    
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    
    // Parametros de calidad
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    int packetError = LoRa.packetFrequencyError();
    
    // Calcular porcentaje aproximado de calidad (formula ya existente)
    // RSSI mejor cuanto más cercano a 0
    int quality = 0;
    if (rssi > -50) quality = 100;
    else if (rssi > -70) quality = 80;
    else if (rssi > -90) quality = 60;
    else if (rssi > -110) quality = 40;
    else quality = 20;
    
    // Informacion del paquete
    Serial.print("Paquete #");
    Serial.print(packetCount);
    Serial.print(" [");
    Serial.print(packetSize);
    Serial.print(" bytes]: \"");
    Serial.print(receivedData);
    Serial.println("\"");
    
    // Parametros de calidad
    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.print(" dBm | SNR: ");
    Serial.print(snr, 1);
    Serial.print(" dB | Calidad: ");
    Serial.print(quality);
    Serial.print("%");
    
    if (packetError != 0) {
      Serial.print(" | Error: ");
      Serial.print(packetError);
      Serial.print(" Hz");
    }
    
    Serial.println();
    
    // Mensajes nuevos (tests)
    if (receivedData != lastReceived) {
      Serial.println("(NEW)");
      lastReceived = receivedData;
    }
    
    Serial.println("------------------------------------------------------");
  }
  
  delay(50);
}