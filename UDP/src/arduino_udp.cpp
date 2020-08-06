/*
  arduino_udp.cpp - Library for managing a udp connection
*/

#include "arduino_udp.h"

// ==============================================================
// PUBLIC

// Constructor
Arduino_UDP::Arduino_UDP(byte *mac, byte *ip, unsigned int cmd_port, unsigned int evnt_port) {

	_cmd_port = cmd_port;
	_evnt_port = evnt_port;

	_udp = new EthernetUDP();
	IPAddress _ip = IPAddress(ip[0], ip[1], ip[2], ip[3]);
	// Start Ethernet and UDP:
	Ethernet.begin(mac, _ip);
  	_udp->begin(_cmd_port);
}

// Read packet
bool Arduino_UDP::doRead(char* packet_buffer) {
	// Read the packet into packetBufffer
	int packet_size = queryPacket();
	if (packet_size > 0) {
		_udp->read(packet_buffer, UDP_TX_PACKET_MAX_SIZE);
		// Terminate buffer
		packet_buffer[packet_size] = '\0';
		return true;
	}
	return false;
}

// Write response
bool Arduino_UDP::sendResponse(char* reply_buffer) {
	// Send a reply to the IP address and port that sent us the packet we received
	_udp->beginPacket(_udp->remoteIP(), _udp->remotePort());
	_udp->write(reply_buffer);
	_udp->endPacket();
	return true;
}

// ==============================================================
// PRIVATE

// Data available?
int Arduino_UDP::queryPacket() {
	int packetSize = _udp->parsePacket();
  //Serial.println(packetSize);
	  if (packetSize)
	    return packetSize;
	   else
     return 0;
}
