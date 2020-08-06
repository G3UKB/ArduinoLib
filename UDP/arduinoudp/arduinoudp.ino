#include "arduino_udp.h"

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};
byte ip[] = {
  192, 168, 1, 178
};
unsigned int localPort = 8888;
unsigned int eventPort = 8889;

// Buffers for receiving and sending data
char  packet_buffer[128]; // Buffer to hold incoming packet,
char  reply_buffer[128];                     // The response data
Arduino_UDP *_udp;

void setup() {
  // Start serial monitor
  Serial.begin(115200);

  // Create UDP instance
  _udp = new Arduino_UDP(mac, ip, localPort, eventPort);
}

void loop() {

  // Test UDP
  while (true) {
    // Wait for data
    while (true) {
       if (_udp->doRead(packet_buffer)) {
          //Serial.println("Data");
          break;
       }
       delay(10);
    }
    // Copy request and return to sender
    strcpy(reply_buffer, packet_buffer);
    _udp->sendResponse(reply_buffer);
  }
}
