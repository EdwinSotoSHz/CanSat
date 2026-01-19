#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>

#define PACKET_SIZE 200

typedef struct {
  uint16_t index;
  uint16_t total;
  uint16_t size;
  uint8_t data[PACKET_SIZE];
} image_packet_t;

WebServer server(80);

uint8_t *imageBuffer = nullptr;
size_t imageSize = 0;
uint16_t receivedPackets = 0;
uint16_t totalPackets = 0;
bool imageReady = false;

void onReceive(const esp_now_recv_info *info,
               const uint8_t *data,
               int len) {

  image_packet_t pkt;
  memcpy(&pkt, data, sizeof(pkt));

  if (pkt.index == 0) {
    if (imageBuffer) free(imageBuffer);

    totalPackets = pkt.total;
    imageSize = totalPackets * PACKET_SIZE;
    imageBuffer = (uint8_t*)malloc(imageSize);

    receivedPackets = 0;
    imageReady = false;
  }

  memcpy(imageBuffer + pkt.index * PACKET_SIZE, pkt.data, pkt.size);
  receivedPackets++;

  if (receivedPackets == totalPackets) {
    imageReady = true;
    Serial.println("Imagen completa");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin("AsusE", "23011edpi");
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  esp_now_init();
  esp_now_register_recv_cb(onReceive);

  server.on("/", []() {
    if (!imageReady) {
      server.send(200, "text/plain", "No hay imagen aun");
      return;
    }

    server.send(200, "text/html",
      "<h2>Imagen recibida</h2><img src='/img'>");
  });

  server.on("/img", []() {
    WiFiClient client = server.client();
    server.setContentLength(imageSize);
    server.send(200, "image/jpeg", "");
    client.write(imageBuffer, imageSize);
  });

  server.begin();
}

void loop() {
  server.handleClient();
}