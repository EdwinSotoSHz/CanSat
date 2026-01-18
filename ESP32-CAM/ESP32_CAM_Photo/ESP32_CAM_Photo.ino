#include "esp_camera.h"
#include <WiFi.h>

// ===== WiFi =====
const char* ssid = "AsusE";
const char* password = "23011edpi";

// ===== Pines cámara AI Thinker =====
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

WiFiServer server(80);
camera_fb_t *last_fb = NULL;

void setup() {
  Serial.begin(115200);

  // --- Config cámara ---
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
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

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Error al iniciar cámara");
    return;
  }

  // --- WiFi ---
  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Escribe 1 y presiona Enter para tomar foto");

  server.begin();
}

void loop() {
  // ===== Serial trigger =====
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '1') {
      if (last_fb) {
        esp_camera_fb_return(last_fb);
        last_fb = NULL;
      }

      last_fb = esp_camera_fb_get();
      if (last_fb) {
        Serial.println("Foto capturada");
      } else {
        Serial.println("Error al capturar foto");
      }
    }
  }

  // ===== Servir imagen =====
  WiFiClient client = server.available();
  if (!client || !last_fb) return;

  while (!client.available()) delay(1);
  client.readStringUntil('\r');

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/jpeg");
  client.println("Content-Length: " + String(last_fb->len));
  client.println();
  client.write(last_fb->buf, last_fb->len);
  client.stop();
}
