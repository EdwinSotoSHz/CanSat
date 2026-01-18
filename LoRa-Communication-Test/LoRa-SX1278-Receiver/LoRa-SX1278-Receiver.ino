#include <LoRa.h>

// pines RECEPTOR
const int loraRST = 15;
const int loraDI0 = 2;
const int loraNSS = 5;
const int loraMOSI = 18;
const int loraMISO = 22;
const int loraSCK = 23;

int SyncWord = 0x22;

// Mejor estructura
struct Payload {
  int16_t num1;  // 2 bytes (-999 a 999)
  int16_t num2; // 2 bytes (-999 a 999)
  float num3; // 4 bytes
  float num4;  // 4 bytes
  char text[6]; // 5 caracteres max
};

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // SPI
  SPI.begin(loraSCK, loraMISO, loraMOSI, loraNSS);
  LoRa.setPins(loraNSS, loraRST, loraDI0);

  // 433mhz
  if (!LoRa.begin(433E6)) {
    while (1);
  }

  // Configurar parámetros con mejor optimización (aun confiable, pero solo para test)
  LoRa.setSpreadingFactor(9);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(6);
  LoRa.setSyncWord(SyncWord);

  Serial.println("Listo");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize == sizeof(Payload)) {

    //Recibir nueva estructura numeros y char
    Payload cansatData;
    LoRa.readBytes((uint8_t*)&cansatData, sizeof(cansatData));

    // Imprimir
    Serial.print("Recibido:");
    Serial.print("\n num1 = ");
    Serial.print(cansatData.num1);
    Serial.print("\n num2 = ");
    Serial.print(cansatData.num2);
    Serial.print("\n num3 = ");
    Serial.print(cansatData.num3);
    Serial.print("\n num4 = ");
    Serial.print(cansatData.num4);
    Serial.print("\n char[5] = ");
    Serial.print(cansatData.text);

    Serial.println();
    Serial.print("RSSI =");
    Serial.print(LoRa.packetRssi());
    Serial.print(" | SNR =");
    Serial.println(LoRa.packetSnr(), 1);
    Serial.println("---------------------------------");
    Serial.println("");
  }
  delay(10);
}