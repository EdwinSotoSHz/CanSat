#include <LoRa.h>

// pines TRANSMISOR
const int loraRST = 23;
const int loraDI0 = 7;
const int loraNSS = 15;
const int loraMOSI = 32;
const int loraMISO = 35;
const int loraSCK = 33;

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

  Serial.println("Listo (formato: int,int,float,float,text)");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    // Validacion para test (cambiar por expresion regular)
    int comma1 = input.indexOf(',');
    int comma2 = input.indexOf(',', comma1 + 1);
    int comma3 = input.indexOf(',', comma2 + 1);
    int comma4 = input.indexOf(',', comma3 + 1);
    if (comma1 == -1 || comma2 == -1 || comma3 == -1 || comma4 == -1) {
      Serial.println("Formato invalido (,)");
      return;
    }

    //Usar nueva estructura numeros y char
    Payload cansatData;
    cansatData.num1 = input.substring(0, comma1).toInt();
    cansatData.num2 = input.substring(comma1 + 1, comma2).toInt();
    cansatData.num3 = input.substring(comma2 + 1, comma3).toFloat();
    cansatData.num4 = input.substring(comma3 + 1, comma4).toFloat();    
    String textStr = input.substring(comma4 + 1);
    textStr.toCharArray(cansatData.text, 6);
    for (int i = textStr.length(); i < 5; i++) { // Rellenar con espacios si no cumple con 5 caracteres 
      cansatData.text[i] = ' ';
    }

    // tiempo transcurrido
    unsigned long t0 = millis();

    // Enviar la estructura en formato binario
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&cansatData, sizeof(cansatData));
    LoRa.endPacket();

    // Imprimir
    Serial.print("Enviado:");
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
    Serial.print("(");
    Serial.print(millis() - t0);
    Serial.println(" ms)");
    Serial.println("---------------------------------");
    Serial.println("");

  }
  delay(15);
}

// 1,222,3.3,4.4,qwert