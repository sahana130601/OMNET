package inbaversim.simulations;

import inbaversim.Demiurge;
import inbaversim.InetContentServer;
import inbaversim.InetWiredNode;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import ned.DatarateChannel;

network SimpleInetUdpCcnNetwork
{
    parameters:
        @display("bgb=400,400;bgg=100,0,grey95;bgu=m");

    submodules:
        configurator: Ipv4NetworkConfigurator {
    		@display("p=200,320");
		}

        inetWiredNode1: InetWiredNode {
            @display("p=100,200");
        }

        inetContentServer1: InetContentServer {
            @display("p=300,200");
        }

        demiurge: Demiurge {
            @display("p=200,80");
        }

    connections allowunconnected:
        inetWiredNode1.wiredInOut[0] <--> DatarateChannel { datarate = 1e6bps; } <--> inetContentServer1.wiredInOut[0];
		inetWiredNode1.pppg[0] <--> DatarateChannel { datarate = 1e6bps; } <--> inetContentServer1.pppg[0];
}