#include "Arduino.h"
#include "LoRaWan_APP.h"
#include <Wire.h>  
#include "HT_SSD1306Wire.h"

/********************************* LoRa Config *********************************************/
#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                             10        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define BUFFER_SIZE                                 30 // Payload size

char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

typedef enum {
    LOWPOWER,
    STATE_RX
} States_t;

States_t state;
int16_t rxNumber = 0, Rssi = 0, rxSize = 0;
bool receiveflag = false;

SSD1306Wire factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    rxNumber++;
    Rssi = rssi;
    rxSize = size;
    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0';
    Radio.Sleep();
    
    Serial.printf("\r\nPaquete recibido \"%s\" con RSSI %d, tamaño %d\r\n", rxpacket, Rssi, rxSize);
    receiveflag = true;
    
    // Vuelve a modo recepción después de procesar
    state = STATE_RX;
}

void lora_init(void) {
    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
    RadioEvents.RxDone = OnRxDone;
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    
    // Configuración solo para recepción
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      0, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
    
    state = STATE_RX; // Inicia en modo recepción
}

void setup() {
    Serial.begin(115200);
    delay(100);
    
    factory_display.init();
    factory_display.clear();
    factory_display.display();

    lora_init();
    
    factory_display.drawString(0, 10, "V2 RECEPTOR");
    factory_display.drawString(0, 30, "Solo recibe");
    factory_display.display();
    delay(2000);
    factory_display.clear();
    
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    
    // Inicia la recepción
    Serial.println("Iniciando modo receptor...");
    Radio.Rx(0);
}

void loop() {
    if (receiveflag) {
        receiveflag = false;
        
        String receivedData = String(rxpacket);
        
        factory_display.clear();
        factory_display.drawString(0, 0, "V2 RECEPTOR");
        factory_display.drawString(0, 15, "Mensaje recibido:");
        factory_display.drawString(0, 30, receivedData.c_str());
        factory_display.drawString(0, 45, ("RSSI: " + String(Rssi) + " dBm").c_str());
        factory_display.display();
        
        delay(2000);
        
        // Vuelve a modo recepción
        factory_display.clear();
        factory_display.drawString(0, 20, "Esperando datos...");
        factory_display.drawString(0, 40, ("#Rx: " + String(rxNumber)).c_str());
        factory_display.display();
        
        Radio.Rx(0);
        state = LOWPOWER;
    }

    switch (state) {
        case STATE_RX:
            Serial.println("Modo recepción activo...");
            Radio.Rx(0);
            state = LOWPOWER;
            break;
            
        case LOWPOWER:
            Radio.IrqProcess();
            break;
            
        default:
            break;
    }
}