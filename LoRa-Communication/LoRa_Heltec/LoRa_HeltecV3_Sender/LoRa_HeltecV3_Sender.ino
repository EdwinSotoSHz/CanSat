#include "Arduino.h"
#include "LoRaWan_APP.h"

/********************************* LoRa Config *********************************************/
#define RF_FREQUENCY 868000000
#define TX_OUTPUT_POWER 10
#define LORA_BANDWIDTH 0
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

struct Payload {
  int16_t num1;
  int16_t num2;
  float   num3;
  float   num4;
  char    text[6];
};

Payload payload;

static RadioEvents_t RadioEvents;

typedef enum { LOWPOWER, STATE_TX } States_t;
States_t state;

void OnTxDone() {
  Serial.println("TX DONE");
  state = STATE_TX;
}

void OnTxTimeout() {
  Serial.println("TX TIMEOUT");
  Radio.Sleep();
  state = STATE_TX;
}

void lora_init() {
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0,
                    LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, LORA_PREAMBLE_LENGTH,
                    LORA_FIX_LENGTH_PAYLOAD_ON, true,
                    0, 0, LORA_IQ_INVERSION_ON, 3000);

  state = STATE_TX;
}

bool readSerialPayload() {
  if (!Serial.available()) return false;

  String line = Serial.readStringUntil('\n');
  line.trim();

  int i1 = line.indexOf(',');
  int i2 = line.indexOf(',', i1 + 1);
  int i3 = line.indexOf(',', i2 + 1);
  int i4 = line.indexOf(',', i3 + 1);

  if (i4 == -1) return false;

  payload.num1 = line.substring(0, i1).toInt();
  payload.num2 = line.substring(i1 + 1, i2).toInt();
  payload.num3 = line.substring(i2 + 1, i3).toFloat();
  payload.num4 = line.substring(i3 + 1, i4).toFloat();

  memset(payload.text, 0, sizeof(payload.text));
  line.substring(i4 + 1).toCharArray(payload.text, 6);

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  lora_init();

  Serial.println("=== V3 EMISOR ===");
  Serial.println("Formato: int,int,float,float,text");
  Serial.println("Ejemplo: 1,2,3.3,4.4,qwert");
}

void loop() {
  if (state == STATE_TX && readSerialPayload()) {
    uint32_t t = millis();

    Serial.printf(
      "[TX %lu ms] %d,%d,%.2f,%.2f,%s\n",
      t,
      payload.num1,
      payload.num2,
      payload.num3,
      payload.num4,
      payload.text
    );

    Radio.Send((uint8_t*)&payload, sizeof(Payload));
    state = LOWPOWER;
  }

  if (state == LOWPOWER) {
    Radio.IrqProcess();
  }
}
