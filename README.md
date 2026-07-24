## AdaptationLayer with INET UDP Integration

This project extends inbaverSim by adding an AdaptationLayer below the RFC8569Forwarder. The layer supports pass-through operation, wrapper-based encapsulation, and UDP/IP-based transmission using the INET framework.

Main added/modified files:

- `src/AdaptationLayer.cc`
- `src/AdaptationLayer.ned`
- `src/TransportMessages.msg`
- `src/InetCcnNode.ned`
- `src/InetWiredNode.ned`
- `src/InetContentServer.ned`
- `simulations/SimpleInetUdpCcnNetwork.ned`

The INET UDP mode uses INET's `UdpSocket` API to transmit CCN packet information through the UDP/IP stack.
