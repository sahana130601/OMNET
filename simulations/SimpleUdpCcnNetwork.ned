//
// Copyright (C) 2025 Asanga Udugama (udugama@uni-bremen.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

package inbaversim.simulations;

import inbaversim.AccessRouter;
import inbaversim.ContentServer;
import inbaversim.CoreRouter;
import inbaversim.Demiurge;
import inbaversim.WiredNode;
import inbaversim.WirelessAccessRouter;
import inbaversim.WirelessNode;
import ned.DatarateChannel;


//
// A simple CCN network.
//
network SimpleUdpCcnNetwork
{
    parameters:
        @display("bgb=400,400;bgg=100,0,grey95;bgu=m");

    submodules:
        wiredNode1: WiredNode {
            @display("p=100,200");
        }
        contentServer1: ContentServer {
            @display("p=300,200");
        }
        demiurge: Demiurge {
            @display("p=200,80");
        }

    connections allowunconnected:
        wiredNode1.wiredInOut++ <--> DatarateChannel { datarate = 1e6bps; } <--> contentServer1.wiredInOut++;
}
