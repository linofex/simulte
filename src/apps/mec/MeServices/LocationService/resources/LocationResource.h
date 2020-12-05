#ifndef _LOCATION_H_
#define _LOCATION_H_

#include "common/LteCommon.h"
#include <vector>
#include <map>
#include "corenetwork/binder/LteBinder.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "apps/mec/MeServices/Resources/TimeStamp.h"

class LteCellInfo;

class LocationResource : public AttributeBase
{
	public:
		/**
		 * the constructor takes a vector of te eNodeBs connceted to the MeHost
		 * and creates a CellInfo object
		*/
        LocationResource();
		LocationResource(std::string& baseUri, std::vector<cModule*>& eNodeBs, LteBinder* binder);
		virtual ~LocationResource();

		nlohmann::ordered_json toJson() const override;

		void addEnodeB(std::vector<cModule*>& eNodeBs);
		void addEnodeB(cModule* eNodeB);
		void addBinder(LteBinder* binder);
		void setBaseUri(const std::string& baseUri);
		nlohmann::ordered_json toJsonCell(std::vector<MacCellId>& cellsID) const;
		nlohmann::ordered_json toJsonUe(std::vector<IPv4Address>& uesID) const;
		nlohmann::ordered_json toJson(std::vector<MacNodeId>& cellsID, std::vector<IPv4Address>& uesID) const;
		

	protected:
		//better mappa <cellID, Cellingo>
		LteBinder *binder_;
		TimeStamp timestamp_;
		std::map<MacCellId, LteCellInfo*> eNodeBs_;
		std::string baseUri_;
};


#endif // _LOCATION_H_
