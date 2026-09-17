#include "LinkController.h"

Define_Module(LinkController);

void LinkController::initialize()
{
    if (!par("enabled").boolValue())
        return;

    // campus.ned lists backbone connections FIRST, so ethg[0] is always the
    // backbone link on both seRouter and coreRouter.
    cModule *seRouter = getModuleByPath("CampusNetwork.seRouter");
    if (!seRouter) {
        EV_ERROR << "[LINK] Cannot find CampusNetwork.seRouter" << endl;
        return;
    }

    cGate *seOut = seRouter->gate("ethg$o", 0);
    if (!seOut) {
        EV_ERROR << "[LINK] seRouter has no ethg$o[0]" << endl;
        return;
    }

    cChannel *rawCh = seOut->getChannel();
    if (!rawCh) {
        EV_ERROR << "[LINK] No channel on seRouter ethg$o[0]" << endl;
        return;
    }

    cDatarateChannel *ch = dynamic_cast<cDatarateChannel*>(rawCh);
    if (!ch) {
        EV_ERROR << "[LINK] Channel is not a cDatarateChannel" << endl;
        return;
    }

    datarate = ch->getDatarate();
    delay    = ch->getDelay();
    per      = ch->getPacketErrorRate();

    EV_INFO << "[LINK] Initialized. SE-Core link: datarate=" << datarate
            << " delay=" << delay << " per=" << per << endl;

    scheduleAt(60.0,  new cMessage("fail"));
    scheduleAt(120.0, new cMessage("recover"));
}

void LinkController::handleMessage(cMessage *msg)
{
    if (!par("enabled").boolValue()) {
        delete msg;
        return;
    }

    cModule *seRouter   = getModuleByPath("CampusNetwork.seRouter");
    cModule *coreRouter = getModuleByPath("CampusNetwork.coreRouter");

    if (!seRouter || !coreRouter) {
        EV_ERROR << "[LINK] Cannot find routers at t=" << simTime() << endl;
        delete msg;
        return;
    }

    // Gate index 0 = backbone on both routers (backbone listed first in campus.ned)
    cGate *seOut   = seRouter->gate("ethg$o", 0);
    cGate *seIn    = seRouter->gate("ethg$i", 0);
    cGate *coreOut = coreRouter->gate("ethg$o", 0);
    cGate *coreIn  = coreRouter->gate("ethg$i", 0);

    if (!seOut || !seIn || !coreOut || !coreIn) {
        EV_ERROR << "[LINK] Gate lookup failed at t=" << simTime() << endl;
        delete msg;
        return;
    }

    if (strcmp(msg->getName(), "fail") == 0)
    {
        // Disconnect forward direction (seRouter -> coreRouter)
        if (seOut->isConnected()) {
            seOut->disconnect();
            EV_INFO << "[LINK] seOut->coreIn DISCONNECTED" << endl;
        }
        // Disconnect reverse direction (coreRouter -> seRouter)
        if (coreOut->isConnected()) {
            coreOut->disconnect();
            EV_INFO << "[LINK] coreOut->seIn DISCONNECTED" << endl;
        }
        EV_INFO << "[LINK] *** SE-Core link FAILED at t=" << simTime() << " ***" << endl;
    }
    else if (strcmp(msg->getName(), "recover") == 0)
    {
        // Reconnect forward direction
        if (!seOut->isConnected()) {
            cDatarateChannel *ch1 = cDatarateChannel::create("seCore_fwd");
            ch1->setDatarate(datarate);
            ch1->setDelay(delay.dbl());
            ch1->setPacketErrorRate(per);
            seOut->connectTo(coreIn, ch1);
            ch1->callInitialize();
            EV_INFO << "[LINK] seOut->coreIn RECONNECTED" << endl;
        }
        // Reconnect reverse direction
        if (!coreOut->isConnected()) {
            cDatarateChannel *ch2 = cDatarateChannel::create("seCore_rev");
            ch2->setDatarate(datarate);
            ch2->setDelay(delay.dbl());
            ch2->setPacketErrorRate(per);
            coreOut->connectTo(seIn, ch2);
            ch2->callInitialize();
            EV_INFO << "[LINK] coreOut->seIn RECONNECTED" << endl;
        }
        EV_INFO << "[LINK] *** SE-Core link RECOVERED at t=" << simTime() << " ***" << endl;
    }

    delete msg;
}
