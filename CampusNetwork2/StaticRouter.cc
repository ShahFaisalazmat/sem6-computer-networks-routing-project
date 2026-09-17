#include "StaticRouter.h"
#include <sstream>
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"

Define_Module(StaticRouter);

uint32_t StaticRouter::parseIPv4(const std::string& ipStr)
{
    uint32_t result = 0;
    std::istringstream ss(ipStr);
    std::string octet;
    int shift = 24;
    while (std::getline(ss, octet, '.') && shift >= 0) {
        result |= (std::stoul(octet) & 0xFF) << shift;
        shift -= 8;
    }
    return result;
}

uint32_t StaticRouter::prefixToMask(int prefixLen)
{
    if (prefixLen == 0) return 0;
    return (~0u) << (32 - prefixLen);
}

int StaticRouter::longestPrefixMatch(uint32_t destIP)
{
    int bestPort = -1;
    uint32_t bestMask = 0;
    for (const auto& entry : routeTable) {
        if ((destIP & entry.subnetMask) == entry.destNetwork) {
            if (entry.subnetMask >= bestMask) {
                bestMask = entry.subnetMask;
                bestPort = entry.outPort;
            }
        }
    }
    return bestPort;
}

void StaticRouter::loadRouteTable()
{
    std::string tableStr = par("routeTable").stringValue();
    if (tableStr.empty()) return;

    std::istringstream tableStream(tableStr);
    std::string entry;
    while (std::getline(tableStream, entry, ';')) {
        size_t start = entry.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = entry.find_last_not_of(" \t\r\n");
        entry = entry.substr(start, end - start + 1);

        std::istringstream ss(entry);
        std::string cidr, via, portStr;
        ss >> cidr >> via >> portStr;
        if (via != "via" || cidr.empty() || portStr.empty()) continue;

        size_t slashPos = cidr.find('/');
        if (slashPos == std::string::npos) continue;

        std::string netAddr = cidr.substr(0, slashPos);
        int prefixLen = std::stoi(cidr.substr(slashPos + 1));

        RouteEntry re;
        re.destNetwork = parseIPv4(netAddr) & prefixToMask(prefixLen);
        re.subnetMask  = prefixToMask(prefixLen);
        re.outPort     = std::stoi(portStr);
        routeTable.push_back(re);

        EV_INFO << "[STATIC] Loaded route: " << netAddr << "/" << prefixLen
                << " -> port " << re.outPort << endl;
    }
}

void StaticRouter::loadInterfaceAddresses()
{
    std::string addrs = par("interfaceAddresses").stringValue();
    if (addrs.empty()) return;

    std::istringstream ss(addrs);
    std::string ip;
    while (std::getline(ss, ip, ';')) {
        size_t start = ip.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = ip.find_last_not_of(" \t\r\n");
        ip = ip.substr(start, end - start + 1);
        interfaceIPs.push_back(parseIPv4(ip));
    }
}

void StaticRouter::handleArpRequest(inet::Packet *packet, int inPort)
{
    try {
        // Pop Ethernet header to reach ARP packet
        packet->popAtFront<inet::EthernetMacHeader>();
        auto arp = packet->peekAtFront<inet::ArpPacket>();

        if (arp->getOpcode() == inet::ARP_REQUEST) {
            uint32_t targetIP = arp->getDestIpAddress().getInt();

            // Check if target IP is one of our interface addresses
            bool isOurs = false;
            for (auto ip : interfaceIPs) {
                if (ip == targetIP) {
                    isOurs = true;
                    break;
                }
            }

            if (isOurs) {
                // Build ARP reply
                auto reply = new inet::Packet("arpReply");
                auto replyArp = inet::makeShared<inet::ArpPacket>();
                replyArp->setOpcode(inet::ARP_REPLY);
                replyArp->setSrcMacAddress(myMac);
                replyArp->setSrcIpAddress(arp->getDestIpAddress());
                replyArp->setDestMacAddress(arp->getSrcMacAddress());
                replyArp->setDestIpAddress(arp->getSrcIpAddress());
                reply->insertAtBack(replyArp);

                auto ethHeader = inet::makeShared<inet::EthernetMacHeader>();
                ethHeader->setDest(arp->getSrcMacAddress());
                ethHeader->setSrc(myMac);
                ethHeader->setTypeOrLength(0x0806);
                reply->insertAtFront(ethHeader);

                EV_INFO << "[STATIC] Sending ARP reply for "
                        << arp->getDestIpAddress()
                        << " to " << arp->getSrcMacAddress()
                        << " on port " << inPort << endl;
                send(reply, "ethg$o", inPort);
            } else {
                EV_INFO << "[STATIC] ARP request for unknown IP "
                        << arp->getDestIpAddress() << ", dropping" << endl;
            }
        }
    } catch (cRuntimeError& e) {
        EV_WARN << "[STATIC] Failed to process ARP packet: " << e.what() << endl;
    }
    delete packet;
}

uint32_t StaticRouter::extractDestIP(inet::Packet *packet)
{
    // Pop Ethernet header if present
    if (packet->getTotalLength() >= inet::B(14)) {
        try {
            packet->popAtFront<inet::EthernetMacHeader>();
        } catch (cRuntimeError& e) {
            // Not Ethernet, continue
        }
    }

    try {
        auto ipv4Header = packet->peekAtFront<inet::Ipv4Header>();
        return ipv4Header->getDestAddress().getInt();
    } catch (cRuntimeError& e) {
        return 0;
    }
}

void StaticRouter::initialize()
{
    packetForwardedSignal = registerSignal("packetForwarded");
    packetDroppedSignal   = registerSignal("packetDropped");
    forwardedCount = 0;
    droppedCount   = 0;

    std::string macStr = par("macAddress").stringValue();
    myMac = inet::MacAddress(macStr.c_str());

    loadInterfaceAddresses();
    loadRouteTable();

    EV_INFO << "[STATIC] Router initialized with " << routeTable.size()
            << " routes, MAC=" << myMac
            << " at t=" << simTime() << endl;
}

void StaticRouter::handleMessage(cMessage *msg)
{
    auto packet = dynamic_cast<inet::Packet*>(msg);
    if (!packet) {
        EV_WARN << "[STATIC] Non-Packet message, dropping" << endl;
        emit(packetDroppedSignal, ++droppedCount);
        delete msg;
        return;
    }

    cGate *inGate = msg->getArrivalGate();
    if (inGate && !inGate->isConnected()) {
        EV_WARN << "[STATIC] Disconnected input gate, dropping" << endl;
        emit(packetDroppedSignal, ++droppedCount);
        delete packet;
        return;
    }

    int inPort = inGate ? inGate->getIndex() : -1;

    // Check if this is an ARP frame (type 0x0806)
    bool isArp = false;
    if (packet->getTotalLength() >= inet::B(14)) {
        try {
            auto ethPeek = packet->peekAtFront<inet::EthernetMacHeader>();
            if (ethPeek->getTypeOrLength() == 0x0806)
                isArp = true;
        } catch (...) {}
    }

    if (isArp) {
        handleArpRequest(packet, inPort);
        return;
    }

    uint32_t destIP = extractDestIP(packet);
    if (destIP == 0) {
        EV_WARN << "[STATIC] Cannot extract IPv4 destination, dropping" << endl;
        emit(packetDroppedSignal, ++droppedCount);
        delete packet;
        return;
    }

    int outPort = longestPrefixMatch(destIP);
    if (outPort < 0 || outPort >= (int)gateSize("ethg")) {
        EV_WARN << "[STATIC] No route to " << destIP
                << ", dropping at t=" << simTime() << endl;
        emit(packetDroppedSignal, ++droppedCount);
        delete packet;
        return;
    }

    cGate *outGate = gate("ethg$o", outPort);
    if (!outGate->isConnected()) {
        EV_WARN << "[STATIC] Output port " << outPort
                << " is down, dropping at t=" << simTime() << endl;
        emit(packetDroppedSignal, ++droppedCount);
        delete packet;
        return;
    }

    // Add Ethernet header with broadcast destination
    auto ethHeader = inet::makeShared<inet::EthernetMacHeader>();
    ethHeader->setDest(inet::MacAddress("FF:FF:FF:FF:FF:FF"));
    ethHeader->setSrc(myMac);
    ethHeader->setTypeOrLength(0x0800);
    packet->insertAtFront(ethHeader);

    EV_INFO << "[STATIC] Forward to dest=" << destIP
            << " out port " << outPort << " at t=" << simTime() << endl;
    emit(packetForwardedSignal, ++forwardedCount);
    send(packet, "ethg$o", outPort);
}

void StaticRouter::finish()
{
    EV_INFO << "[STATIC] Forwarded=" << forwardedCount
            << " Dropped=" << droppedCount << endl;
    recordScalar("packetsForwarded", forwardedCount);
    recordScalar("packetsDropped",   droppedCount);
}
