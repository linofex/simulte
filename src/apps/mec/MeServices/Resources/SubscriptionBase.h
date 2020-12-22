/*
 * MeasRepUeNotification.h
 *
 *  Created on: Dec 10, 2020
 *      Author: linofex
 */

#ifndef APPS_MEC_MESERVICES_RNISERVICE_RESOURCES_MEASREPUENOTIFICATION_H_
#define APPS_MEC_MESERVICES_RNISERVICE_RESOURCES_MEASREPUENOTIFICATION_H_

#include "apps/mec/MeServices/Resources/AttributeBase.h"
#include "apps/mec/MeServices/Resources/TimeStamp.h"
#include "apps/mec/MeServices/RNIService/resources/Ecgi.h"
#include "apps/mec/MeServices/RNIService/resources/AssociateId.h"

class EnodeBStatsCollector;
class MeasRepUeNotification : public AttributeBase
{
    public:
        MeasRepUeNotification();
        MeasRepUeNotification(std::vector<cModule*>& eNodeBs);
        virtual ~MeasRepUeNotification();

        nlohmann::ordered_json toJson() const override;

        void addEnodeB(std::vector<cModule*>& eNodeBs);
        void addEnodeB(cModule* eNodeB);

        nlohmann::ordered_json toJsonCell(std::vector<MacCellId>& cellsID) const;
        nlohmann::ordered_json toJsonUe(std::vector<MacNodeId>& uesID) const;
        nlohmann::ordered_json toJson(std::vector<MacCellId>& cellsID, std::vector<MacNodeId>& uesID) const;

        bool startNotification(std::vector<MacCellId>& cellsID, std::vector<MacNodeId>& uesID, Trigger trigger);
        void stopNotification(std::vector<MacCellId>& cellsID, std::vector<MacNodeId>& uesID, Trigger trigger);


    protected:


        TimeStamp timestamp_;
        std::map<MacCellId, EnodeBStatsCollector*> eNodeBs_;




};






#endif /* APPS_MEC_MESERVICES_RNISERVICE_RESOURCES_MEASREPUENOTIFICATION_H_ */
