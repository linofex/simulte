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

package lte.stack.packetFlowManager;

//
simple PacketFlowManagerBase
{
}

simple PacketFlowManagerEnb extends PacketFlowManagerBase {
    @class("PacketFlowManagerEnb");
	@signal[delay];
    @statistic[delay](title="Delay at the rlc layer UL"; unit="s"; source="delay"; record=mean,vector);
}

simple PacketFlowManagerUe extends PacketFlowManagerBase {
    @class("PacketFlowManagerUe");
}
