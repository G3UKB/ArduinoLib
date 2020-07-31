/*
  g3ukb_udp.h - Library for managing a udp connection
*/

#ifndef g3ukb_udp_h
#define g3ukb_udp_h

#include "Arduino.h"
#include <Ethernet.h>                // Base Ethernet lib
#include <EthernetUdp.h>             // UDP library from: bjoern@cs.stanford.edu 12/30/2008

class G3UKB_UDP
{
  public:
    G3UKB_UDP(byte *mac, byte *ip, unsigned int cmd_port, unsigned int evnt_port);

	// Method prototypes
	bool doRead(char* packet_buffer);
	bool sendResponse(char* reply_buffer);

  private:
  	// Net info
	IPAddress *_ip;
	int _cmd_port;
	int _evnt_port;
	EthernetUDP *_udp;

	// Method prototypes
	int queryPacket();
};

#endif