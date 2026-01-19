#include "esp_camera.h"
#include <WiFi.h>
#include <esp_now.h>

// ===== PINOUT AI THINKER =====
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define PACKET_SIZE 200

typedef struct {
  uint16_t index;
  uint16_t total;
  uint16_t size;
  uint8_t data[PACKET_SIZE];
} image_packet_t;

// CAMBIA POR LA MAC DEL RECEPTOR
uint8_t receiverMac[] = {0xC8, 0x2E, 0x18, 0x8E, 0xFA, 0x30};

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  esp_now_init();

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, receiverMac, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_camera_init(&config);

  Serial.println("Escribe 1 para tomar y enviar foto");
}

void loop() {
  if (Serial.read() == '1') {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return;

    uint16_t totalPackets = (fb->len + PACKET_SIZE - 1) / PACKET_SIZE;

    for (uint16_t i = 0; i < totalPackets; i++) {
      image_packet_t pkt;
      pkt.index = i;
      pkt.total = totalPackets;

      uint16_t remaining = fb->len - i * PACKET_SIZE;
      pkt.size = remaining < PACKET_SIZE ? remaining : PACKET_SIZE;

      memcpy(pkt.data, fb->buf + i * PACKET_SIZE, pkt.size);
      esp_now_send(receiverMac, (uint8_t*)&pkt, sizeof(pkt));
      delay(5);
    }

    esp_camera_fb_return(fb);
    Serial.println("Imagen enviada");
  }
}