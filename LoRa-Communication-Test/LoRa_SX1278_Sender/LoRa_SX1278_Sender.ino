#include <LoRa.h>

// pines TRANSMISOR
const int loraRST = 23;
const int loraDI0 = 7;
const int loraNSS = 15;
const int loraMOSI = 32;
const int loraMISO = 35;
const int loraSCK = 33;

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

  Serial.println("Transmisor listo");
  Serial.println("Ingresar valor:");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() > 0) {
      lastMessage = input;
      
      // Enviar por LoRa
      LoRa.beginPacket();
      LoRa.print(input);
      LoRa.endPacket();
      
      Serial.print("Enviado: ");
      Serial.println(input);
      Serial.println("-----------------");
      Serial.println();
    }
  }
  
  delay(50);
}