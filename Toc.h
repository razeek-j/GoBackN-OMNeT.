/*
 * Toc.h
 *
 *  Created on: 29 Aug 2025
 *      Author: razeek_j
 */

#ifndef __GOBACKN_TOC_H_
#define __GOBACKN_TOC_H_

#include <omnetpp.h>

using namespace omnetpp;

class Toc : public cSimpleModule
{
  private:
    // Variables to hold the state of the receiver
    int windowSizeW;
    int ackFrequencyN;
    int expectedSeqNum;
    int packetsSinceLastAck;

    //Buffer and RNR state ---
    cQueue buffer;
    bool isRnrState;

    // Signal handles for statistics
    simsignal_t receivedPkSignal;
    simsignal_t lostPkSignal;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
