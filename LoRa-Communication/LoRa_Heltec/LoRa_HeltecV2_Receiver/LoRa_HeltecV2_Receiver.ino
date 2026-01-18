#include "Arduino.h"
#include "LoRaWan_APP.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"

/********************************* LoRa Config *********************************************/
#define RF_FREQUENCY 868000000
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

typedef enum { LOWPOWER, STATE_RX } States_t;
States_t state;

int16_t Rssi;
int8_t Snr;
bool received = false;

SSD1306Wire factory_display(
  0x3c, 500000, SDA_OLED, SCL_OLED,
  GEOMETRY_128_64, RST_OLED
);

void OnRxDone(uint8_t *data, uint16_t size, int16_t rssi, int8_t snr) {
  if (size == sizeof(Payload)) {
    memcpy(&payload, data, sizeof(Payload));
    Rssi = rssi;
    Snr = snr;
    received = true;
  }
  Radio.Sleep();
  state = STATE_RX;
}

void lora_init() {
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    0, LORA_PREAMBLE_LENGTH,
                    0, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0,
                    LORA_IQ_INVERSION_ON, true);

  state = STATE_RX;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  factory_display.init();
  factory_display.clear();
  factory_display.drawString(0, 0, "V2 RECEPTOR");
  factory_display.drawString(0, 12, "Esperando RX");
  factory_display.display();

  lora_init();
}

void loop() {
  if (received) {
    received = false;
    uint32_t t = millis();

    factory_display.clear();
    factory_display.drawString(0, 0, "V2 RX");
    factory_display.drawString(
    0, 12,
    "n1:" + String(payload.num1) +
    " n2:" + String(payload.num2)
    );
    factory_display.drawString(
    0, 24,
    "n3:" + String(payload.num3, 2)
    );
    factory_display.drawString(
    0, 36,
    "n4:" + String(payload.num4, 2)
    );
    factory_display.drawString(
    0, 48,
    "R:" + String(Rssi) +
    " S:" + String(Snr)
    );
    factory_display.display();

    Serial.printf(
      "[RX %lu ms] %d,%d,%.2f,%.2f,%s | RSSI:%d SNR:%d\n",
      t,
      payload.num1,
      payload.num2,
      payload.num3,
      payload.num4,
      payload.text,
      Rssi,
      Snr
    );
  }

  if (state == STATE_RX) {
    Radio.Rx(0);
    state = LOWPOWER;
  }

  if (state == LOWPOWER) {
    Radio.IrqProcess();
  }
}
