#ifndef _L2MEAS_H_
#define _L2MEAS_H_

#include "common/LteCommon.h"
#include <vector>
#include <map>
#include "corenetwork/nodes/mec/MEPlatform/MeServices/Resources/AttributeBase.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/RNIService/resources/CellInfo.h"
#include "corenetwork/nodes/mec/MEPlatform/MeServices/Resources/TimeStamp.h"

class EnodeBStatsCollector;

class L2Meas : public AttributeBase
{
	public:
		/**
		 * the constructor takes a vector of the eNodeBs connected to the MeHost
		 * and creates a CellInfo object
		*/
        L2Meas();
		L2Meas(std::vector<cModule*>& eNodeBs);
		virtual ~L2Meas();

		nlohmann::ordered_json toJson() const override;

		void addEnodeB(std::vector<cModule*>& eNodeBs);
		void addEnodeB(cModule* eNodeB);

		nlohmann::ordered_json toJsonCell(std::vector<MacCellId>& cellsID) const;
		nlohmann::ordered_json toJsonUe(std::vector<MacNodeId>& uesID) const;
		nlohmann::ordered_json toJson(std::vector<MacNodeId>& cellsID, std::vector<MacNodeId>& uesID) const;
		

	protected:
		//better mappa <cellID, Cellingo>

		TimeStamp timestamp_;
		std::map<MacCellId, EnodeBStatsCollector*> eNodeBs_;

		

};


#endif // _L2MEAS_H_