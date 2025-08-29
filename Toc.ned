//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 


// The receiver module
simple Toc
{
    parameters:
        int windowSizeW; // The receiver's buffer/window size
        int ackFrequencyN; // How many packets to receive before sending an ACK
        double packetLossRate; // Probability of losing a packet (0.0 to 1.0)
        @signal[receivedPkSignal](type="long"); 
        @signal[lostPkSignal](type="long"); 
        @statistic[receivedPackets](title="Packets Received"; source="receivedPkSignal"; record=count);
        @statistic[lostPackets](title="Packets Lost"; source="lostPkSignal"; record=count);
    gates:
        input in;
        output out;
}