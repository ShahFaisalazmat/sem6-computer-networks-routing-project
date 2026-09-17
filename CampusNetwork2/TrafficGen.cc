#include "TrafficGen.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

Define_Module(TrafficGen);

void TrafficGen::socketDataArrived(inet::UdpSocket *, inet::Packet *packet)
{
    std::string name = packet->getName();
    size_t p1 = name.find('-'), p2 = name.rfind('-');
    if (p1 != std::string::npos && p2 != std::string::npos && p1 != p2) {
        try {
            emit(endToEndDelaySignal, simTime() - SimTime(std::stod(name.substr(p2+1))));
        } catch (...) {}
    }
    emit(packetReceivedSignal, ++receivedCount);
    delete packet;
}
void TrafficGen::socketErrorArrived(inet::UdpSocket *, inet::Indication *ind) { delete ind; }
void TrafficGen::socketClosed(inet::UdpSocket *) {}

bool TrafficGen::tryConnect()
{
    if (destAddress.empty()) return false;
    try {
        udpSocket.connect(inet::L3AddressResolver().resolve(destAddress.c_str()), destPort);
        socketConnected = true;
        return true;
    } catch (cRuntimeError &e) {
        EV_WARN << "[TG] resolve failed: " << e.what() << endl;
        return false;
    }
}

void TrafficGen::initialize(int stage)
{
    // Call ApplicationBase::initialize ONLY for stage 0 to register with dispatcher
    ApplicationBase::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        trafficMode  = par("trafficMode").stringValue();
        destAddress  = par("destAddress").stringValue();
        destPort     = par("destPort").intValue();
        localPort    = par("localPort").intValue();
        packetSize   = par("packetSize").intValue();
        sendInterval = par("sendInterval").doubleValue();
        burstSize    = par("burstSize").intValue();
        burstGap     = par("burstGap").doubleValue();
        startTime    = par("startTime").doubleValue();
        stopTime     = par("stopTime").doubleValue();

        seqNumber = sentCount = receivedCount = burstRemaining = 0;
        inBurst = socketConnected = false;
        isSender = !destAddress.empty();

        packetSentSignal     = registerSignal("packetSent");
        packetReceivedSignal = registerSignal("packetReceived");
        endToEndDelaySignal  = registerSignal("endToEndDelay");

        sendTimer = new cMessage("sendTimer");
        startMsg  = new cMessage("startTraffic");
        stopMsg   = new cMessage("stopTraffic");
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        udpSocket.setOutputGate(gate("socketOut"));
        udpSocket.setCallback(this);
        udpSocket.bind(localPort);

        scheduleAt(startTime, startMsg);
        if (stopTime > startTime)
            scheduleAt(stopTime, stopMsg);
    }
}

// Override handleMessage directly to intercept our timers FIRST
// before ApplicationBase::handleMessage touches them
void TrafficGen::handleMessage(cMessage *msg)
{
    if (msg == startMsg) {
        if (isSender && !tryConnect()) return;
        if (trafficMode == "bursty") { burstRemaining = burstSize; inBurst = true; }
        scheduleNextSend();
    }
    else if (msg == stopMsg) {
        if (sendTimer->isScheduled()) cancelEvent(sendTimer);
    }
    else if (msg == sendTimer) {
        sendPacket();
        scheduleNextSend();
    }
    else {
        // All other messages (UDP socket messages) go to ApplicationBase
        ApplicationBase::handleMessage(msg);
    }
}

void TrafficGen::sendPacket()
{
    if (!isSender || !socketConnected) return;
    std::string name = "pkt-" + std::to_string(seqNumber)
                       + "-" + std::to_string(simTime().dbl());
    auto pkt = new inet::Packet(name.c_str());
    pkt->insertAtBack(inet::makeShared<inet::ByteCountChunk>(inet::B(packetSize)));
    emit(packetSentSignal, ++sentCount);
    seqNumber++;
    udpSocket.send(pkt);
    if (trafficMode == "bursty" && inBurst)
        if (--burstRemaining <= 0) inBurst = false;
}

void TrafficGen::scheduleNextSend()
{
    if (!isSender || !socketConnected || simTime() >= stopTime) return;
    if (trafficMode == "cbr")
        scheduleAt(simTime() + sendInterval, sendTimer);
    else if (trafficMode == "bursty") {
        if (inBurst && burstRemaining > 0)
            scheduleAt(simTime() + 0.001, sendTimer);
        else {
            inBurst = true; burstRemaining = burstSize;
            scheduleAt(simTime() + burstGap, sendTimer);
        }
    }
}

void TrafficGen::finish()
{
    recordScalar("packetsSent",     sentCount);
    recordScalar("packetsReceived", receivedCount);
    if (isSender && sentCount > 0)
        recordScalar("PDR_%", 100.0 * receivedCount / sentCount);
    cancelAndDelete(sendTimer); sendTimer = nullptr;
    cancelAndDelete(startMsg);  startMsg  = nullptr;
    cancelAndDelete(stopMsg);   stopMsg   = nullptr;
}
