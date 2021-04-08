#include <string>
#include <vector>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>


//Pcapplusplus
#include <Packet.h>
#include <UdpLayer.h>
#include <IPv4Layer.h>
#include <PayloadLayer.h>
#include "PcapLiveDeviceList.h"
#include "PlatformSpecificUtils.h"

// Utils
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

// packet for extractor class
struct MW2Packet {
	string srcIP;
	string destIP;
	uint16_t srcport;
	uint16_t destport;
	size_t payloadlen;
	unsigned char* payload;
};

/**
 * A struct for collecting packet statistics
 */
struct PacketStats
{
	int ethPacketCount;
	int ipv4PacketCount;
	int ipv6PacketCount;
	int tcpPacketCount;
	int udpPacketCount;
	int dnsPacketCount;
	int httpPacketCount;
	int sslPacketCount;


	/**
	 * Clear all stats
	 */
	void clear() { ethPacketCount = 0; ipv4PacketCount = 0; ipv6PacketCount = 0; tcpPacketCount = 0; udpPacketCount = 0; dnsPacketCount = 0; httpPacketCount = 0; sslPacketCount = 0; }

	/**
	 * C'tor
	 */
	PacketStats() { clear(); }

	/**
	 * Collect stats from a packet
	 */
	void consumePacket(pcpp::Packet& packet)
	{
		if (packet.isPacketOfType(pcpp::Ethernet))
			ethPacketCount++;
		if (packet.isPacketOfType(pcpp::IPv4))
			ipv4PacketCount++;
		if (packet.isPacketOfType(pcpp::IPv6))
			ipv6PacketCount++;
		if (packet.isPacketOfType(pcpp::TCP))
			tcpPacketCount++;
		if (packet.isPacketOfType(pcpp::UDP))
			udpPacketCount++;
		if (packet.isPacketOfType(pcpp::HTTP))
			httpPacketCount++;
		if (packet.isPacketOfType(pcpp::SSL))
			sslPacketCount++;
	}

	/**
	 * Print stats to console
	 */
	void printToConsole()
	{
		printf("Ethernet packet count: %d\n", ethPacketCount);
		printf("IPv4 packet count:     %d\n", ipv4PacketCount);
		printf("IPv6 packet count:     %d\n", ipv6PacketCount);
		printf("TCP packet count:      %d\n", tcpPacketCount);
		printf("UDP packet count:      %d\n", udpPacketCount);
		printf("DNS packet count:      %d\n", dnsPacketCount);
		printf("HTTP packet count:     %d\n", httpPacketCount);
		printf("SSL packet count:      %d\n", sslPacketCount);
	}
};

// Sniffer
static bool onPacketArrivesBlockingMode(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* cookie);
uint8_t get_payload_type(unsigned char* payload);
std::string get_localIP();