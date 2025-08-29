/*
 * Toc.cc
 *
 * Created on: 29 Aug 2025
 * Author: razeek_j
 */

#include "Toc.h"
#include "GoBackN_m.h"

Define_Module(Toc);

void Toc::initialize()
{
    // Read parameters from the omnetpp.ini file
    windowSizeW = par("windowSizeW");
    ackFrequencyN = par("ackFrequencyN");
    buffer.setName("receiverBuffer");

    // Initialize receiver state
    expectedSeqNum = 0;
    packetsSinceLastAck = 0;
    isRnrState = false;

    // Register signals for statistics collection
    receivedPkSignal = registerSignal("receivedPkSignal");
    lostPkSignal = registerSignal("lostPkSignal");

    EV << "Toc Initialized. Buffer Size (W): " << windowSizeW << ", ACK Freq (N): " << ackFrequencyN << "\n";

    // NOTE: The processing timer is no longer scheduled here at the start.
    // It will be scheduled only when the first packet arrives in the buffer.
}

void Toc::handleMessage(cMessage *msg)
{
    //================================================================
    //== 1. HANDLE SELF-MESSAGE FOR BUFFER PROCESSING
    //================================================================
    if (msg->isSelfMessage()) {
        // This simulates the application layer processing a packet from the buffer
        if (!buffer.isEmpty()) {
            EV << "Toc processing packet from buffer, freeing up space.\n";
            delete (cPacket *)buffer.pop(); // Remove one packet from the buffer

            // If we were in an RNR state and now have space, send an RR message
            if (isRnrState && buffer.getLength() < windowSizeW) {
                EV << "--> Toc buffer has space. Sending RR.\n";
                isRnrState = false;
                GoBackNPacket *rr = new GoBackNPacket("RR");
                rr->setKind(RR);
                rr->setSequenceNumber(expectedSeqNum);
                send(rr, "out");
            }
        }

        // --- EFFICIENT TIMER LOGIC ---
        // Only reschedule the timer if there are still packets left to process.
        if (!buffer.isEmpty()) {
            scheduleAt(simTime() + 0.01, msg);
        } else {
            // If the buffer is empty, the timer's job is done.
            // We delete the message to prevent a memory leak.
            delete msg;
        }
        return; // End of self-message logic
    }

    //================================================================
    //== 2. HANDLE MESSAGES FROM THE SENDER (TIC)
    //================================================================
    GoBackNPacket *packet = check_and_cast<GoBackNPacket *>(msg);

    if (packet->getKind() == QUERY_REQUEST) {
        EV << "Toc received QUERY_REQUEST from Tic.\n";
        GoBackNPacket *reply = new GoBackNPacket("QueryReply");
        reply->setKind(QUERY_REPLY);
        reply->setSequenceNumber(windowSizeW);
        EV << "Toc sending QUERY_REPLY with W=" << windowSizeW << " back to Tic.\n";
        send(reply, "out");

    } else if (packet->getKind() == DATA) {
        if (buffer.getLength() >= windowSizeW && !isRnrState) {
            EV << "!!! Toc buffer is FULL. Sending RNR.\n";
            isRnrState = true;
            GoBackNPacket *rnr = new GoBackNPacket("RNR");
            rnr->setKind(RNR);
            rnr->setSequenceNumber(expectedSeqNum);
            send(rnr, "out");
        }

        if (uniform(0, 1) < par("packetLossRate").doubleValue()) {
            EV << "!!! Toc: Packet LOST, seq=" << packet->getSequenceNumber() << "\n";
            bubble("Packet lost!");
            emit(lostPkSignal, 1L);
            delete packet;
            return;
        }

        if (packet->getSequenceNumber() == expectedSeqNum) {
            EV << "Toc received correct DATA, seq=" << expectedSeqNum << ", adding to buffer.\n";
            emit(receivedPkSignal, 1L);

            // --- EFFICIENT TIMER LOGIC ---
            // If the buffer was empty, this is the first new packet, so "wake up" the processing timer.
            bool bufferWasEmpty = buffer.isEmpty();
            buffer.insert(packet->dup());
            if (bufferWasEmpty) {
                 scheduleAt(simTime() + 0.01, new cMessage("processingTimer"));
            }

            expectedSeqNum = (expectedSeqNum + 1) % 8; // Assuming 3 bits for wrap-around consistency
            packetsSinceLastAck++;

            if (packetsSinceLastAck >= ackFrequencyN) {
                GoBackNPacket *ack = new GoBackNPacket("Ack");
                ack->setKind(ACK);
                ack->setSequenceNumber(expectedSeqNum);
                send(ack, "out");
                EV << "--> Toc sending cumulative ACK for seq=" << expectedSeqNum << "\n";
                packetsSinceLastAck = 0;
            }
        } else {
            EV << "!!! Toc received OUT-OF-ORDER packet, seq=" << packet->getSequenceNumber()
               << ". Expected: " << expectedSeqNum << ". Discarding.\n";
            GoBackNPacket *ack = new GoBackNPacket("Ack");
            ack->setKind(ACK);
            ack->setSequenceNumber(expectedSeqNum);
            send(ack, "out");
            EV << "--> Toc re-sending cumulative ACK for seq=" << expectedSeqNum << "\n";
            packetsSinceLastAck = 0;
        }
    }
    delete packet;
}
