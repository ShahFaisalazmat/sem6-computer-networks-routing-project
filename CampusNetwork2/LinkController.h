#ifndef LINKCONTROLLER_H
#define LINKCONTROLLER_H

#include <omnetpp.h>

using namespace omnetpp;

/**
 * LinkController manages the SE-Core backbone link failure scenario.
 * Disconnects the SE-Core link at t=60s and reconnects at t=120s.
 *
 * Gate index assumption (guaranteed by campus.ned connection order):
 *   seRouter.ethg[0]   = backbone to coreRouter
 *   coreRouter.ethg[0] = backbone to seRouter
 */
class LinkController : public cSimpleModule
{
  private:
    double    datarate;
    simtime_t delay;
    double    per;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif // LINKCONTROLLER_H
