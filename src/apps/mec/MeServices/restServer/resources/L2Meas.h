#ifndef _L2MEAS_H_
#define _L2MEAS_H_

#include "AttributeBase.h"


#include "CellInfo.h"
#include "common/LteCommon.h"
#include "TimeStamp.h"
// #include "CellUeInfo.h"
#include <vector>
#include <map>

class L2Meas : public AttributeBase
{
	public:
		/**
		 * the constructor takes a vector of te eNodeBs connceted to the MeHost
		 * and creates a CellInfo object
		*/
        L2Meas();
		L2Meas(std::vector<cModule*>& eNodeBs);
		virtual ~L2Meas();

		nlohmann::json toJson() const override;

		void addEnodeB(std::vector<cModule*>& eNodeBs);
		void addEnodeB(cModule* eNodeB);

		nlohmann::json toJson(std::vector<MacNodeId>& cellsID) const;
		nlohmann::json toJson(std::vector<std::string>& uesID) const;
		nlohmann::json toJson(std::vector<MacNodeId>& cellsID, std::vector<MacNodeId>& uesID) const;
		

	protected:
		//better mappa <cellID, Cellingo>

		TimeStamp timestamp_;
		std::map<MacCellId, CellInfo> eNodeBs_;

		

};


#endif // _L2MEAS_H_
