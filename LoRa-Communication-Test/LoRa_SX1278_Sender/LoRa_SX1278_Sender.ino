#include <LoRa.h>

// Pines TRANSMISOR
const int loraRST = 23;
const int loraDI0 = 7;  // DIO0
const int loraNSS = 15;
const int loraMOSI = 32;
const int loraMISO = 35;
const int loraSCK = 33;

int SyncWord = 0x22;
int dataToSend = 0;  // Solo para test (alamacenar)

void setup() {
  Serial.begin(115200);
  
  while (!Serial);
  Serial.println("Ingresa un numero para enviar:");

  // SPI
  SPI.begin(loraSCK, loraMISO, loraMOSI, loraNSS);
  
  // Configurar pines LoRa
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
  // Leer monitor serial
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    // Convertir numeros
    if (input.length() > 0) {
      dataToSend = input.toInt();
      
      // Enviar el dato por LoRa
      Serial.print("Enviando: ");
      Serial.println(dataToSend);
      
      LoRa.beginPacket();
      LoRa.print(dataToSend);
      LoRa.endPacket();
      
      Serial.println("Exitoso!");
      Serial.println("---------------------------------");
      Serial.println();
      Serial.println("Ingresa otro numero para enviar:");
    }
  }
  
  delay(100); // evitar sobrecarga
}