#ifndef __INET_GENERICSERVICE_H
#define __INET_GENERICSERVICE_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"
#include <vector>
#include <map>
#include "inet/applications/tcpapp/TCPSrvHostApp.h"
#include <queue>

#include "corenetwork/nodes/mec/MEPlatform/MeServices/httpUtils/httpUtils.h"
#include "corenetwork/binder/LteBinder.h"
#include "common/MecCommon.h"

/**
 * @author Alessandro Noferi
 *
 * This class implements the general structure of a Mec Service.
 * It holds all the TCP connections with the e.g Mec Applications and
 * manages its lifecycle.
 * * It manages Request-Replay and Subscribe-Notify schemes.
 *
 * Every request is inserted in the queue and executed in FIFO order.
 * Also subscription event are queued in a separate queue and have the priority.
 * The execution times are calculated with the calculateRequestServiceTime method.
 *
 * During initialization it saves all the eNodeB connected to the MeHost in
 * which the service is running.
 *
 * Each connection is managed by the SocketManager object that implements the
 * TCPSocket::CallbackInterface
 *
 * It must be subclassed and only the methods relative to the requests management (e.g handleGETrequest)
 * have to be implemented.
 *
 */


#define REQUEST_RNG 0
#define SUBSCRIPTION_RNG 1

typedef std::map<std::string, std::string> reqMap;
enum RequestState {CORRECT, BAD_REQ_LINE, BAD_HEADER, BAD_HTTP, BAD_REQUEST, DIFF_HOST, UNDEFINED};

class SocketManager;
class SubscriptionBase;

class MeServiceBase: public cSimpleModule, public ILifecycle
{
    public:
        MeServiceBase();

    protected:
        inet::TCPSocket serverSocket; // Used to listen incoming connections
        inet::TCPSocketMap socketMap; // Stores the connections
        std::string host_;
        LteBinder* binder_;
        cModule* meHost_;

        std::string baseUriQueries_;
        std::string baseUriSubscriptions_;
        std::string baseSubscriptionLocation_;

        unsigned int subscriptionId_; // identifier for new subscriptions

        // currently not uses
        std::set<std::string>supportedQueryParams_;
        std::set<std::string>supportedSubscriptionParams_;


        typedef std::map<unsigned int, SubscriptionBase*> Subscriptions;
        Subscriptions subscriptions_; //list of all active subscriptions

        std::vector<cModule*> eNodeB_;     //eNodeBs connected to the ME Host

        /* TODO reimplement message management in a more OMNet++ style
         */
        cMessage *currentRequestServed_;
        reqMap currentRequestServedmap_;
        int requestQueueSize_;
        std::string currentRequestServedPayload_;
        RequestState currentRequestState_;

        cMessage *requestService_;
        double requestServiceTime_;
        cQueue requests_;               // queue that holds incoming requests

        cMessage *subscriptionService_;
        double subscriptionServiceTime_;
        int subscriptionQueueSize_;
        cQueue subscriptionEvents_;          // queue that holds events relative to subscriptions
        cMessage *currentSubscriptionServed_;

        // signals for statistics
        simsignal_t requestQueueSizeSignal_;


        /*
         * This method is called for every request in the requests_ queue.
         * It check if the receiver is still connected and in case manages the request
         */
        virtual bool manageRequest();

        /*
         * This method is called for every element in the subscriptions_ queue.
         */
        virtual bool manageSubscription();

        /*
         * This method checks the queues length and in case it simulates a request/subscription
         * execution time.
         * Subscriptions have precedence wrt requests
         *
         * if parameter is true -> scheduleAt(NOW)
         * This is
         *
         *
         * @param now when to send the next event
         */
        virtual void scheduleNextEvent(bool now = false);

        virtual void initialize(int stage) override;
        virtual int  numInitStages() const override { return inet::NUM_INIT_STAGES; }
        virtual void handleMessage(cMessage *msg) override;
        virtual void finish() override;
        virtual void refreshDisplay() const override;

        /*
         * This method finds all the eNodeB connected to the Mec Host hosting the service
         */
        virtual void getConnectedEnodeB();

        /*
         * This method parses a HTTP request splitting headers from body (if present)
         *
         * @param packet_ tcp packet payload
         * @param socket to send back responses in case of the request is malformed
         * @request pointer to the structure where to save the the parse result
         */
        virtual bool parseRequest(std::string& packet_, inet::TCPSocket *socket, reqMap* request);

        /*
         * This method parses a HTTP request splitting headers from body (if present).
         * the request parsed is cMessage *currentRequestServed_;
         */
        virtual void parseCurrentRequest();

        /*
         * This method call the handle method according to the HTTP verb
         * e.g GET --> handleGetRequest()
         * These methods have to be implemented by the MEC services
         */
        virtual void handleCurrentRequest(inet::TCPSocket *socket);

        /*
         * This method manages the situation of an incoming request when
         * the request queue is full. It responds with a HTTP 503
         */
        virtual void handleRequestQueueFull(cMessage *msg);

        /*
        * This method calculate the service time of the request based on:
        *  - the method (e.g. GET POST)
        *  - the number of parameters
        *  - .ini parameter
        *  - combination of the above
        *
        * In the base class it returns a Poisson service time with mean
        * given by an .ini parameter.
        * The Mec service can implement the calculation as preferred
        *
        */
        virtual double calculateRequestServiceTime();

        /*
         * Abstract methods
         *
         * handleGETRequest
         * handlePOSTRequest
         * handleDELETERequest
         * handlePUTRequest
         *
         * The above methods handle the corresponding HTTP requests
         * @param uri uri of the resource
         * @param socket to send back the response
         *
         * @param body body of the request
         *
         */

        virtual void handleGETRequest(const std::string& uri, inet::TCPSocket* socket) = 0;
        virtual void handlePOSTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket)   = 0;
        virtual void handlePUTRequest(const std::string& uri, const std::string& body, inet::TCPSocket* socket)    = 0;
        virtual void handleDELETERequest(const std::string& uri, inet::TCPSocket* socket) = 0;

        /*
         * This method handles a subscription event. The kind variable of the msg
         * specifies the type (e.g PERIODICAL, UTILIZATION_THRSHOLD_80, DISTANCE
         *
         * @param msg thta describes the request type
         *
         */
        virtual bool handleSubscriptionType(cMessage *msg) = 0;

//        virtual void removeSubscription(inet::TCPSocket* socket) = 0;
        virtual ~MeServiceBase();

        virtual bool handleOperationStage(inet::LifecycleOperation *operation, int stage, inet::IDoneCallback *doneCallback) override
        { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

    public:
        /*
         * This method can be used by a module that want to inform that
         * something happened, like a value greater than a  threshold
         * in order to send a subscription
         *
         * The param is wrong, it must be a structure that also reports the value like
         * structures{
         *  type
         *  value
         *  }
         */
        virtual void triggeredEvent(short int event);

        /*
         * This method adds the request in the requests_ queue
         *
         * @param msg request
         */
        virtual void newRequest(cMessage *msg);

        // This method adds the subscription event in the subscriptions_ queue

        virtual void newSubscriptionEvent(cMessage *msg);

        /*
         * This method handles a request. It parses the payload and in case
         * calls the correct method (e.g GET, POST)
         *
         * @param request HTTP request
         * @param socket used to send back the response
         */
        virtual void handleRequest(cMessage* request, inet::TCPSocket *socket);

        /* This method is used by the SocketManager object in order to remove itself from the
         * map
         *
         * @param connection connection object to be deleted
         */
        virtual void removeConnection(SocketManager *connection);

        virtual Http::DataType getDataType(std::string& packet_);

        /* This method can be used by the socketManager class to emit
         * the length of the request queue upon a request arrival
         */
        virtual void emitRequestQueueLength();

        /* This method removes the subscriptions associated
         * with a closed connection
         */
        virtual void removeSubscritions(int connId);
};


#endif // __INET_GENERICSERVICE_H

