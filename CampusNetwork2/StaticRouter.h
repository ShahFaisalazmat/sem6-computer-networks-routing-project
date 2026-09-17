#ifndef STATICROUTER_H
#define STATICROUTER_H

#include <omnetpp.h>
#include <string>
#include <vector>
#include <map>
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"

using namespace omnetpp;

/**
 * RouteEntry stores one static route.
 */
struct RouteEntry {
    uint32_t destNetwork;
    uint32_t subnetMask;
    int      outPort;
};

/**
 * StaticRouter: custom IPv4 static router.
 * Handles raw Ethernet frames, responds to ARP requests,
 * performs longest-prefix-match, and forwards IP packets.
 */
class StaticRouter : public cSimpleModule
{
  private:
    std::vector<RouteEntry> routeTable;
    std::vector<uint32_t>   interfaceIPs;
    std::map<uint32_t, std::pair<inet::MacAddress, int>> macTable; // learned MACs
    inet::MacAddress myMac;

    simsignal_t packetForwardedSignal;
    simsignal_t packetDroppedSignal;

    long forwardedCount;
    long droppedCount;

    uint32_t parseIPv4(const std::string& ipStr);
    uint32_t prefixToMask(int prefixLen);
    int longestPrefixMatch(uint32_t destIP);
    void loadRouteTable();
    void loadInterfaceAddresses();
    void handleArpRequest(inet::Packet *packet, int inPort);
    uint32_t extractDestIP(inet::Packet *packet);

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif // STATICROUTER_H
