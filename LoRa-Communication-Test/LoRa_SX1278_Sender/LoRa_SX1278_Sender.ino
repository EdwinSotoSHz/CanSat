#include <LoRa.h>

// pines TRANSMISOR
const int loraRST = 23;
const int loraDI0 = 7;
const int loraNSS = 15;
const int loraMOSI = 32;
const int loraMISO = 35;
const int loraSCK = 33;

int SyncWord = 0x22;
String dataToSend = "";

void setup() {
  Serial.begin(115200);
  
  while (!Serial);
  Serial.println("LoRa Transmitter ESP32");
  Serial.println("Listo para enviar texto");
  Serial.println("-----------------------");

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

  Serial.println("Configuracion completa");
  Serial.println();
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() > 0) {
      dataToSend = input;
      
      Serial.print("Enviando: \"");
      Serial.print(dataToSend);
      Serial.println("\"");
      
      // Medir tiempo de envio
      unsigned long startTime = millis();
      
      LoRa.beginPacket();
      LoRa.print(dataToSend);
      int result = LoRa.endPacket();
      
      unsigned long endTime = millis();
      unsigned long sendTime = endTime - startTime;
      
      if (result == 1) {
        Serial.print("Envio exitoso (");
        Serial.print(sendTime);
        Serial.println(" ms)");
      } else {
        Serial.println("Error en envio");
      }
      
      Serial.println("------------------------");
      Serial.println("");
    }
  }
  
  delay(100);
}