#ifndef PACKET_H
#define PACKET_H

#include <vector>
#include <iostream>
using namespace std;

class Packet
{

struct Header
{
    uint16_t seq;
    uint16_t ack;
    uint16_t win;
    uint16_t flag;
    void setFlag(bool ac, bool sy, bool fi) { flag |= (ac << 2) | (sy << 1) | fi ;}
} m_header; 

std::vector<uint8_t> m_data;


public:	
	Packet();
	Packet(uint16_t seq, uint16_t ack, uint16_t win, bool ac, bool sy, bool fi);
	~Packet();
	uint8_t *encode(); // easy set flag
	uint16_t getSeq() {return m_header.seq;}
	uint16_t getAck() {return m_header.ack;}
	uint16_t getWin() {return m_header.win;}
	uint16_t getFlag() {return m_header.flag;}

	uint8_t *m_packet;
	void debug(uint8_t *buffer);



};

#endif