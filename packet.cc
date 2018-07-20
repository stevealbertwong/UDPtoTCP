#include "packet.h"
#include <string.h>     /* strcat, memcpy */


Packet::Packet(){
	
}
Packet::Packet(uint16_t seq, uint16_t ack, uint16_t win, bool ac, bool sy, bool fi){
	m_header.seq = seq;
	m_header.ack = ack;
	m_header.win = win;
	m_header.setFlag(ac,sy,fi);

}


uint8_t *Packet::encode(){
	uint32_t total_size = sizeof(m_header);
	m_packet = new uint8_t(total_size);
	memcpy(m_packet, &m_header, sizeof(m_header));
	return m_packet;
}

void Packet::debug(uint8_t *buffer){
	cout << (uint16_t)(((struct Header*) buffer)->win) << endl;
	// cout << m_header.seq << endl;
}