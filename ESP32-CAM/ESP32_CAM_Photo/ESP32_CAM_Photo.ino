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

  // ===== CONFIGURACIÓN CORREGIDA =====
  // Resolución XGA para campo de visión más amplio (menos "zoom")
  config.frame_size = FRAMESIZE_SVGA;      // 1024x768 - Más amplio
  
  // Calidad JPEG buena
  config.jpeg_quality = 8;                // Mejor calidad (menor número = mejor)
  
  // Dos framebuffers para estabilidad
  config.fb_count = 2;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Error al iniciar cámara");
    return;
  }

  // ===== AJUSTES ESPECÍFICOS PARA BRILLO Y CONTRASTE =====
  sensor_t *s = esp_camera_sensor_get();
  if (s != NULL) {
    Serial.println("Configurando sensor...");
    
    // 1. PRIMERO APAGAR CONTROLES AUTOMÁTICOS TEMPORALMENTE
    s->set_gain_ctrl(s, 0);        // Ganancia manual
    s->set_exposure_ctrl(s, 0);    // Exposición manual
    s->set_awb_gain(s, 0);         // Balance blancos manual
    
    // 2. AJUSTES PARA MEJORAR BRILLO Y REDUCIR OPACIDAD
    s->set_brightness(s, 1);       // Aumentar brillo (rango: -2 a 2)
    s->set_contrast(s, 1);         // Aumentar contraste para más definición
    s->set_saturation(s, 1);       // Aumentar saturación para colores más vivos
    s->set_sharpness(s, 0);        // Nitidez normal (0)
    
    // 3. CONFIGURAR EXPOSICIÓN MANUAL PARA MÁS LUZ
    s->set_aec_value(s, 400);      // Valor exposición más alto (rango: 0-1200)
    // Prueba estos valores: 300 (interior), 400 (normal), 500-600 (más brillo)
    
    // 4. CONFIGURAR GANANCIA PARA MÁS SENSIBILIDAD A LA LUZ
    s->set_gainceiling(s, GAINCEILING_8X);  // Ganancia máxima 8X
    s->set_agc_gain(s, 5);         // Ganancia AGC (0-30)
    
    // 5. VOLVER A ENCENDER AUTOMÁTICOS CON CONFIGURACIÓN MÁS LUMINOSA
    delay(100);
    s->set_gain_ctrl(s, 1);        // Ganancia automática ON
    s->set_exposure_ctrl(s, 1);    // Exposición automática ON
    s->set_awb_gain(s, 1);         // Balance blancos automático ON
    
    // 6. CONFIGURAR MODO ESPECÍFICO PARA INTERIORES/EXTERIORES
    s->set_wb_mode(s, 1);          // 1=Soleado (más cálido), 2=Nublado (más frío)
    
    // 7. ESPECIAL: Configuración especial para menos opacidad
    s->set_special_effect(s, 0);   // 0=Sin efecto, 2=Negativo, 3=BN, etc.
    
    // 8. AJUSTAR LÍMITE DE EXPOSICIÓN
    s->set_aec2(s, 0);             // 0=AEC desactivado, 1=AEC activado
    s->set_ae_level(s, 0);         // Nivel exposición (-2 a 2)
    
    Serial.println("Sensor configurado");
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
  Serial.println("Configuración: XGA (1024x768), brillo aumentado");

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
        Serial.print("Tamaño: ");
        Serial.print(last_fb->len);
        Serial.println(" bytes");
        Serial.print("Resolución: 1024x768 (XGA) - Campo más amplio");
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