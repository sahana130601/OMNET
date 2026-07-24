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

package inbaversim;

simple AdaptationLayer
{
    parameters:
        @class(inbaversim::AdaptationLayer);
        bool udpMode = default(false);
        bool inetUdpMode = default(false);
        int localPort = default(9695);
        int destPort = default(9695);
        string destAddress = default("");
        @display("i=block/layer");

    gates:
        inout forwarderInOut[];
        inout transportInOut[];

        // For real INET UDP integration later
        input socketIn;
        output socketOut;
}
