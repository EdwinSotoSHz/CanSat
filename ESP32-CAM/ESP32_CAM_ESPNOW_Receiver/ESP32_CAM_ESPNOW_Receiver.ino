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

String metadata = "";

void onReceive(const esp_now_recv_info*,
               const uint8_t* data,
               int) {

  image_packet_t pkt;
  memcpy(&pkt, data, sizeof(pkt));

  // ---- metadata ----
  if (pkt.index == 0) {
    metadata = "";
    for (int i = 0; i < pkt.size; i++)
      metadata += (char)pkt.data[i];

    if (imageBuffer) free(imageBuffer);

    totalPackets = pkt.total - 1;
    imageSize = totalPackets * PACKET_SIZE;
    imageBuffer = (uint8_t*)malloc(imageSize);

    receivedPackets = 0;
    imageReady = false;
    return;
  }

  memcpy(imageBuffer + (pkt.index - 1) * PACKET_SIZE,
         pkt.data,
         pkt.size);

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
  while (WiFi.status() != WL_CONNECTED) delay(300);

  Serial.println(WiFi.localIP());

  esp_now_init();
  esp_now_register_recv_cb(onReceive);

  server.on("/", []() {
    if (!imageReady) {
      server.send(200, "text/plain", "No hay imagen aun");
      return;
    }

    String html =
      "<h2>Imagen recibida</h2>"
      "<pre>" + metadata + "</pre>"
      "<img src='/img'>";

    server.send(200, "text/html", html);
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
