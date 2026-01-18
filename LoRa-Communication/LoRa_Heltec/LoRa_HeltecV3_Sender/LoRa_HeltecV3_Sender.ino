#include "Arduino.h"
#include "LoRaWan_APP.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"

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

SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED,
                    GEOMETRY_128_64, RST_OLED);

void OnTxDone() {
  Serial.println("TX OK");
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

  int idx1 = line.indexOf(',');
  int idx2 = line.indexOf(',', idx1 + 1);
  int idx3 = line.indexOf(',', idx2 + 1);
  int idx4 = line.indexOf(',', idx3 + 1);

  if (idx4 == -1) return false;

  payload.num1 = line.substring(0, idx1).toInt();
  payload.num2 = line.substring(idx1 + 1, idx2).toInt();
  payload.num3 = line.substring(idx2 + 1, idx3).toFloat();
  payload.num4 = line.substring(idx3 + 1, idx4).toFloat();

  memset(payload.text, 0, sizeof(payload.text));
  line.substring(idx4 + 1).toCharArray(payload.text, 6);

  return true;
}

void setup() {
  Serial.begin(115200);
  display.init();
  display.clear();
  display.display();
  lora_init();

  Serial.println("Formato: int,int,float,float,text");
}

void loop() {
  if (state == STATE_TX && readSerialPayload()) {
    uint32_t t = millis();

    display.clear();
    display.drawString(0, 0, "V3 TX");
    display.drawString(0, 12, payload.text);
    display.drawString(0, 24, "ms:");
    display.drawString(30, 24, String(t));
    display.display();

    Serial.printf(
      "TX [%lu ms] -> %d,%d,%.2f,%.2f,%s\n",
      t, payload.num1, payload.num2,
      payload.num3, payload.num4, payload.text
    );

    Radio.Send((uint8_t*)&payload, sizeof(Payload));
    state = LOWPOWER;
  }

  if (state == LOWPOWER) {
    Radio.IrqProcess();
  }
}
