#include "Arduino.h"
#include "LoRaWan_APP.h"

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
  delay(500);
  lora_init();

  Serial.println("=== V2 RECEPTOR ===");
}

void loop() {
  if (received) {
    received = false;
    uint32_t t = millis();

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
