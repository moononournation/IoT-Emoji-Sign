/*
  This ESP8266 Web Socket Emoji Sign based on:
  https://github.com/tzapu/WiFiManager
  https://github.com/Links2004/arduinoWebSockets
  https://github.com/adafruit/Adafruit_NeoPixel
*/

#define MDNS_NAME "esp-emoji"
#define NEOPIXEL_PIN 4
#define WIDTH 8
#define HEIGHT 8
 
// Library that help you connect your WiFi AP on the air
#include <WiFiManager.h>

// server libraries
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

// NeoPixel library
#include <Adafruit_NeoPixel.h>

// setup server ports
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// setup NePixel
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(WIDTH * HEIGHT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ400);

// handle WebSocket message
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text[%u].\n", num, length);
      //Serial.println(payload);

      uint8_t i = 0;
      for (uint8_t y = 0; y < HEIGHT; y++) {
      for (uint8_t x = 0; x < WIDTH; x++) {
        uint8_t r = payload[i++];
        uint8_t g = payload[i++];
        uint8_t b = payload[i++];
        uint16_t p = (y % 2) ? (((y + 1) * 8) - x - 1) : ((y * 8) + x);
        pixels.setPixelColor(p, pixels.Color(r, g, b));
        //Serial.printf("(%u, %u) [%u] color: %u, %u, %u\n", x, y, p, r, g, b);
      }
      }
      pixels.show(); // This sends the updated pixel color to the hardware.

      break;
  }
}

void setup() {
  Serial.begin(115200);

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.fill(pixels.Color(0, 0, 0)); // Fill black.
  pixels.show(); // This sends the updated pixel color to the hardware.

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
//  wifiManager.resetSettings(); //reset saved settings
  wifiManager.autoConnect(MDNS_NAME);

  // start MDNS
  if (MDNS.begin(MDNS_NAME)) {
    Serial.println("MDNS responder started.");
  }

  // start web server
  // handle index
  server.on("/", []() {
    // send index.html
    server.send(200, "text/html", "<!DOCTYPE html><html><head><meta name='viewport' content='user-scalable=no,initial-scale=2.0' /><style>body{background:#000;color:#fff;text-align:center}div{margin:10px}</style></head><body><div><h3>Pick An Emoji</h3> <img id='emojis' onclick='clickEmoji(event);' src='data:image/png;base64,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'></div><div><p id='out'></p> <canvas id='emoji' width='8' height='8'></canvas></div> <script>function nw(){return new WebSocket('ws://'+location.hostname+':81/',['arduino']);} var ws=nw();function cc(c){return String.fromCharCode(c/64);} function clickEmoji(e){var xo=3;var yo=3;var xs=13;var ys=12;var x=e.offsetX;var y=e.offsetY;var c=Math.round((x-xo-4)/xs);var r=Math.round((y-yo-4)/ys);document.getElementById('out').innerText='Selected ['+c+', '+r+']';var img=document.getElementById('emojis');var ce=document.getElementById('emoji');var ctx=ce.getContext('2d');ctx.drawImage(img,-(xo+(xs*c)),-(yo+(ys*r)),131,122);var d=ctx.getImageData(0,0,8,8).data;var t='';for(var i=0;i<64;i++){t+=cc(d[(i*4)]);t+=cc(d[(i*4)+1]);t+=cc(d[(i*4)+2]);} ws.send(t,{binary:true});}</script> </body></html>");
    Serial.println("index.html sent.");
  });
  server.begin();

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}

unsigned long last_10sec = 0;
unsigned int counter = 0;

void loop() {
  unsigned long t = millis();
  webSocket.loop();
  server.handleClient();

  if ((t - last_10sec) > 10 * 1000) {
    counter++;
    bool ping = (counter % 2);
    int i = webSocket.connectedClients(ping);
    Serial.printf("%d Connected websocket clients ping: %d\n", i, ping);
    last_10sec = millis();
  }
}
