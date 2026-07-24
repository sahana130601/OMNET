#include <omnetpp.h>
#include <cstring>

#include "TransportMessages_m.h"
#include "RFC8609Messages_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include <sstream>
#include <vector>

using namespace omnetpp;

namespace inbaversim {

class AdaptationLayer : public cSimpleModule
{
  private:
    inet::UdpSocket socket;
    bool socketReady = false;

    void setupSocket();
    std::string serializeCcnPacket(cPacket *packet);
    cPacket *deserializeCcnPacket(const char *data);

  protected:
      virtual void initialize() override;
      virtual void handleMessage(cMessage *msg) override;
};

Define_Module(AdaptationLayer);

void AdaptationLayer::initialize()
{
    bool inetUdpMode = par("inetUdpMode").boolValue();

    if (inetUdpMode) {
        scheduleAt(SIMTIME_ZERO, new cMessage("setupInetSocket"));
    }
}

void AdaptationLayer::setupSocket()
{
    if (socketReady)
        return;

    int localPort = par("localPort").intValue();

    socket.setOutputGate(gate("socketOut"));
    socket.bind(inet::L3Address(), localPort);

    socketReady = true;

    EV_INFO << "INET UDP socket initialized on localPort=" << localPort << "\n";
}
std::string AdaptationLayer::serializeCcnPacket(cPacket *packet)
{
    std::ostringstream os;

    if (auto *interest = dynamic_cast<InterestMsg *>(packet)) {
        os << "INTEREST|"
           << interest->getPrefixName() << "|"
           << interest->getDataName() << "|"
           << interest->getVersionName() << "|"
           << interest->getSegmentNum() << "|"
           << interest->getHopLimit() << "|"
           << interest->getLifetime().dbl() << "|"
           << interest->getHeaderSize() << "|"
           << interest->getPayloadSize() << "|"
           << interest->getHopsTravelled() << "|"
           << interest->getReflexiveNamePrefix() << "|"
           << interest->getForwardPendingToken() << "|"
           << interest->getReversePendingToken();
    }
    else if (auto *content = dynamic_cast<ContentObjMsg *>(packet)) {
        os << "CONTENT|"
           << content->getPrefixName() << "|"
           << content->getDataName() << "|"
           << content->getVersionName() << "|"
           << content->getSegmentNum() << "|"
           << content->getCachetime().dbl() << "|"
           << content->getExpirytime().dbl() << "|"
           << content->getHeaderSize() << "|"
           << content->getPayloadSize() << "|"
           << content->getTotalNumSegments() << "|"
           << content->getPayloadAsString() << "|"
           << content->getReversePendingToken();
    }
    else {
        os << "UNKNOWN|" << packet->getName();
    }

    return os.str();
}
cPacket *AdaptationLayer::deserializeCcnPacket(const char *data)
{
    std::vector<std::string> fields;
    std::stringstream ss(data);
    std::string item;

    while (std::getline(ss, item, '|')) {
        fields.push_back(item);
    }

    if (fields.empty())
        return nullptr;

    if (fields[0] == "INTEREST" && fields.size() >= 13) {
        auto *interest = new InterestMsg("Interest");

        interest->setPrefixName(fields[1].c_str());
        interest->setDataName(fields[2].c_str());
        interest->setVersionName(fields[3].c_str());
        interest->setSegmentNum(std::stoi(fields[4]));
        interest->setHopLimit(std::stoi(fields[5]));
        interest->setLifetime(SimTime(std::stod(fields[6])));
        interest->setHeaderSize(std::stoi(fields[7]));
        interest->setPayloadSize(std::stoi(fields[8]));
        interest->setHopsTravelled(std::stoi(fields[9]));
        interest->setReflexiveNamePrefix(fields[10].c_str());
        interest->setForwardPendingToken(std::stoi(fields[11]));
        interest->setReversePendingToken(std::stoi(fields[12]));

        interest->setByteLength(interest->getHeaderSize() + interest->getPayloadSize());

        return interest;
    }

    if (fields[0] == "CONTENT" && fields.size() >= 12) {
        auto *content = new ContentObjMsg("ContentObj");

        content->setPrefixName(fields[1].c_str());
        content->setDataName(fields[2].c_str());
        content->setVersionName(fields[3].c_str());
        content->setSegmentNum(std::stoi(fields[4]));
        content->setCachetime(SimTime(std::stod(fields[5])));
        content->setExpirytime(SimTime(std::stod(fields[6])));
        content->setHeaderSize(std::stoi(fields[7]));
        content->setPayloadSize(std::stoi(fields[8]));
        content->setTotalNumSegments(std::stoi(fields[9]));
        content->setPayloadAsString(fields[10].c_str());
        content->setReversePendingToken(std::stoi(fields[11]));

        content->setByteLength(content->getHeaderSize() + content->getPayloadSize());

        return content;
    }

    return nullptr;
}

void AdaptationLayer::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (strcmp(msg->getName(), "setupInetSocket") == 0) {
            setupSocket();
        }

        delete msg;
        return;
    }

    cGate *arrivalGate = msg->getArrivalGate();

    if (arrivalGate == nullptr) {
        EV_WARN << "AdaptationLayer received message without arrival gate\n";
        delete msg;
        return;
    }

    const char *gateBaseName = arrivalGate->getBaseName();

    int gateIndex = -1;
    if (arrivalGate->isVector())
        gateIndex = arrivalGate->getIndex();

    bool udpMode = par("udpMode").boolValue();
    bool inetUdpMode = par("inetUdpMode").boolValue();

    /*
     * Direction 1:
     * Forwarder -> AdaptationLayer -> Transport
     */
    if (strcmp(gateBaseName, "forwarderInOut") == 0) {
        EV_INFO << "AdaptationLayer received packet from FORWARDER: "
                << msg->getName()
                << ", udpMode=" << udpMode
                << ", inetUdpMode=" << inetUdpMode
                << "\n";

        if (inetUdpMode) {
            setupSocket();

            cPacket *ccnPacket = check_and_cast<cPacket *>(msg);

            auto *inetPacket = new inet::Packet(ccnPacket->getName());

            EV_INFO << "INET UDP MODE: creating INET UDP packet for CCN packet: "
                    << ccnPacket->getName() << "\n";

            std::string serialized = serializeCcnPacket(ccnPacket);

            inetPacket->setName(serialized.c_str());

            const auto& payload = inet::makeShared<inet::ApplicationPacket>();
            payload->setChunkLength(inet::B(serialized.size()));
            payload->setSequenceNumber(0);

            inetPacket->insertAtBack(payload);

            delete ccnPacket;

            const char *destAddress = par("destAddress").stringValue();
            int destPort = par("destPort").intValue();

            inet::L3Address destAddr;
            inet::L3AddressResolver().tryResolve(destAddress, destAddr);

            EV_INFO << "INET UDP MODE: sending CCN packet through INET UDP: "
                    << inetPacket->getName()
                    << " to " << destAddress
                    << ":" << destPort
                    << "\n";

            socket.sendTo(inetPacket, destAddr, destPort);
        }
        else if (udpMode) {
            EV_INFO << "UDP MODE: encapsulating CCN packet into UDP wrapper: "
                    << msg->getName() << "\n";

            cPacket *ccnPacket = check_and_cast<cPacket *>(msg);

            auto *udpWrapper = new UdpEncapsulatedCcnMsg("UdpEncapsulatedCcnMsg");

            udpWrapper->setSrcPort(par("localPort").intValue());
            udpWrapper->setDestPort(par("destPort").intValue());
            udpWrapper->setSrcAddress("local");
            udpWrapper->setDestAddress(par("destAddress").stringValue());
            udpWrapper->setUdpHeaderSize(8);
            udpWrapper->setPayloadSize(ccnPacket->getByteLength());

            udpWrapper->setByteLength(8);
            udpWrapper->encapsulate(ccnPacket);

            send(udpWrapper, "transportInOut$o", gateIndex);
        }
        else {
            send(msg, "transportInOut$o", gateIndex);
        }
    }
    /*
     * Direction 2:
     * Transport -> AdaptationLayer -> Forwarder
     */
    else if (strcmp(gateBaseName, "transportInOut") == 0) {
        EV_INFO << "AdaptationLayer received packet from TRANSPORT: "
                << msg->getName()
                << ", udpMode=" << udpMode
                << ", inetUdpMode=" << inetUdpMode
                << "\n";

        if (udpMode) {
            auto *udpWrapper = dynamic_cast<UdpEncapsulatedCcnMsg *>(msg);

            if (udpWrapper != nullptr) {
                EV_INFO << "UDP MODE: decapsulating UDP wrapper back into CCN packet\n";

                cPacket *ccnPacket = udpWrapper->decapsulate();

                EV_INFO << "UDP MODE: extracted CCN packet: "
                        << ccnPacket->getName() << "\n";

                delete udpWrapper;

                send(ccnPacket, "forwarderInOut$o", gateIndex);
            }
            else {
                EV_WARN << "UDP MODE: expected UdpEncapsulatedCcnMsg but received "
                        << msg->getClassName()
                        << ". Passing through unchanged.\n";

                send(msg, "forwarderInOut$o", gateIndex);
            }
        }
        else {
            send(msg, "forwarderInOut$o", gateIndex);
        }
    }
    else if (strcmp(gateBaseName, "socketIn") == 0) {
        EV_INFO << "INET UDP MODE: received packet from socketIn: "
                << msg->getName() << "\n";

        auto *inetPacket = check_and_cast<inet::Packet *>(msg);

        cPacket *ccnPacket = deserializeCcnPacket(inetPacket->getName());

        if (ccnPacket != nullptr) {
            EV_INFO << "INET UDP MODE: recreated CCN packet from INET UDP packet: "
                    << ccnPacket->getName() << "\n";

            delete inetPacket;

            send(ccnPacket, "forwarderInOut$o", 0);
        }
        else {
            EV_WARN << "INET UDP MODE: could not recreate CCN packet from received INET packet\n";
            delete inetPacket;
        }
    }
    else {
        EV_WARN << "AdaptationLayer received message on unknown gate: "
                << arrivalGate->getFullName() << "\n";
        delete msg;
    }
}
} // namespace inbaversim

