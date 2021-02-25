
#ifndef _LOCATIONSERVICE_TEST_normal_H
#define _LOCATIONSERVICE_TEST_normal_H

#include "corenetwork/nodes/mec/MEPlatform/MeServices/LocationService/LocationService.h"


/**
 *
 *
 *
 */

//typedef struct{
//    inet::TCPSocket *socket;
//    std::vector<MacNodeId> cellIds;
//    std::vector<MacNodeId> ues;
//    std::vector<Trigger> trigger;
//    std::string consumerUri;
//    std::string appInstanceId;
//    std::string subscriptionType;
//    std::string subscriptionId;
//
//    double expiretaionTime;
//} SubscriptionInfo;



class Location;
class SocketManager_normal;

class LocationService_test_normal: public LocationService
{
  private:
    int numApps_;
    double prob_;
//
//    LocationResource LocationResource_;
//    typedef std::map<std::string, std::map<std::string, SubscriptionInfo >> SubscriptionsStructure;
//    SubscriptionsStructure subscriptions_;
//
//    double LocationSubscriptionPeriod_;
//    cMessage *LocationSubscriptionEvent_;
//
//    unsigned int subscriptionId_;
//    std::string baseUriQueries_;
//    std::string baseUriSubscriptions_;
//    std::set<std::string>supportedQueryParams_;
//    std::set<std::string>supportedSubscriptionParams_;
//
//    bool scheduledSubscription;
//
//    bool manageLocationSubscriptions(Trigger trigger);

  public:
    LocationService_test_normal();
    double getProb(){ return prob_;}
    int getNumApp(){return numApps_;}
    int getQueuedApp() {binomial(3, numApps_, prob_);}
    virtual void removeConnection(SocketManager_normal *connection);

  protected:

    simsignal_t requestQueueSizeSignal_;

    virtual void initialize(int stage) override;
//    virtual int  numInitStages() const override { return inet::NUM_INIT_STAGES; }
//    virtual void finish() override;
//    virtual void refreshDisplay() const override;
    virtual void handleMessage(cMessage *msg) override;
    virtual bool manageRequest() override;
//    virtual void handleGETRequest(const std::string& uri, inet::TCPSocket* socket);
//    virtual void handlePOSTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket);
//    virtual void handlePUTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket);
//    virtual void handleDELETERequest(const std::string& uri, inet::TCPSocket* socket);
//    virtual bool handleSubscriptionType(cMessage *msg);


    virtual ~LocationService_test_normal();


};


#endif // ifndef _LocationService_test_normal_TEST_H

