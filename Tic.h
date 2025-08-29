/*
 * Tic.h
 *
 *  Created on: 29 Aug 2025
 *      Author: razeek_j
 */

#ifndef __GOBACKN_TIC_H_
#define __GOBACKN_TIC_H_

#include <omnetpp.h>

using namespace omnetpp;

class Tic : public cSimpleModule
{
  private:
    // Variables to hold the state of the sender
    int windowSizeW;
    int sendBase;
    int nextSeqNum;
    cMessage *timeoutEvent; // A timer for retransmissions
    cMessage *sendTimer;    // A timer for sending new data packets
    cQueue unackedPackets; // to buffer unacked packets
    int seqNumLimit;
    bool isReceiverReady;

  protected:
    // The following methods are redefined from cSimpleModule
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
