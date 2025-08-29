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
// Define an enum for different message types
enum MessageType {
    QUERY_REQUEST = 0;
    QUERY_REPLY = 1;
    DATA = 2;
    ACK = 3;
    RR = 4;
    RNR = 5;
};

// A generic base packet
packet GoBackNPacket {
    int kind @enum(MessageType); // Use the enum to identify the message type
    int sequenceNumber;
}