#ifndef TRAFFICGEN_H
#define TRAFFICGEN_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/common/packet/Packet.h"

using namespace omnetpp;

class TrafficGen : public inet::ApplicationBase, public inet::UdpSocket::ICallback
{
  private:
    std::string trafficMode, destAddress;
    int destPort, localPort, packetSize, burstSize;
    double sendInterval, burstGap, startTime, stopTime;
    long seqNumber, sentCount, receivedCount;
    int burstRemaining;
    bool inBurst, isSender, socketConnected;
    cMessage *sendTimer = nullptr;
    cMessage *startMsg  = nullptr;
    cMessage *stopMsg   = nullptr;
    inet::UdpSocket udpSocket;
    simsignal_t packetSentSignal, packetReceivedSignal, endToEndDelaySignal;
    void sendPacket();
    void scheduleNextSend();
    bool tryConnect();
  protected:
    virtual void initialize(int stage) override;
    virtual int  numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenUp(cMessage *msg) override {}
    virtual void handleStartOperation(inet::LifecycleOperation *op) override {}
    virtual void handleStopOperation(inet::LifecycleOperation *op)  override {}
    virtual void handleCrashOperation(inet::LifecycleOperation *op) override {}
    virtual void finish() override;
    virtual void socketDataArrived(inet::UdpSocket *s, inet::Packet *p) override;
    virtual void socketErrorArrived(inet::UdpSocket *s, inet::Indication *ind) override;
    virtual void socketClosed(inet::UdpSocket *s) override;
};

#endif
