/*
1. class construction, rule of 3
2. data type of header, payload, packet
3. encode() - append payload to header, then typecast to byte array
*/
#ifndef PACKET_H
#define PACKET_H

#include <vector>
#include <iostream>
using namespace std;

class Packet{

private:
	struct Header{
		uint16_t seq;
		uint16_t ack;
		uint16_t win;
		uint16_t flag; // each flag occupy 1 bit -> 16 bits 16 flags
	} m_header; // 8 bytes
	void setFlag(bool syn, bool ack, bool fin) { m_header.flag |= (syn << 2) | (ack << 1) | fin;}
	
	std::vector<uint8_t> m_payload; 			
	uint8_t *m_packet; // recv(), send() -> uint8_t byte array as arg
	uint32_t m_encoded_size; // size of m_header + size of m_payload

public:	
	Packet(){}; // default uninitialize constructor -> ELF .BSS segment
	Packet(uint16_t seq, uint16_t ack, uint16_t win, bool syn_flag, bool ack_flag, bool fin_flag); // construct ACK, SYN, FIN packet 
	Packet(uint16_t seq, uint16_t ack, uint16_t win, std::vector<uint8_t> m_payload); // construct data packet
	Packet(char *byte_array, uint32_t recv_size); // byte array initialization	
	// Packet(const Packet& rhs); // copy constructor, deep copy
	// Packet(Packet&& rhs); // move constructor, shallow copy
	// Packet& operator=(const Packet& rhs); // assignment, copy to existing packet
	~Packet();
			
	uint16_t getSeq() {return m_header.seq;}
	uint16_t getAck() {return m_header.ack;}
	uint16_t getWin() {return m_header.win;}
	uint16_t getFlag() {return m_header.flag;}
	uint32_t getEncodedSize() {return m_encoded_size;}
	uint8_t *getPacket() {return m_packet;}

	uint8_t *encode(); // append m_payload to m_header, then typecast char* to uint8* 	
	void debug();
};
#endif

// Reference:
/* 3 ways to convert byte array into vector m_payload
1. m_payload(byte_array + sizeof(Header), byte_array + recv_size);	
2. std::copy (byte_array + sizeof(Header), byte_array + recv_size, m_payload.begin());
3. for(int i = sizeof(Header); i < recv_size; i++){
	m_payload.push_back(byte_array[i]);}
*/


// Reference: full header + checksum error function

// struct ipheader {
//  unsigned char      iph_ihl:5, iph_ver:4;
//  unsigned char      iph_tos;
//  unsigned short int iph_len;
//  unsigned short int iph_ident;
//  unsigned char      iph_flag;
//  unsigned short int iph_offset;
//  unsigned char      iph_ttl;
//  unsigned char      iph_protocol;
//  unsigned short int iph_chksum;
//  unsigned int       iph_sourceip;
//  unsigned int       iph_destip;
// };
// struct udpheader {
//  unsigned short int udph_srcport;
//  unsigned short int udph_destport;
//  unsigned short int udph_len;
//  unsigned short int udph_chksum;
// };
// char buffer[PCKT_LEN];
// struct ipheader *ip = (struct ipheader *) buffer;
// struct udpheader *udp = (struct udpheader *) (buffer + 
// 	sizeof(struct ipheader));
// memset(buffer, 0, PCKT_LEN);
// // Writing into buffer => No data/payload just datagram
// ip->iph_ihl = 5;
// ip->iph_ver = 4;
// ip->iph_tos = 16; // Low delay
// ip->iph_len = sizeof(struct ipheader) + sizeof(struct udpheader);
// ip->iph_ident = htons(54321);
// ip->iph_ttl = 64; // hops
// ip->iph_protocol = 17; // UDP
// ip->iph_sourceip = inet_addr(argv[1]);
// ip->iph_destip = inet_addr(argv[3]);
// udp->udph_srcport = htons(atoi(argv[2]));
// udp->udph_destport = htons(atoi(argv[4]));
// udp->udph_len = htons(sizeof(struct udpheader));
// ip->iph_chksum = checksum((unsigned short *)buffer, 
// 	sizeof(struct ipheader) + sizeof(struct udpheader));
// sd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
// sendto(sd, buffer, ip->iph_len, 0, (struct sockaddr *)&sin, sizeof(sin));

// unsigned short checksum(unsigned short *buf, int nwords)
// {       
//         unsigned long sum;
//         for(sum=0; nwords>0; nwords--)
//                 sum += *buf++;
//         sum = (sum >> 16) + (sum &0xffff);
//         sum += (sum >> 16);
//         return (unsigned short)(~sum);
// }