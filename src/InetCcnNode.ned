package inbaversim;

import inet.mobility.contract.IMobility;
import inet.common.MessageDispatcher;
import inet.transportlayer.udp.Udp;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.Ipv4NetworkLayer;
import inet.linklayer.ppp.PppInterface;

module InetCcnNode
{
    parameters:
        @networkNode();

        int numApps = default(0);
        int numWiredTransports = default(0);
        int numWirelessTransports = default(0);
		int numPppInterfaces = default(1);
        @display("bgb=430,330");

    gates:
        inout wiredInOut[numWiredTransports];
		inout pppg[numPppInterfaces] @labels(PppFrame-conn) @allowUnconnected;

    submodules:
        numen: Numen {
            @display("p=43,39;t=Numen,t");
        }

        app[numApps]: <> like IApplication {
            @display("p=185,39;t=Applications,t");
        }

        fwd: <> like IForwarder {
            @display("p=185,120;t=Forwarder,t");
        }

        adaptationLayer: AdaptationLayer {
            @display("p=185,190;t=AdaptationLayer,t");
        }
        
        at: MessageDispatcher {
   			 @display("p=300,190;t=AppToTransport,t");
		}

		udp: Udp {
    		parameters:
       		 interfaceTableModule = "^.interfaceTable";
  		     @display("p=300,250;t=UDP,t");
		}
		
		tn: MessageDispatcher {
    		@display("p=300,310;t=TransportToNetwork,t");
		}

		ipv4: Ipv4NetworkLayer {
    		parameters:
        	interfaceTableModule = "^.interfaceTable";
        	@display("p=300,370;t=IPv4,t");
		}
		
		cb: MessageDispatcher {
    		@display("p=300,430;t=NetworkToLink,t");
		}

		li: MessageDispatcher {
  			@display("p=300,490;t=LinkInterface,t");
		}

		ppp[sizeof(pppg)]: PppInterface {
    		parameters:
        		interfaceTableModule = "^.interfaceTable";
        		@display("p=300,550,row,100;q=txQueue");
}

		interfaceTable: InterfaceTable {
    		@display("p=300,310;t=InterfaceTable,t");
		}

        wired[numWiredTransports]: <> like ITransport {
            @display("p=91,270;t=WiredTransports,t");
        }

        wireless[numWirelessTransports]: <> like ITransport {
            @display("p=287,270;t=WirelessTransports,t");
        }

        mob: <> like IMobility {
            @display("p=326,39;t=Mobility,t");
        }

    connections allowunconnected:
        
        adaptationLayer.socketOut --> at.in++;
		adaptationLayer.socketIn <-- at.out++;
		
		at.out++ --> udp.appIn;
		at.in++ <-- udp.appOut;
		
		udp.ipOut --> tn.in++;
		udp.ipIn <-- tn.out++;

		tn.out++ --> ipv4.transportIn;
		tn.in++ <-- ipv4.transportOut;
        
        ipv4.ifOut --> cb.in++;
		ipv4.ifIn <-- cb.out++;

		cb.out++ --> li.in++;
		li.out++ --> cb.in++;

		for i=0..sizeof(pppg)-1 {
    		pppg[i] <--> ppp[i].phys;
		}

		for i=0..sizeof(pppg)-1 {
    		ppp[i].upperLayerOut --> li.in++;
   		 	ppp[i].upperLayerIn <-- li.out++;
		}

        for i=0..numApps-1 {
            app[i].forwarderInOut <--> fwd.appInOut++;
        }

        for i=0..numWirelessTransports-1 {
            fwd.transportInOut++ <--> adaptationLayer.forwarderInOut++;
            adaptationLayer.transportInOut++ <--> wireless[i].forwarderInOut;
        }

        for i=0..numWiredTransports-1 {
            fwd.transportInOut++ <--> adaptationLayer.forwarderInOut++;
            adaptationLayer.transportInOut++ <--> wired[i].forwarderInOut;
            wired[i].physicalInOut <--> wiredInOut[i];
        }
}