/*
  arduino_udp.h - Library for managing a udp connection
*/

#ifndef arduino_udp_h
#define arduino_udp_h

#include "Arduino.h"
#include <Ethernet.h>                // Base Ethernet lib
#include <EthernetUdp.h>             // UDP library from: bjoern@cs.stanford.edu 12/30/2008

class Arduino_UDP
{
  public:
    Arduino_UDP(byte *mac, byte *ip, unsigned int cmd_port, unsigned int evnt_port);

	// Method prototypes
	bool doRead(char* packet_buffer);
	bool sendResponse(char* reply_buffer);

  private:
  	// Net info
	int _cmd_port;
	int _evnt_port;
	EthernetUDP *_udp;

	// Method prototypes
	int queryPacket();
};

#endif
