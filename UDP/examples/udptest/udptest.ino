//#include <arduino_udp.h>
#include "src/arduino_udp.h"

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};
byte ip[] = {
  192, 168, 1, 178
};
unsigned int localPort = 8888;
unsigned int eventPort = 8889;

// Buffers for receiving and sending data
char  packet_buffer[UDP_TX_PACKET_MAX_SIZE]; // Buffer to hold incoming packet,
char  reply_buffer[128];                     // The response data
G3UKB_UDP *_udp;

// UDP udp(mac, ip, localPort, eventPort);


void setup() {
  _udp = new G3UKB_UDP(mac, ip, localPort, eventPort);
}

void loop() {
  _udp->doRead(packet_buffer);
  strcpy(reply_buffer, packet_buffer);
  _udp->sendResponse(reply_buffer);
  exit(0);
}
