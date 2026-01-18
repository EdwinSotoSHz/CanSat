// Code for Heltec V3 with Heltec V2 compatibility
#include "Arduino.h"
#include "LoRaWan_APP.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"

/********************************* LoRa Config *********************************************/
#define RF_FREQUENCY        868000000
#define TX_OUTPUT_POWER     10
#define LORA_BANDWIDTH      0
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE     1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define BUFFER_SIZE 30

char txpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

typedef enum {
    LOWPOWER,
    STATE_TX
} States_t;

States_t state;
int16_t txNumber = 0;

SSD1306Wire factory_display(0x3c, 500000, SDA_OLED, SCL_OLED,
                            GEOMETRY_128_64, RST_OLED);

void OnTxDone(void) {
    Serial.println("TX done");
    state = STATE_TX;
}

void OnTxTimeout(void) {
    Serial.println("TX timeout");
    Radio.Sleep();
    state = STATE_TX;
}

void lora_init(void) {
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

void setup() {
    Serial.begin(115200);
    factory_display.init();
    factory_display.clear();
    factory_display.display();
    lora_init();
}

void loop() {
    switch (state) {
        case STATE_TX:
            delay(1000);
            txNumber++;
            sprintf(txpacket, "V3:%d", txNumber);

            factory_display.clear();
            factory_display.drawString(0, 0, "V3 TX");
            factory_display.drawString(0, 12, txpacket);
            factory_display.display();

            Serial.printf("Enviando: %s\n", txpacket);
            Radio.Send((uint8_t*)txpacket, strlen(txpacket));
            state = LOWPOWER;
            break;

        case LOWPOWER:
            Radio.IrqProcess();
            break;
    }
}
