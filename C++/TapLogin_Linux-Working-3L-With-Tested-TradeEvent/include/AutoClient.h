#ifndef AUTOCLIENT_H
#define AUTOCLIENT_H


#include <map>
#include <arpa/inet.h>
#include "Enums.h"


#include <nn.h>
#include <pubsub.h>
//#include <nanomsg/nn.h>
//#include <nanomsg/pubsub.h>

//#include "INICPP/INIReader.h"
#include <iostream>
#include <stdlib.h>
#include <boost/signals2.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <boost/ptr_container/ptr_map.hpp>

#include <StrategyClass.h>
#include <pthread.h>

//#include <boost/thread.hpp>
using namespace Enums;



class AutoClient
{
    public:
        AutoClient();
        virtual ~AutoClient();
        void InItClass(map<int,ContractDetails> AllCimlist);
         void Dispose();
        long ClientIdAuto;
        long ClientIdAlgo;
void HandleOnFOPairSubscription (strFOPAIR _FOpairObj);
    void HandleOnFOPairUnSubscription (strFOPAIR _FOpairObj);
    void HandleOnFOPairDiff (FOPAIRDIFF _INpairDiff);
     int AutoCountID;

    protected:

    map<int , StrategyClass> _StgClass;

    int Datasock;// = nn_socket(AF_SP, NN_SUB);
    int sock ;
	//map<int, FinalPrice> _Datadict;
	map<int, NFToken> SymbolDictionary;
	map<int,struct FOPAIRDIFF> _FOPAIRDIFF ;

//	map<int, map<char, OrderDetails>> _OrderDetails

    map<int,OrderDetails> _OrderDetailsBuy;
    map<int,OrderDetails> _OrderDetailsSell;

	map<int,  NearMonthPacket> _NMPACK;

    //typedef boost::signals2::signal<void(FinalPrice*)> DataSignal;
    typedef boost::signals2::signal<void (FinalPrice)> DataSignal;

    boost::ptr_map<int, DataSignal> _DataPack;

    pthread_mutex_t count_mutex;
    //map<int , std::mutex> _mutex;
//std::array<std::mutex, 30> _mutex;

//#define CancelCode 20070
//#define ModificationCode 20070
//*************************************************
   // boost::mutex mutex;

    //std::mutex _lock;

    map<int,bool> _SubStatus;
    boost::thread_group producer_threads;


	long   UserId;//Trader
	short  BranchId;
    string SECTION;
    string CONTRACTFILEPATH;
    string DATANANOPATH;
    boost::thread* _dataThread;
    boost::thread* _eventThread;

	int BrokerId;

	MS_OE_REQUEST_TR OEObj;
	MS_OM_REQUEST_TR OMObj;

    //INIReader reader;

    bool IsExit;

    int eventconnectid;
    int dataconnectid;

    bool DataThreadExitted;
    bool EventThreadExitted;

    //map<int,struct ContractDetails> cimlist;
    map<int,ContractDetails> cimlist;
    private:

     int UID;

    void PadRight(char *string, int padded_len, char *pad);
    void toUpper(char* pArray, int arrayLength);
    TokenPartnerDetails  InitTokenDetails( int FirstLeg, int alternateLeg, int PortfolioName);
    MS_OE_REQUEST_TR ReturnNearPack(int Token,BUYSELL BS,int Qty,int FMBlq =0);
    void htond (double &x);
    MS_OM_REQUEST_TR ReturnModificationPack(int Token,BUYSELL BS);

    void Datasubscriber ();
void SubscribeTokenOnFalse(int Token);
void UnSubscribeTokenOnTrue(int Token);
void OnDataArrived(FinalPrice* _fp);


 void SubscribeToken (int _token);
 void UnSubscribeToken (int _token);

 void SubscribePF (SubscriptionTagDets key);
 void SubscribePF (string key);
 void SubscribePF (long _PF);

 void UnSubscribePF (long _PF);

 void SubscribePFOnFalse(short _PF,long Key);
 void UnSubscribePFOnTrue(int _PF, long Key);

 long long concat(long long x, long long y);
 void Eventsubscriber ();
 void CancelAllOrder();
 void StopAllOrder();


 void CancelBuyOrder(int Token);
 void CancelSellOrder(int Token);


};


#endif // AUTOCLIENT_H
