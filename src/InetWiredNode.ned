package inbaversim;

module InetWiredNode extends InetCcnNode
{
    parameters:
        @networkNode();

        int inetWiredNodeNumWiredTransports = default(1);
        int inetWiredNodeNumWirelessTransports = default(0);

        string forwarder = default("RFC8569WithRFForwarder");

        double ethDataRate @unit(bps) = default(1e6bps);
        double ethPacketErrorRate @unit(pct) = default(0.0pct);

        string mobility = default("StationaryMobility");

        fwd.typename = forwarder;

        numWiredTransports = inetWiredNodeNumWiredTransports;
        numWirelessTransports = inetWiredNodeNumWirelessTransports;

        wired[0].typename = "WiredTransport";
        wired[0].dataRate = ethDataRate;
        wired[0].packetErrorRate = ethPacketErrorRate;
        wired[0].headerSize = 32byte;

        //adaptationLayer.udpMode = true;
        //adaptationLayer.inetUdpMode = false;

        mob.typename = mobility;

        @display("i=device/pc2;bgb=418,317");
}