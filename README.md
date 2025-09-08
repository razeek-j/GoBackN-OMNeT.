# Go-Back-N Simulation in OMNeT++

This project implements the **Go-Back-N ARQ protocol** using OMNeT++ and demonstrates flow control, packet loss handling, and retransmission mechanisms.

Demo: https://youtu.be/LoDON5u7SEE

## Features / Project Requirements
The project fulfills all core requirements and bonus tasks:

### Tic (Sender)
- Sends packets at a configurable **data rate** (`*.tic.dataRate`)  
- Maintains **send window** (`sendBase`, `nextSeqNum`)  
- Keeps a **queue of unacknowledged packets** for retransmission (bonus task)  
- Handles **timeouts** for lost packets  

### Toc (Receiver)
- Maintains a **receive buffer** (`cQueue`) (bonus task)  
- Supports **cumulative ACKs** after `N` packets (`*.toc.ackFrequencyN`)  
- Sends **RR/RNR messages** to control sender transmission based on buffer state  
- Simulates **packet loss** (`*.toc.packetLossRate`)  

### Protocol Logic
- Initial handshake: **QUERY_REQUEST → QUERY_REPLY** triggers sender to start  
- Sliding window flow control implemented with **sequence number wrap-around**  
- Correct handling of **out-of-order packets**  
- Demonstrates operation **with and without packet loss**  

### Configuration
All key parameters are configurable via `omnetpp.ini`:
- Data rate  
- Window size (`W`)  
- ACK frequency (`N`)  
- Packet loss rate  
- Sequence number bits  

---

## How to Run
1. **Import Project** into OMNeT++ IDE.  
2. **Build Project** (generates `_m.cc` and `_m.h` from the `.msg` file).  
3. Configure `omnetpp.ini` as needed (data rate, window size, ACK frequency, packet loss rate).  
4. **Run Simulation** via OMNeT++ and analyze results with generated graphs.  

