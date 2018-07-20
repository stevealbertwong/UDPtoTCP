# UDPtoTCP


FSM graph
...

How it works - command, screencap
...

TCP vs UDP
1. listen(), accept(), recv() etc. => connection/data structure/stateful
2. implements the "connection" data structure
...



Challenges

1. nonblocking recvfrom() - select(), nonblocking socket

	fcntl() or recvfrom(MSG_DONTWAIT) vs select() non blocking

	why select() 

	=> polling wastes resources, interrupts elegant, efficient, saves CPU resources

	=> select() / blocking uses sleep and interrupt mechanism, however pure blocking completely puts CPU to sleep, select allows timer


2. dividing server files into packets

3. engineer TCP packet from UCP packets - header + data + endianess

 bit manipulation: turning uint8_t byte array into struct of uint16_t: flag, seq#, ack#, window
 rule of 3
 raw dup packet actual construction as reference

4. handshake implementation in accept(), connect() api




5. Reno FSM implementation in send(), recv() api

6. TCP and Packet class design
	TCP: class => subclass
	Packet: a class instead of struct => varities of packets construction needed


