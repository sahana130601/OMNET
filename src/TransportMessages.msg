//
// Copyright (C) 2025 Asanga Udugama (udugama@uni-bremen.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

//
// Transport message that encapsulates CCNx messages
//
//
// Existing inbaverSim transport wrapper
//
packet TransportMsg
{
    // source, destination addresses
    string sourceAddress;
    bool broadcastMsg;
    string destinationAddress;

    // msg sizes in bytes
    int headerSize;
    int payloadSize;
}

//
// Experimental wrapper for CCN-over-UDP adaptation layer
//
packet UdpEncapsulatedCcnMsg
{
    int srcPort;
    int destPort;
    string srcAddress;
    string destAddress;
    int udpHeaderSize;
    int payloadSize;
}

