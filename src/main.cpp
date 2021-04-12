#include <sniffer.h>
#include <extractor.h>
#include <EndianPortable.h>
#include <iphlpapi.h>

using namespace std;
static string interfaceIPAddr;
Extractor extractor;

int __cdecl main(void)
{
    if (extractor.initExtractor("33333") == INVALID_SOCKET)
        return 1;

    // IPv4 address of the interface we want to sniff
    interfaceIPAddr = get_localIP();

    // find the interface by IP address
    pcpp::PcapLiveDevice* dev = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIp(interfaceIPAddr.c_str());
    if (dev == NULL)
    {
        printf("Cannot find interface with IPv4 address of '%s'\n", interfaceIPAddr.c_str());
        exit(1);
    }


    // open the device before start capturing/sending packets
    if (!dev->open())
    {
        printf("Cannot open device\n");
        exit(1);
    }

    // create the stats object
    PacketStats stats;

    // Capturing packets in blocking mode
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Filter for mw2-packets
    std::string filter = "udp port 28960";
    dev->setFilter(filter);

    printf("\nStarting capture in blocking mode on %s\nFilter: %s\n\n", interfaceIPAddr.c_str(), filter.c_str());

    // start capturing in blocking mode. Give a callback function to call to whenever a packet is captured, the stats object as the cookie and a 10 seconds timeout
    dev->startCaptureBlockingMode(onPacketArrivesBlockingMode, &stats, 0);

    // thread is blocked until capture is finished

    // capture is finished, print results
    printf("Results:\n");
    stats.printToConsole();

    stats.clear();

    // close the device before application ends
    dev->close();

    /* DONE sniffing UDP Packets */
    return 0;
}


/**
 * a callback function for the blocking mode capture which is called each time a packet is captured
 */
static bool onPacketArrivesBlockingMode(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* cookie)
{
    // extract the stats object form the cookie
    PacketStats* stats = (PacketStats*)cookie;

    // parse the raw packet
    pcpp::Packet parsedPacket(packet);

    // collect stats from packet
    stats->consumePacket(parsedPacket);

    // extract payloadlayer from packet
    pcpp::PayloadLayer* payloadLayer = parsedPacket.getLayerOfType<pcpp::PayloadLayer>();

    if (payloadLayer == NULL)
    {
        printf("Something went wrong, couldn't get payload\n");
        exit(1);
    }

    // Filter relevant packets before further processing
    uint8_t payloadtype = get_payload_type(payloadLayer->getPayload());
    if (payloadtype == 0)
        return false; // payload is not of relevance -> dont send to client

        // Just extract it
    MW2Packet pkg;
    pkg.srcIP = parsedPacket.getLayerOfType<pcpp::IPv4Layer>()->getSrcIpAddress().toString();
    pkg.destIP = parsedPacket.getLayerOfType<pcpp::IPv4Layer>()->getDstIpAddress().toString();
    pkg.srcport = be16toh(parsedPacket.getLayerOfType<pcpp::UdpLayer>()->getUdpHeader()->portSrc);
    pkg.destport = be16toh(parsedPacket.getLayerOfType<pcpp::UdpLayer>()->getUdpHeader()->portDst);
    pkg.payloadlen = payloadLayer->getPayloadLen();
    pkg.payload = payloadLayer->getPayload();
    
    switch (payloadtype)
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            extractor.extract_partystate(pkg);
            break;
    }
    // return false means we don't want to stop capturing after this callback
    return false;
}

// return the type of the payload packet
uint8_t get_payload_type(unsigned char* payload) 
{
    char partystate[] = "0partystate";
    char entparty[] = "0endparty";
    char ident[] = "0ident";
    bool bingo = true;

    for (int i = 4, j = 0; i < 15; i++, j++)
    {
        if (payload[i] != ident[j])
        {
            bingo = false;
            break;
        }
    }
    if (bingo) return 1; // 1: the package is of type ident, meaning that a new player joined the party

    bingo = true;
    for (int i = 7, j = 0; i <= 15; i++, j++)
    {
        if (payload[i] != entparty[j])
        {
            bingo = false;
            break;
        }
    }
    if (bingo) return 2; // 2: the package is of type endparty, meaning that a player left the party

    bingo = true;
    for (int i = 4, j = 0; i < 15; i++, j++)
    {
        if (payload[i] != partystate[j])
        {
            bingo = false;
            break;
        }
    }
    if (bingo) return 3; // 3: the package is of type partystate, meaning the host sends player information

    // 0: the packet is not of relevance
    return 0;
}

std::string get_localIP()
{
    /* Variables used by GetIpAddrTable */
    PMIB_IPADDRTABLE pIPAddrTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    IN_ADDR IPAddr;

    /* Variables used to return error message */
    LPVOID lpMsgBuf;

    // Before calling AddIPAddress we use GetIpAddrTable to get
    // an adapter to which we can add the IP.
    pIPAddrTable = (MIB_IPADDRTABLE*)MALLOC(sizeof(MIB_IPADDRTABLE));

    if (pIPAddrTable) {
        // Make an initial call to GetIpAddrTable to get the
        // necessary size into the dwSize variable
        if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) ==
            ERROR_INSUFFICIENT_BUFFER) {
            FREE(pIPAddrTable);
            pIPAddrTable = (MIB_IPADDRTABLE*)MALLOC(dwSize);

        }
        if (pIPAddrTable == NULL) {
            printf("Memory allocation failed for GetIpAddrTable\n");
            exit(1);
        }
    }
    // Make a second call to GetIpAddrTable to get the
    // actual data we want
    if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) != NO_ERROR) {
        printf("GetIpAddrTable failed with error %d\n", dwRetVal);
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       // Default language
            (LPTSTR)&lpMsgBuf, 0, NULL)) {
            std::cout << "Error: " << lpMsgBuf;
            LocalFree(lpMsgBuf);
        }
        exit(1);
    }
    std::string loopback = "127.0.0.1";
    std::string currentIP;

    for (int i = 0; i < (int)pIPAddrTable->dwNumEntries; i++) {
        IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[i].dwAddr;
        currentIP = inet_ntoa(IPAddr);
        if (currentIP._Equal(loopback))
            continue;
        else
            break;
    }

    if (pIPAddrTable) {
        FREE(pIPAddrTable);
        pIPAddrTable = NULL;
    }
    return currentIP;
}