/*
 * Tic.cc
 *
 * Created on: 29 Aug 2025
 * Author: razeek_j
 */

#include "Tic.h"
#include "GoBackN_m.h"
#include <cmath>

Define_Module(Tic);

void Tic::initialize()
{
    // Initialize sender state variables
    windowSizeW = 0;
    sendBase = 0;
    nextSeqNum = 0;
    timeoutEvent = new cMessage("timeoutEvent");
    sendTimer = new cMessage("sendTimer");
    unackedPackets.setName("unackedPackets");
    isReceiverReady = true; // Assume the receiver is ready at the start

    // Read seqNumBits parameter and calculate the limit
    int bits = par("seqNumBits");
    seqNumLimit = pow(2, bits);
    EV << "Sequence number limit set to " << seqNumLimit << " (" << bits << " bits)\n";

    // Send the initial query to find out the receiver's window size
    EV << "Tic initializing: Sending QUERY_REQUEST to Toc\n";
    GoBackNPacket *query = new GoBackNPacket("Query");
    query->setKind(QUERY_REQUEST);
    query->setSequenceNumber(0);
    send(query, "out");
}

void Tic::handleMessage(cMessage *msg)
{
    //================================================================
    //== 1. HANDLE SELF-MESSAGES (TIMERS)
    //================================================================
    if (msg->isSelfMessage()) {
        if (msg == sendTimer) {
            if (isReceiverReady && nextSeqNum < sendBase + windowSizeW) {
                GoBackNPacket *pkt = new GoBackNPacket("Data");
                pkt->setKind(DATA);
                pkt->setSequenceNumber(nextSeqNum);
                send(pkt->dup(), "out");
                EV << "Tic sending DATA, seq=" << nextSeqNum << "\n";
                unackedPackets.insert(pkt);
                if (!timeoutEvent->isScheduled()) {
                    scheduleAt(simTime() + 5.0, timeoutEvent);
                }
                nextSeqNum = (nextSeqNum + 1) % seqNumLimit;
                // Schedule the next transmission
                scheduleAt(simTime() + 1.0 / par("dataRate").doubleValue(), sendTimer);
            }
        }
        else if (msg == timeoutEvent) {
            EV << "!!! TIMEOUT expired. Resending window starting from seq=" << sendBase << "\n";
            bubble("Timeout! Resending.");
            scheduleAt(simTime() + 5.0, timeoutEvent);
            for (cQueue::Iterator iter(unackedPackets); !iter.end(); iter++) {
                GoBackNPacket *pkt = check_and_cast<GoBackNPacket *>(*iter);
                send(pkt->dup(), "out");
                EV << "--> Tic RE-sending DATA, seq=" << pkt->getSequenceNumber() << "\n";
            }
        }
        return;
    }

    //================================================================
    //== 2. HANDLE INCOMING PACKETS FROM TOC
    //================================================================
    if (uniform(0, 1) < par("packetLossRate").doubleValue()) {
        EV << "!!! Tic: Incoming packet LOST (likely an ACK/RR/RNR)\n";
        bubble("Incoming packet lost!");
        delete msg;
        return;
    }

    GoBackNPacket *packet = check_and_cast<GoBackNPacket *>(msg);

    if (packet->getKind() == QUERY_REPLY) {
        windowSizeW = packet->getSequenceNumber();
        EV << "Tic received QUERY_REPLY from Toc with window size W=" << windowSizeW << "\n";
        cancelEvent(sendTimer); // SAFETY FIX
        scheduleAt(simTime(), sendTimer);

    } else if (packet->getKind() == ACK) {
        int ackNum = packet->getSequenceNumber();
        EV << "<-- Tic received ACK for seq=" << ackNum << "\n";
        sendBase = ackNum;
        while (!unackedPackets.isEmpty()) {
            GoBackNPacket *pktInQueue = check_and_cast<GoBackNPacket *>(unackedPackets.front());
            if (pktInQueue->getSequenceNumber() == sendBase) {
                 break;
            }
            delete (cPacket *)unackedPackets.pop();
        }
        cancelEvent(timeoutEvent);
        if (!unackedPackets.isEmpty()) {
            scheduleAt(simTime() + 5.0, timeoutEvent);
            EV << "Timer restarted for new sendBase.\n";
        } else {
            EV << "All packets ACKed, timer is stopped.\n";
        }
    }
    else if (packet->getKind() == RNR) {
        EV << "<-- Tic received RNR. Pausing transmission.\n";
        bubble("RNR received!");
        isReceiverReady = false;
    } else if (packet->getKind() == RR) {
        EV << "<-- Tic received RR. Resuming transmission.\n";
        bubble("RR received!");
        isReceiverReady = true;
        cancelEvent(sendTimer); // SAFETY FIX
        scheduleAt(simTime(), sendTimer);
    }

    delete packet;
}
