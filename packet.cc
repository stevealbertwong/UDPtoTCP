/*
TODO: now only encode header not yet encode header + data
*/
#include <bitset>
#include "packet.h"
#include <string.h>     /* strcat, memcpy */

// socket stream -> packet struct
Packet::Packet(char *byte_array, uint32_t recv_size){	
	m_encoded_size = recv_size; //  need recv_size from recv_from(), ptr no size
	m_packet = new uint8_t(m_encoded_size);
	memset(m_packet, '\0', m_encoded_size);	

	memcpy(m_packet, byte_array, m_encoded_size);
		
	/* decode() byte_array into m_header n m_payload */	
	memcpy(&m_header, byte_array, sizeof(m_header)); 	
	cout << "m_header.flag : " << m_header.flag << endl;	
	
	if (recv_size > sizeof(m_header)){ // if data packet
						
		m_payload = std::vector<uint8_t>(byte_array + sizeof(m_header), byte_array + recv_size);
		// memcpy(&m_payload[0], byte_array + sizeof(m_header), recv_size - sizeof(m_header)); // bug			
		cout << "m_payload: " << &m_payload[0] << endl;	
		// cout << "byte array payload : " << byte_array + sizeof(m_header) << endl;
	}

	// m_payload.clear();			
	// memcpy(&m_payload[0], byte_array + sizeof(m_header), recv_size - sizeof(m_header)); // bug			
	

	// std::vector<uint8_t>::iterator it;	
	// for(it = m_payload.begin(); it < m_payload.end(); it++){
	// 	cout << "m_payload : " << *it<< endl;}	

	// debug();
}

Packet::Packet(uint16_t seq, uint16_t ack, uint16_t win, bool syn_flag, bool ack_flag, bool fin_flag){	
	m_header.seq = seq;	// seq#
	m_header.ack = ack;	// ack#
	m_header.win = win;	// window size
	setFlag(syn_flag,ack_flag,fin_flag); // indicate packet type
	encode();
}

Packet::Packet(uint16_t seq, uint16_t ack, uint16_t win, std::vector<uint8_t> m_payload){
	m_header.seq = seq;	// seq#
	m_header.ack = ack;	// ack#
	m_header.win = win;	// window size
	setFlag(0,0,0); // data packet
	m_payload = m_payload;
	encode();
}

// memcpy() struct n vector -> byte array 
uint8_t *Packet::encode(){	
	m_encoded_size = sizeof(m_header) + m_payload.size(); // header is fixed 8 bytes
	cout << "m_encoded_size : " << m_encoded_size << endl; 
	
	m_packet = new uint8_t(m_encoded_size);
	memset(m_packet, '\0', m_encoded_size);	// null terminate byte array 

	memcpy(m_packet, &m_header, sizeof(m_header)); // struct to byte array
	// std::copy(m_payload.begin(), m_payload.end(), m_packet + sizeof(m_header));
	memcpy(m_packet + sizeof(m_header), &m_payload[0], m_payload.size());
	cout << "m_payload.size() : " << m_payload.size() << endl;
	return m_packet; 	
}

void Packet::debug(){
		
	
	// cout << *(long*)m_packet << endl; // 
	
	for(int i=0;i<m_encoded_size-1;i++)
		printf("%02X ",m_packet[i]);
	printf("\n");
	
}

// Packet::Packet(const Packet& rhs){ // copy constructor, deep copy
// 	m_header = rhs.m_header;
// 	m_payload = rhs.m_payload;
// 	m_packet = new uint8_t[rhs.m_encoded_size];
// 	memcpy(&m_packet, rhs.m_packet, rhs.m_encoded_size); 
// }

// Packet::Packet(Packet&& rhs){ // move constructor, shallow copy
// 	// like std::move() -> convert lvalue to rvalue
// 	m_header = rhs.m_header;
// 	m_payload = rhs.m_payload;
// 	m_encoded_size = rhs.m_encoded_size;
// 	m_packet = rhs.m_packet;	
// 	rhs.m_packet = nullptr; // DONT free "moved" pointer !!
// }

// Packet& Packet::operator=(const Packet& rhs){ // assignment, copy to existing packet
// 	this->m_header = rhs.m_header;
// 	this->m_payload = rhs.m_payload;
// 	memcpy(this->m_packet, rhs.m_packet, rhs.m_encoded_size); 	
// 	return *this; // lhs
// }

/*
CANNOT FREE HERE 
PACKET HAS BEEN PASSED AS COPY IN OTHER FUNCTION SO ALREADY FREED
SOLUTION: RULE OF 3 -> WHEN PACKET PASSED IN FUNCTION AS ARG -> DEEP COPY
*/
Packet::~Packet(){


	// if(m_packet){		
	// 	delete []m_packet;
	// }
}