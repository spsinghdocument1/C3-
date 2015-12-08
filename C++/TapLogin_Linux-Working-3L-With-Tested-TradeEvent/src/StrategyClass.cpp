#include "StrategyClass.h"
#include "string.h"

#include "StaticClass.h"
#include <arpa/inet.h>
#include "../PFMap.h"
//#include <boost/thread/mutex.hpp>
//#include<boost/thread/condition_variable.hpp>

#include <pthread.h>

using namespace std;




StrategyClass::StrategyClass()
{
    //ctor
            BuyFarStatus= (OrderStatus)NONE;
            SellFarStatus = (OrderStatus)NONE;

            SellTradeCounter=0;
            BuyTradeCounter=0;

            memset(&FarFP,0,sizeof(FinalPrice));
            memset(&NearFP,0,sizeof(FinalPrice));
            memset(&ThirdFP,0,sizeof(FinalPrice));

            CancelCode=ntohs(20070);
            ModificationCode=ntohs(20040);

            memset(&_pfBuy,0,sizeof(PFHolder));
            memset(&_pfSell,0,sizeof(PFHolder));
            memset(&_shBuy,0,sizeof(StructHash));
            memset(&_shSell,0,sizeof(StructHash));

            memset(&_pfMKTBuy,0,sizeof(PFHolder));
            memset(&_pfMKTSell,0,sizeof(PFHolder));
            memset(&_shMKTBuy,0,sizeof(StructHash));
            memset(&_shMKTSell,0,sizeof(StructHash));

            memset(&_pfMKTBuy2,0,sizeof(PFHolder));
            memset(&_pfMKTSell2,0,sizeof(PFHolder));
            memset(&_shMKTBuy2,0,sizeof(StructHash));
            memset(&_shMKTSell2,0,sizeof(StructHash));

            _BNSFMNQ=0;
            _BFSNMNQ =0;
            _BNSFMXQ =0;
            _BFSNMXQ =0;
            BFSNDIFF =0;
            BNSFDIFF =0;

        count_mutex = PTHREAD_MUTEX_INITIALIZER;
        initbuy_mutex = PTHREAD_MUTEX_INITIALIZER;
        initsell_mutex = PTHREAD_MUTEX_INITIALIZER;

}

StrategyClass::~StrategyClass()
{
    //dtor
}

long long StrategyClass::concat(long long x, long long y)
{
    long long temp = y;
    while (y != 0) {
        x *= 10;
        y /= 10;
    }
    return x + temp;
}

 void StrategyClass::OnSubscriptionEventHandler(strFOPAIR* _FOpairObj)
        {
            FOpairObj = *_FOpairObj;
           // strFOPAIR _FOpairObj;
           // memcpy(&_FOpairObj,buffer,sizeof(_FOpairObj));
            FarFP.Token = _FOpairObj->TokenFar;
            NearFP.Token = _FOpairObj->TokenNear;
            ThirdFP.Token = _FOpairObj->TokenThree;

            PFNumber = _FOpairObj->PORTFOLIONAME;
           // cout << "PFNumber "<< PFNumber <<" New class OnSub PF " << PFNumber << " Far " << FarFP.Token << " Near "<< NearFP.Token<<endl<<endl;

            _shBuy.Token1 = FarFP.Token;
            _shBuy.side1 = BUY;

            _shSell.Token1 = FarFP.Token;
            _shSell.side1 = SELL;

             _pfBuy.PF = PFNumber;
            _pfBuy.CID = ClientID;

             _pfSell.PF = PFNumber;
            _pfSell.CID = ClientID;

            _shMKTBuy.Token1 = NearFP.Token;
            _shMKTBuy.side1 = BUY;

            _shMKTSell.Token1 = NearFP.Token;
            _shMKTSell.side1 = SELL;

             _pfMKTBuy.PF = PFNumber;
            _pfMKTBuy.CID = ClientID;

             _pfMKTSell.PF = PFNumber;
            _pfMKTSell.CID = ClientID;


            _shMKTBuy2.Token1 = ThirdFP.Token;
            _shMKTBuy2.side1 = BUY;

            _shMKTSell2.Token1 = ThirdFP.Token;
            _shMKTSell2.side1 = SELL;

             _pfMKTBuy2.PF = PFNumber;
            _pfMKTBuy2.CID = ClientID;

             _pfMKTSell2.PF = PFNumber;
            _pfMKTSell2.CID = ClientID;


            long lng=concat((short)(MessageType)eORDER,_pfSell.CID);
            lng = concat(lng,(short)_pfSell.PF);

            _pfBuy.SubscriptionTag = lng;
            _pfSell.SubscriptionTag = lng;

            _pfMKTBuy.SubscriptionTag = lng;
            _pfMKTSell.SubscriptionTag = lng;

            _pfMKTBuy2.SubscriptionTag = lng;
            _pfMKTSell2.SubscriptionTag = lng;

            CancelBuyOrder = false;
	 	 	CancelSellOrder= false;

        }

        void StrategyClass::OnUnSubscriptionEventHandler(strFOPAIR* _FOpairObj)
        {
            //strFOPAIR _FOpairObj;

	 	 	// memcpy(&_FOpairObj,buffer,sizeof(_FOpairObj));

            if(BuyFarStatus==OPEN || BuyFarStatus== REPLACED || BuyFarStatus == PENDING)
            {
                BuyFarStatus = PENDING;
                CancelBuyOrder = true;
                cout<<"StrategyClass OnUnSubscriptionEventHandler BUY "<<(BUYSELL)BUY<<" FarFP.Token ="<<FarFP.Token<<endl<<endl;
                ORDER_CANCEL_IN_TR ( FarFP.Token, (BUYSELL)BUY);
            }
            if(SellFarStatus == OPEN || SellFarStatus == REPLACED|| BuyFarStatus == PENDING)
            {
                SellFarStatus = PENDING;
                CancelSellOrder= true;

                cout<<"StrategyClass OnUnSubscriptionEventHandler SELL \t"<<(BUYSELL)SELL<<" FarFP.Token "<<FarFP.Token<<endl<<endl;
                ORDER_CANCEL_IN_TR ( FarFP.Token, (BUYSELL)SELL);

            }


            _BNSFMNQ=0;
            _BFSNMNQ =0;
            _BNSFMXQ =0;
            _BFSNMXQ =0;
            BFSNDIFF =0;
            BNSFDIFF =0;

            BuyFarStatus=NONE;
            SellFarStatus=NONE;

            cout << " Unsubscription recieved in Strategy for CID " << ClientID << " PF "<< PFNumber<<endl;
       // cout << "PFNumber "<< PFNumber <<" New class OnUnSub PF " << PFNumber << " Far " << _FOpairObj->TokenFar << " Near "<< _FOpairObj->TokenNear<<endl<<endl;
        }

    int StrategyClass::GetExpectedProdPrice(  BUYSELL BS, FinalPrice FP, int Ratio,bool reverse = false)
	{

			int RetVal = 0;

			if (!reverse)
			{
				// THis case calculates the price to generate buy spread
				RetVal = BS== (BUYSELL)BUY ? (FP.MINASK*Ratio* -1) : (FP.MAXBID*Ratio);
			}
			else
			{
				// Here in case of sale actual stg with buy mode token will be sold just to make a complete trade
				RetVal = BS == (BUYSELL)BUY ? (FP.MAXBID*Ratio ) : (FP.MINASK*Ratio * -1);
			}
			return RetVal;

	}



    int StrategyClass::GetExpectedBidPrice(BUYSELL BS,FinalPrice FP,int Ratio,bool Create= false)
    {
        int RetVal=0;

        if(Create)
        {
            RetVal = BS == (BUYSELL)BUY ? (FP.MAXBID * Ratio * -1) : (FP.MINASK * Ratio );

        }
        else
        {
            RetVal = BS == (BUYSELL)BUY ? (FP.MINASK * Ratio ) :  (FP.MAXBID * Ratio * -1 );

        }

    }


        void StrategyClass::OnDataEventHandler(FinalPrice _fp)
        {

          //  boost::mutex::scoped_lock lock(the_inmutex);
          //  cout << " New class OnDataArrived PF "<< PFNumber <<" Token "<< _fp.Token << " Bid "<< _fp.MAXBID << " Ask "<< _fp.MINASK<<endl<<endl;
          //  cout << " FarFP Token "<< FarFP.Token << " NearFP Token "<< NearFP.Token<<endl<<endl;
            if(FarFP.Token==0 && NearFP.Token==0)
            return;

             pthread_mutex_trylock(&count_mutex);

            if(FarFP.Token== _fp.Token)
            {
                FarFP = _fp;
            }
            else if(NearFP.Token == _fp.Token)
            {
                NearFP = _fp;
            }
            else if(ThirdFP.Token == _fp.Token)
            {
                ThirdFP = _fp;
            }
           // cout <<" FarFP.MINASK "<<FarFP.MINASK << " FarFP.MAXBID "<<FarFP.MAXBID << " NearFP.MAXBID " << NearFP.MAXBID << " NearFP.MINASK " <<NearFP.MINASK <<endl<<endl;

            if(FarFP.MINASK > 0 && FarFP.MAXBID >0 && NearFP.MAXBID >0 && NearFP.MINASK >0)
            {
                if(ThirdFP.Token >0 && ThirdFP.MINASK <= 0 && ThirdFP.MAXBID<=0)
                return;

                int secondbuyProd = GetExpectedProdPrice((BUYSELL)FOpairObj.TokenNearSide, NearFP, FOpairObj.TokenNearRatio); // Market rate for Leg2

                int firstbuyProd =  GetExpectedBidPrice((BUYSELL)FOpairObj.TokenFarSide, FarFP, FOpairObj.TokenFarRatio,true); //Best rate for Leg1


                int thirdbuyprod =ThirdFP.Token>0?GetExpectedProdPrice((BUYSELL)FOpairObj.TokenThreeSide, ThirdFP, FOpairObj.TokenThreeRatio):0;
    //int secondbuyProd = GetExpectedBidPrice((BUYSELL)_FOPair.Token2Side, FarFP, _FOPair.Token2Ratio,true);

   // int thirdbuyProd = ThirdToken > 0 ? GetExpectedProdPrice((BUYSELL)_NFToken._OptTokens.Token3side, ThirdFP, _NFToken._OptTokenDets.Token3Ratio)  : 0;
   // int fourthbuyProd = FourthToken > 0 ? GetExpectedProdPrice((BUYSELL)_NFToken._OptTokens.Token4side, FourthFP, _NFToken._OptTokenDets.Token4Ratio)  : 0;



            //cout << "firstbuyProd "<< firstbuyProd << " secondbuyProd "<<secondbuyProd << " thirdbuyprod "<<thirdbuyprod<<endl;



    //int firstsellProd = GetExpectedProdPrice((BUYSELL)_FOPair.Token1Side, NearFP, _FOPair.Token1Ratio,true);
    int secondsellProd = GetExpectedProdPrice((BUYSELL)FOpairObj.TokenNearSide,NearFP , FOpairObj.TokenNearRatio,true); //Market rate for leg2

    int firstsellProd = GetExpectedBidPrice((BUYSELL)FOpairObj.TokenFarSide, FarFP, FOpairObj.TokenFarRatio,false); // Best Rate for Leg1

    int thirdsellprod =ThirdFP.Token>0?GetExpectedProdPrice((BUYSELL)FOpairObj.TokenThreeSide,ThirdFP , FOpairObj.TokenThreeRatio,true):0;


 //cout << "firstsellProd "<< firstsellProd << " secondsellProd "<<secondsellProd << " thirdsellprod "<<thirdsellprod<<endl;


    int dAsksDifference = firstsellProd+secondsellProd +thirdsellprod;// + thirdsellProd + fourthsellProd;
    int dBidsDifference = firstbuyProd + secondbuyProd +thirdbuyprod;// + fourthbuyProd;


          //  cout <<"PFNumber "<< PFNumber <<" dAsksDifference " << dAsksDifference<< " dBidsDifference "<< dBidsDifference <<endl; //" SellFarStatus " << SellFarStatus << " SellTradeCounter "<<SellTradeCounter<<endl<<endl;

            CreateSide = FOpairObj.TokenFarSide;
            ReverseSide = CreateSide == BUY ? SELL:BUY;

if(1==1)
{
            if((SellFarStatus==(OrderStatus)TRADE || SellFarStatus==(OrderStatus)CANCEL || SellFarStatus==(OrderStatus)REJECTED || SellFarStatus==(OrderStatus)NONE) &&
               BNSFDIFF !=0  && _BNSFMNQ > 0 && _BNSFMXQ > 0 && SellTradeCounter < _BNSFMXQ  && abs(BNSFDIFF) <= abs(dAsksDifference) && !CancelSellOrder )
            {
                int dFarMonthSellRate = 0;

                 if (abs(BNSFDIFF) < abs(dAsksDifference))
                {
                      dFarMonthSellRate = (FarFP.MINASK) - 5;

                }
                else if (abs(BNSFDIFF) == abs(dAsksDifference ))
                {
                      dFarMonthSellRate = (NearFP.MINASK) + (BNSFDIFF);
                }
               // cout << " dFarMonthSellRate " << dFarMonthSellRate << " FarFP.MAXBID "  <<FarFP.MAXBID<<endl<<endl;
                if(dFarMonthSellRate > FarFP.MAXBID)
                {

                    pthread_mutex_lock(&initsell_mutex);


                   SellFarStatus = (OrderStatus)PENDING;
                   //_FillData.producer(new FinalPrice());
                   // _FillData.producer((char*)new FinalPrice());


                    _shSell.Qty1= _BNSFMNQ * BLQ;
                    _shSell.Price1=dFarMonthSellRate;
                    _shSell.side1=ReverseSide;
                    _pfSell.HashKey = MyHash<StructHash>()(_shSell);
                    _pfSell.side =(short)ReverseSide;
                    BOARD_LOT_IN_TRSell (FarFP.Token,_pfSell.side,
                                	ntohl(_shSell.Qty1),
                                    ntohl(dFarMonthSellRate),_pfSell);

                   // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA PFnumber " <<  PFNumber << " Sell bid @ "<< dFarMonthSellRate<<  " Token "<< _shSell.Token1<< " Side "<< _shSell.side1
                   // << " Qty "<< _shSell.Qty1 << " Price "<< _shSell.Price1 <<  " Hashkey "<< _pfSell.HashKey<<" AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"<< endl<<endl;
                    pthread_mutex_unlock(&initsell_mutex);



                }
                else
                {
                    cout <<"PFNumber \t"<< PFNumber << "\t BNSF \t"<< "New Order Sell price calculated less than Bid"<<endl<<endl;
                }

            }

            else if((SellFarStatus == (OrderStatus)OPEN || SellFarStatus == (OrderStatus)REPLACED )&& abs(dAsksDifference) >= abs(BNSFDIFF) && !CancelSellOrder)
			{
                int dQuoteRate;
                if (abs(dAsksDifference) > abs(BNSFDIFF) )
                {
                    dQuoteRate= (FarFP.MINASK) - 5;

                }//Price Greater Than zero check
                else if (abs(dAsksDifference) == abs(BNSFDIFF))
                {
                    dQuoteRate = (NearFP.MINASK) + BNSFDIFF;
                }

                if(dQuoteRate > FarFP.MAXBID)
                {
                    SellFarStatus = (OrderStatus)PENDING;
                    ORDER_MOD_IN_TR (FarFP.Token,
									          //      ntohl( OrdDetailSell.Qty *BLQ),
									 ntohl(dQuoteRate),ReverseSide
									);
                   // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA Modified Sell PF "<< PFNumber << " AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"<<endl<<endl;
                }
                else
                {
                    cout <<"PFNumber \t"<< PFNumber << "\t BNSF \t"<< "Modify Order Sell price calculated less than Bid"<<endl<<endl;
                }
            }
            else if (((SellFarStatus == (OrderStatus)OPEN || SellFarStatus == (OrderStatus)REPLACED) && abs(dAsksDifference) < abs(BNSFDIFF)) ||
            (CancelSellOrder && (SellFarStatus == (OrderStatus)OPEN || SellFarStatus == (OrderStatus)REPLACED) ))
            {
                SellFarStatus = (OrderStatus)PENDING;
                ORDER_CANCEL_IN_TR ( FarFP.Token, ReverseSide);
              //  cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA Cancelled Sell PF "<< PFNumber << " AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"<<endl<<endl;
            }
        }

         //   cout <<"PFNumber"<< PFNumber << " dAsksDifference " << dAsksDifference<< " dBidsDifference "<< dBidsDifference << " BuyFarStatus " << BuyFarStatus << " BuyTradeCounter "<<BuyTradeCounter<<endl<<endl;
if(1==1)
{
             if((BuyFarStatus==(OrderStatus)TRADE || BuyFarStatus==(OrderStatus)CANCEL || BuyFarStatus==(OrderStatus)REJECTED || BuyFarStatus==(OrderStatus)NONE) &&
               BFSNDIFF !=0  && _BFSNMNQ > 0 && _BFSNMXQ > 0 && BuyTradeCounter < _BFSNMXQ  && abs(BFSNDIFF) >= abs(dBidsDifference)  && !CancelBuyOrder)
            {
                int dFarMonthBuyRate = 0;

                if (abs(BFSNDIFF) > abs(dBidsDifference))
                {
                    dFarMonthBuyRate = (FarFP.MAXBID) + 5;
                }
                else if (abs(BFSNDIFF) == abs(dBidsDifference ))
                {
                    dFarMonthBuyRate = (NearFP.MAXBID) + (BFSNDIFF);
                }

                if(dFarMonthBuyRate < FarFP.MINASK  && dFarMonthBuyRate > 0)
                {

                    pthread_mutex_lock(&initbuy_mutex);
                    BuyFarStatus = (OrderStatus)PENDING;

                    _shBuy.Qty1= _BFSNMNQ * BLQ;
                    _shBuy.side1=CreateSide;
                    _shBuy.Price1=dFarMonthBuyRate;
                    _pfBuy.HashKey = MyHash<StructHash>()(_shBuy);
                    _pfBuy.side = (short)CreateSide ;

                    BOARD_LOT_IN_TRBuy (FarFP.Token,_pfBuy.side,
                                    ntohl( _shBuy.Qty1),
                                    ntohl(dFarMonthBuyRate),_pfBuy
                                );
                   // cout << " QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ PFnumber " <<  PFNumber << " buy bid @ "<< dFarMonthBuyRate<<  " Token "<< _shBuy.Token1<< " Side "<< _shBuy.side1
                  //  << " Qty "<< _shBuy.Qty1 << " Price "<< _shBuy.Price1 <<  " Hashkey "<< _pfBuy.HashKey<<" QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"<< endl<<endl;
                    pthread_mutex_unlock(&initbuy_mutex);


                }
                else
                {
                    cout <<"PFNumber \t"<< PFNumber << "\t BFSN \t"<< "New Order Buy price calculated greater than Ask"<<endl<<endl;
                }

            }

            else if((BuyFarStatus == (OrderStatus)OPEN || BuyFarStatus == (OrderStatus)REPLACED )&& abs(dBidsDifference) <= abs(BFSNDIFF) && !CancelBuyOrder)
			{
                int dQuoteRate;
                if (abs(dBidsDifference) < abs(BFSNDIFF ))
                {
                    dQuoteRate= (FarFP.MAXBID) + 5;

                }//Price Greater Than zero check
                else if (abs(dBidsDifference) == abs(BFSNDIFF))
                {
                    dQuoteRate = (NearFP.MAXBID) + BFSNDIFF;
                }

                if(dQuoteRate < FarFP.MINASK && dQuoteRate > 0)
                {
                    BuyFarStatus = (OrderStatus)PENDING;
                    ORDER_MOD_IN_TR (FarFP.Token,
						         //       ntohl(OrdDetailBuy.Qty *BLQ),
                                    ntohl(dQuoteRate),CreateSide
						);
					//	cout << "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ Modified Buy PF "<< PFNumber <<" QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"<< endl<<endl;
                }
                else
                {
                    cout <<"PFNumber \t"<< PFNumber << "\t BFSN \t"<< "Modify Order Buy price calculated greater than Ask"<<endl<<endl;
                }
            }
            else if ((BuyFarStatus == (OrderStatus)OPEN || BuyFarStatus == (OrderStatus)REPLACED) && abs(dBidsDifference) > abs(BFSNDIFF)
                    || ((BuyFarStatus == (OrderStatus)OPEN || BuyFarStatus == (OrderStatus)REPLACED) &&CancelBuyOrder))
            {
                BuyFarStatus = (OrderStatus)PENDING;
                ORDER_CANCEL_IN_TR ( FarFP.Token, CreateSide);
              // cout << "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQq Cancelled Buy PF "<< PFNumber << " QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQq"<<endl<<endl;
            }
            }

        }
            pthread_mutex_unlock(&count_mutex);
       // lock.unlock();
      //  the_incondition_variable.notify_one();

        }



        void StrategyClass::OnDifferenceEventHandler(FOPAIRDIFF* _INpairDiff)
        {
			//struct FOPAIRDIFF _INpairDiff;

           // memcpy(&_INpairDiff,buffer,sizeof(_INpairDiff));

            	_BNSFMNQ = _INpairDiff->BNSFMNQ;
                _BFSNMNQ = _INpairDiff->BFSNMNQ;
                _BNSFMXQ = _INpairDiff->BNSFMXQ;
                _BFSNMXQ = _INpairDiff->BFSNMXQ;
            //	DEPTHORBEST = _FOPAIRDIFF [PFNumber].Depth_Best ==Depth ? true  :false ;

                BFSNDIFF = (int)_INpairDiff->BFSNDIFF;
                BNSFDIFF = (int)_INpairDiff->BNSFDIFF;

               // cout << " New class OnDiff PF " << PFNumber << " BNSFDIFF " << _INpairDiff->BNSFDIFF << " BFSN "<< _INpairDiff->BFSNDIFF<<endl<<endl;
        }


void StrategyClass::BOARD_LOT_IN_TRBuy(int FarTokenNo,BUYSELL BS,int qty, int price, PFHolder _PF)  //-- 20000
		{// MS_OE_REQUEST_TR 110+26

          //  cout<<"Buy: "<<FarTokenNo<<" price "<<htonl(price)<<" size "<<sizeof (_NMPACK [FarTokenNo].FARMONTHBUY) <<endl;

                    //cout<<"BOARD_LOT_IN_TRBuy  : 1 ";
                    MS_OE_REQUEST_TR obj;
                   // cout<<"BOARD_LOT_IN_TRBuy  : 2 ";
                    memset(&obj,0,138);
                    //cout<<"BOARD_LOT_IN_TRBuy  : 3 ";
                    obj=_NMPack.FARMONTH;
                    obj.Buy_SellIndicator =ntohs((short)BS);

                   // cout<<"BOARD_LOT_IN_TRBuy  : 4 ";
                    obj.DisclosedVolume=obj.Volume=qty;
	 				obj.Price=price;
	 				//cout<<"BOARD_LOT_IN_TRBuy  : 5 ";
	 				//ProcessToEnqueue((char*)&obj,136);
	 				//cout<<"BOARD_LOT_IN_TRBuy  : 6 ";

	 				_PF._oetr = obj;
                    _PF._size = 136;

                    _PFHashMap.AddRecord(_PF,false);

                   _FillData.BidQueue(_PF);

                   cout <<"PFNumber "<< PFNumber <<" New Buy Order sent from STG "<<  " Symbol "<< obj.Symbol<< " Instrument Name "<< obj.InstrumentName <<endl;

        }


 void StrategyClass::BOARD_LOT_IN_TRSell(int FarTokenNo,BUYSELL BS,int qty, int price, PFHolder _PF)  //-- 20000
		{// MS_OE_REQUEST_TR 110+26

//cout<<"Sell: "<<FarTokenNo<<" price "<<htonl(price)<<" size "<<sizeof (_NMPACK [FarTokenNo].FARMONTHSELL) <<endl;

                   // cout<<"BOARD_LOT_IN_TRSell  : 1 ";
                    MS_OE_REQUEST_TR obj;
                  // cout<<"BOARD_LOT_IN_TRSell  : 2 ";
                    memset(&obj,0,136);
                  // cout<<"BOARD_LOT_IN_TRSell  : 3 ";
                    obj=_NMPack.FARMONTH;
                    obj.Buy_SellIndicator =ntohs((short)BS);
	 				obj.DisclosedVolume=obj.Volume=qty;
	 				//cout<<"BOARD_LOT_IN_TRSell  : 4 ";
	 				obj.Price=price;
	 				//cout<<"BOARD_LOT_IN_TRSell  : 5 ";
	 				//ProcessToEnqueue((char*)&obj,136);
	 				//cout<<"BOARD_LOT_IN_TRSell  : 6 ";
                    _PF._oetr = obj;
                    _PF._size=136;

                     //cout<<"\nLength= "<<ntohl(obj.Length) <<" tcode "<<ntohl(obj.TransactionCode);

                    _PFHashMap.AddRecord(_PF,false);

	 				_FillData.BidQueue(_PF);

                    cout << "PFNumber "<< PFNumber <<" New Sell Order sent from STG "<<  " Symbol "<< obj.Symbol<< " Instrument Name "<< obj.InstrumentName <<endl;
		}


 void StrategyClass::ORDER_MOD_IN_TR(int TokenNo, int price,BUYSELL buySell)// 20040
		{//MS_OM_REQUEST_TR 138+26=164

                    //cout<<"ORDER_MOD_IN_TR  : 1 ";
                    MS_OM_REQUEST_TR obj;
                  //  cout<<"ORDER_MOD_IN_TR  : 2 ";
                    memset(&obj,0,164);
                  //  cout<<"ORDER_MOD_IN_TR  : 3 ";
  	 				switch ((short)buySell)
  	 				{
	 				case 1:
	 					obj=_NMPack.FARMONTHMODBUY;
	 				//	cout<<"ORDER_MOD_IN_TR  : 4 ";
                        //cout << " Mod Buy Order sent from STG "<<endl;
	 					break;
	 				case 2:
	 					obj=_NMPack.FARMONTHMODSELL;
	 					//cout << " Mod Sell Order sent from STG "<<endl;
                      //  cout<<"ORDER_MOD_IN_TR  : 5 ";
	 					break;
	 				}
	 				char TransCodeBytes[0];
                    //cout<<"ORDER_MOD_IN_TR  : 6 ";
	 				obj.TransactionCode=ModificationCode;
	 			//	obj.DisclosedVolume=obj.Volume=volume;
	 				//cout<<"ORDER_MOD_IN_TR  : 7 ";
	 				obj.Price=price;
	 				//cout<<"ORDER_MOD_IN_TR  : 8 ";

                   // ProcessToEnqueue((char*)&obj,164);
                  //  cout<<"ORDER_MOD_IN_TR  : 9 ";
                    _pfBuy._size = 164;
                    _pfBuy._omtr= obj;
                  _FillData.BidQueue(_pfBuy);

		}


 void StrategyClass::ORDER_CANCEL_IN_TR(int TokenNo, BUYSELL buySell)  //-- 20070
		{// MS_OM_REQUEST_TR 138+26

        //cout<<"ORDER_CANCEL_IN_TR  : 1 ";
		MS_OM_REQUEST_TR obj;
		//cout<<"ORDER_CANCEL_IN_TR  : 2 ";
        memset(&obj,0,164);
        //cout<<"ORDER_CANCEL_IN_TR  : 3 ";
  	 				switch ((short)buySell) {
	 				case 1:
	 					obj=_NMPack.FARMONTHMODBUY;
                    cout << " Cancel Buy Order sent from STG 4"<<endl;
	 				//	cout<<"ORDER_CANCEL_IN_TR  : 4 "<<" OrderNumber ="<<_NMPack.FARMONTHMODBUY.OrderNumber<<endl;
	 					break;
	 				case 2:
	 					obj=_NMPack.FARMONTHMODSELL;
	 					cout << " Cancel Sell Order sent from STG  5"<<endl;
                     //   cout<<"ORDER_CANCEL_IN_TR  : 5 "<<" OrderNumber ="<<_NMPack.FARMONTHMODBUY.OrderNumber<<endl;
	 					break;
	 				}
	 			//	cout<<"ORDER_CANCEL_IN_TR  : 6 ";
	 				char TransCodeBytes[0];
	 				obj.TransactionCode=CancelCode;
	 				//cout<<"ORDER_CANCEL_IN_TR  : 7 ";
	 				if(obj.OrderNumber==0 || obj.OrderNumber== -1)
	 				{
                        cout << " OrderNumber invalid. Cancellation not sent " << endl;
                        return;
	 				}
                   // ProcessToEnqueue((char*)&obj,164);
                   // cout<<"ORDER_CANCEL_IN_TR  : 8\n ";

                   _pfBuy._size = 164;
                    _pfBuy._omtr= obj;
                  _FillData.BidQueue(_pfBuy);
		}


 void StrategyClass::ORDER_CONFIRMATION_TR (char *buffer) //-- 20073
		{

			 MS_OE_RESPONSE_TR obj;//156
			 cout<<"ORDER_CONFIRMATION_TR  : 1 ";
            memset(&obj,0,156);
            cout<<"ORDER_CONFIRMATION_TR  : 2 ";
            memcpy(&obj,buffer,156);

       cout<<"ORDER_CONFIRMATION_TR  : 3 ";
		short _BS = htons(obj.Buy_SellIndicator);
		int _TKN =htonl(obj.TokenNo);

       // cout << "Token " << _TKN << " _OrderNo " << _OrderNo << " ORDER_CONFIRMATION_TR In" << endl;
        if(_TKN== FarFP.Token)
        {
		switch (_BS)
		{
		case  (BUYSELL)BUY:
			{
			cout<<"ORDER_CONFIRMATION_TR  : 4 ";
            MS_OM_REQUEST_TR _obj;
            memset(&_obj,0,164);
            _obj=_NMPack.FARMONTHMODBUY;
            _obj.OrderNumber=obj.OrderNumber;
            _obj.DisclosedVolume =obj.DisclosedVolume ;
            _obj.DisclosedVolumeRemaining =obj.DisclosedVolumeRemaining ;
            _obj.TotalVolumeRemaining =obj.TotalVolumeRemaining ;
            _obj.Volume =obj.Volume ;
            _obj.VolumeFilledToday =obj.VolumeFilledToday ;
            _obj.Price =obj.Price ;
            _obj.EntryDateTime =obj.EntryDateTime ;
            _obj.LastModified =obj.LastModified ;
            _obj.filler =obj.filler ;

//     _NMPACK [_TKN].FARMONTHMODBUY=_obj;

            _NMPack.FARMONTHMODBUY=_obj;
            BuyFarStatus = (OrderStatus)OPEN;

             if(CancelBuyOrder)
            {
               ORDER_CANCEL_IN_TR ( FarFP.Token, (BUYSELL)BUY);
            }

//cout<<"ORDER_CONFIRMATION_TR  : 8 ";


     //   _OrderDetailsBuy[_TKN].orderstat = (OrderStatus)OPEN;
    // 	_OrderDetailsBuy[_TKN].OrderNumber = _OrderNo;
     //   _OrderDetailsBuy[_TKN].Price = _price;

     	//	cout << "Token " << _TKN << "  obj.OrderNumber " <<  obj.OrderNumber<< " ORDER_CONFIRMATION_TR In" << endl;
     	//	cout << "Token " << _TKN << "  _obj.OrderNumber " <<  _obj.OrderNumber << " ORDER_CONFIRMATION_TR In" << endl;
     	//	cout << "Token " << _TKN << " _NMPACK [_TKN].FARMONTHMODBUY " <<  _NMPACK [_TKN].FARMONTHMODBUY.OrderNumber << " ORDER_CONFIRMATION_TR In" << endl;
     		//cout<<"ORDER_CONFIRMATION_TR  : 9 ";
     		//cout << " Mod Buy Order recieved in STG "<<endl;
				break;
			}
		case  (BUYSELL)SELL:

			{
			cout<<"ORDER_CONFIRMATION_TR  : 5 ";
			    MS_OM_REQUEST_TR _obj;
            memset(&_obj,0,164);
            _obj=_NMPack.FARMONTHMODSELL;
            _obj.OrderNumber=obj.OrderNumber;
            _obj.DisclosedVolume =obj.DisclosedVolume ;
             _obj.DisclosedVolumeRemaining =obj.DisclosedVolumeRemaining ;
              _obj.TotalVolumeRemaining =obj.TotalVolumeRemaining ;
               _obj.Volume =obj.Volume ;
                _obj.VolumeFilledToday =obj.VolumeFilledToday ;
                 _obj.Price =obj.Price ;
                  _obj.EntryDateTime =obj.EntryDateTime ;
                   _obj.LastModified =obj.LastModified ;
                    _obj.filler =obj.filler ;

//cout<<"ORDER_CONFIRMATION_TR  : 11 ";


            _NMPack.FARMONTHMODSELL=_obj;
            SellFarStatus = (OrderStatus)OPEN;

             if(CancelSellOrder)
                    {
                        ORDER_CANCEL_IN_TR ( FarFP.Token, (BUYSELL)SELL);
                    }

             //   cout << " Mod Sell Order recieved in STG "<<endl;
//cout<<"ORDER_CONFIRMATION_TR  : 12 ";

     	break;
			}
		}
		}
        cout <<"ORDER_CONFIRMATION_TR 6  "<<endl;
     //   ORDER_MOD_IN_TR(_TKN, 50,ntohl( _price+100),(BUYSELL)_BS);
	}

		 void StrategyClass::ORDER_CXL_CONFIRMATION_TR (char *buffer) //-- 20075
	{

			 MS_OE_RESPONSE_TR obj;//156
			//  cout<<"ORDER_CONFIRMATION_TR  : 1 ";
			 memset(&obj,0,156);
			 memcpy(&obj,buffer,156);
			//  cout<<"ORDER_CONFIRMATION_TR  : 2 ";
        short _BS = htons(obj.Buy_SellIndicator);

		// cout<<"ORDER_CONFIRMATION_TR  : 3 ";
        switch(_BS)
		{
            case (BUYSELL)BUY:
            {
                //_OrderDetailsBuy[_TKN].orderstat = (OrderStatus)CANCEL;
                //_OrderDetailsBuy[_TKN].TotalTraded -=1 ;


               BuyFarStatus = (OrderStatus)CANCEL;

            cout << " ORDER_CXL_CONFIRMATION_TR Cancel Buy Order recieved in STG "<<endl;
                   /* map<int,OrderDetails>::iterator iterase = _OrderDetailsBuy.find(_TKN);
                    if(iterase!=_OrderDetailsBuy.end())
                    {
                        _OrderDetailsBuy.erase(iterase);
                    }*/
                    //cout<<"ORDER_CONFIRMATION_TR  : 4 ";

              }
                break;

            case (BUYSELL)SELL:


            SellFarStatus = (OrderStatus)CANCEL;

                cout << "ORDER_CXL_CONFIRMATION_TR Cancel Sell Order recieved in STG "<<endl;
                    //cout<<"ORDER_CONFIRMATION_TR  : 5 ";
                    /*{
                        map<int,OrderDetails>::iterator iterase = _OrderDetailsSell.find(_TKN);
                        if(iterase!=_OrderDetailsSell.end())
                        {
                            _OrderDetailsSell.erase(iterase);
                        }
                    }*/
                break;

		}



	}
//***********************

		 void StrategyClass::ORDER_MOD_CONFIRMATION_TR (char *buffer) //-- 20074
		{
//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step1"<<endl;

       MS_OE_RESPONSE_TR obj;//156
//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step2"<<endl;
	   memset(&obj,0,156);
	//   cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step3"<<endl;
		memcpy(&obj,buffer,156);
		//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step4"<<endl;
		short _BS = htons(obj.Buy_SellIndicator);
		//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step5"<<endl;
		int _TKN = htonl(obj.TokenNo);
		//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step6"<<endl;
		double _OrderNo =obj.OrderNumber ;
		//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step7"<<endl;
		int _price = htonl(obj.Price);
		//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step8"<<endl;

		switch (_BS) {
		case  (BUYSELL)BUY:
			{
			//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step9"<<endl;
            MS_OM_REQUEST_TR _obj;
			//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step10"<<endl;
            memset(&_obj,0,164);
			//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step11"<<endl;
            _obj=_NMPack.FARMONTHMODBUY;
			//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step12"<<endl;
            _obj.OrderNumber=obj.OrderNumber;
			//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step13"<<endl;
            _obj.DisclosedVolume =obj.DisclosedVolume ;
			// cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step14"<<endl;
             _obj.DisclosedVolumeRemaining =obj.DisclosedVolumeRemaining ;
			// cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step15"<<endl;
              _obj.TotalVolumeRemaining =obj.TotalVolumeRemaining ;
			  //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step16"<<endl;
               _obj.Volume =obj.Volume ;
			   //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step17"<<endl;
                _obj.VolumeFilledToday =obj.VolumeFilledToday ;
				//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step18"<<endl;
                 _obj.Price =obj.Price ;
				 //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step19"<<endl;
                  _obj.EntryDateTime =obj.EntryDateTime ;
				  //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step20"<<endl;
                   _obj.LastModified =obj.LastModified ;
				   //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step21"<<endl;
                    _obj.filler =obj.filler ;
					//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step22"<<endl;

            //_NMPACK [_TKN].FARMONTHMODBUY=_obj;

            _NMPack.FARMONTHMODBUY=_obj;
            BuyFarStatus = (OrderStatus)REPLACED;

            if(CancelBuyOrder)
            {
               ORDER_CANCEL_IN_TR ( FarFP.Token, (BUYSELL)BUY);
            }

//cout << " ORDER_MOD_CONFIRMATION_TR Buy Order recieved in STG "<<endl;
			//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step25"<<endl;
            break;
			}
		case  (BUYSELL)SELL:
			{
			    MS_OM_REQUEST_TR _obj;
			//	cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step26"<<endl;
            memset(&_obj,0,164);
			//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step27"<<endl;
            _obj=_NMPack.FARMONTHMODSELL;
			//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step28"<<endl;
            _obj.OrderNumber=obj.OrderNumber;
			//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step29"<<endl;
            _obj.DisclosedVolume =obj.DisclosedVolume ;
			//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step30"<<endl;
             _obj.DisclosedVolumeRemaining =obj.DisclosedVolumeRemaining ;
			 //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step31"<<endl;
              _obj.TotalVolumeRemaining =obj.TotalVolumeRemaining ;
			  //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step32"<<endl;
               _obj.Volume =obj.Volume ;
			   //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step33"<<endl;
                _obj.VolumeFilledToday =obj.VolumeFilledToday ;
				//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step34"<<endl;
                 _obj.Price =obj.Price ;
				 //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step35"<<endl;
                  _obj.EntryDateTime =obj.EntryDateTime ;
				  //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step36"<<endl;
                   _obj.LastModified =obj.LastModified ;
				   //cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step37"<<endl;
                    _obj.filler =obj.filler ;
					//cout<<"ORDER_MOD_CONFIRMATION_TR =>"<<"step38"<<endl;


				 _NMPack.FARMONTHMODSELL=_obj;
                  SellFarStatus = (OrderStatus)REPLACED;


                    if(CancelSellOrder)
                    {
                        ORDER_CANCEL_IN_TR ( FarFP.Token, (BUYSELL)SELL);
                    }

//cout << " ORDER_MOD_CONFIRMATION_TR Sell Order recieved in STG "<<endl;
                break;
			}
		}


	//	ORDER_CANCEL_IN_TR(_TKN,(BUYSELL)_BS);

	}


		void StrategyClass::TRADE_CONFIRMATION_TR (char *buffer) //-- 20222
		{
		cout<<"TRADE_CONFIRMATION_TR =>"<<"step1"<<endl;
          MS_TRADE_CONFIRM_TR obj_Trade;
		//  cout<<"TRADE_CONFIRMATION_TR =>"<<"step2"<<endl;
          memset(&obj_Trade,0,153);
		//  cout<<"TRADE_CONFIRMATION_TR =>"<<"step3"<<endl;
          memcpy(&obj_Trade,buffer,153);
		//  cout<<"TRADE_CONFIRMATION_TR =>"<<"step4"<<endl;
				int _TKN =htonl(obj_Trade.Token);
		//		cout<<"TRADE_CONFIRMATION_TR =>"<<"step5"<<endl;
				short _BS = htons(obj_Trade.Buy_SellIndicator);
				int Qty = htons(obj_Trade.FillQuantity)/BLQ;
		//		cout<<"TRADE_CONFIRMATION_TR =>"<<"step6"<<endl;
          if(FarFP.Token ==_TKN)
          {
		  //cout<<"TRADE_CONFIRMATION_TR =>"<<"step7"<<endl;
                _TradeCreateSide = FOpairObj.TokenFarSide;
                _TradeReverseSide = _TradeCreateSide == BUY ? SELL : BUY;
                 cout<<"Trade Far  _TKN: "<<_TKN<<" _BS: "<<_BS<<endl;

			//cout<<"TRADE_CONFIRMATION_TR =>"<<"step8"<<endl;
				if(_BS==(short)_TradeCreateSide)
				{
				///Leg2 Market Code starts //////
                    MS_OE_REQUEST_TR obj_New;
				//	cout<<"TRADE_CONFIRMATION_TR =>"<<"step10"<<endl;
                    memset(&obj_New,0,138);
				//	cout<<"TRADE_CONFIRMATION_TR =>"<<"step11"<<endl;

                    obj_New = _NMPack.NEARMONTHMARKET;
                    obj_New.Buy_SellIndicator =ntohs((short)FOpairObj.TokenNearSide);
                    obj_New.DisclosedVolume=obj_New.Volume=obj_Trade.FillQuantity;

                    _shMKTSell.Qty1= _BFSNMNQ * BLQ;
                    _shMKTSell.Price1=0;
                    _shMKTSell.side1 = (short)FOpairObj.TokenNearSide;

                    _pfMKTSell.HashKey = MyHash<StructHash>()(_shMKTSell);
                    _pfMKTSell.side =(short)FOpairObj.TokenNearSide;
                    _pfMKTSell._size = 136;
                    _pfMKTSell._oetr= obj_New;

                    _PFHashMap.AddRecord(_pfMKTSell,false);

                    _FillData.MktQueue(_pfMKTSell);

                ///Leg2 Market Code ends //////

                ///Leg3 Market Code starts //////
                    if(ThirdFP.Token>0)
                    {
                    MS_OE_REQUEST_TR obj_New2;
				//	cout<<"TRADE_CONFIRMATION_TR =>"<<"step10"<<endl;
                    memset(&obj_New2,0,138);
				//	cout<<"TRADE_CONFIRMATION_TR =>"<<"step11"<<endl;

                    obj_New2 = _NMPack.NEARMONTHMARKET2;
                    obj_New2.Buy_SellIndicator =ntohs((short)FOpairObj.TokenThreeSide);
                    obj_New2.DisclosedVolume=obj_New2.Volume=obj_Trade.FillQuantity;

                    _shMKTSell2.Qty1= _BFSNMNQ * BLQ;
                    _shMKTSell2.Price1=0;
                    _shMKTSell2.side1 = (short)FOpairObj.TokenThreeSide;

                    _pfMKTSell2.HashKey = MyHash<StructHash>()(_shMKTSell2);
                    _pfMKTSell2.side =(short)FOpairObj.TokenThreeSide;
                    _pfMKTSell2._size = 136;
                    _pfMKTSell2._oetr= obj_New2;

                    _PFHashMap.AddRecord(_pfMKTSell2,false);

                    _FillData.MktQueue(_pfMKTSell2);
                    }
                ///Leg3 Market Code ends //////

				//	cout<<"TRADE_CONFIRMATION_TR =>"<<"step12"<<endl;
					//cout << "Near Month Pack placed" << endl<< endl;
	 				//rocessToEnqueue((char*)&obj_New,136);
				//	cout<<"TRADE_CONFIRMATION_TR =>"<<"step13"<<endl;
                    BuyTradeCounter+=1;
                    BuyFarStatus = (OrderStatus)TRADE;


				//	cout<<"TRADE_CONFIRMATION_TR =>"<<"step14"<<endl;
                }

				else if(_BS==(short)_TradeReverseSide)
				{
                    /// ReverseSide Leg2 code starts////
                    MS_OE_REQUEST_TR obj_New;
			//		cout<<"TRADE_CONFIRMATION_TR =>"<<"step16"<<endl;
                    memset(&obj_New,0,138);
                    BUYSELL _BS1 = FOpairObj.TokenNearSide == BUY ? SELL :BUY;
			//		cout<<"TRADE_CONFIRMATION_TR =>"<<"step17"<<endl;
                    obj_New=_NMPack.NEARMONTHMARKET;
                    obj_New.DisclosedVolume=obj_New.Volume=obj_Trade.FillQuantity;
                    obj_New.Buy_SellIndicator =ntohs((short)_BS1);

                     _shMKTBuy.Qty1= _BNSFMNQ * BLQ;
                    _shMKTBuy.Price1=0;
                    _shMKTBuy.side1 = (short)_BS1;

                    _pfMKTBuy.HashKey = MyHash<StructHash>()(_shMKTBuy);
                    _pfMKTBuy.side =(short)_BS1;

                    _pfMKTBuy._size = 136;
                    _pfMKTBuy._oetr= obj_New;

                    _PFHashMap.AddRecord(_pfMKTBuy,false);

                    _FillData.MktQueue(_pfMKTBuy);
                    /// ReverseSide Leg2 code ends////

                    /// ReverseSide Leg3 code starts////
                    if(ThirdFP.Token>0)
                    {
                    MS_OE_REQUEST_TR obj_New2;
			//		cout<<"TRADE_CONFIRMATION_TR =>"<<"step16"<<endl;
                    memset(&obj_New2,0,138);
                    BUYSELL _BS2 = FOpairObj.TokenThreeSide == BUY ? SELL :BUY;
			//		cout<<"TRADE_CONFIRMATION_TR =>"<<"step17"<<endl;
                    obj_New2=_NMPack.NEARMONTHMARKET2;
                    obj_New2.DisclosedVolume=obj_New2.Volume=obj_Trade.FillQuantity;
                    obj_New2.Buy_SellIndicator =ntohs((short)_BS2);

                     _shMKTBuy2.Qty1= _BNSFMNQ * BLQ;
                    _shMKTBuy2.Price1=0;
                    _shMKTBuy2.side1 = (short)_BS2;

                    _pfMKTBuy2.HashKey = MyHash<StructHash>()(_shMKTBuy2);
                    _pfMKTBuy2.side =(short)_BS2;

                    _pfMKTBuy2._size = 136;
                    _pfMKTBuy2._oetr= obj_New2;

                    _PFHashMap.AddRecord(_pfMKTBuy2,false);

                    _FillData.MktQueue(_pfMKTBuy2);
                    }
                    /// ReverseSide Leg2 code ends////
                   SellTradeCounter+=1;
                   SellFarStatus = (OrderStatus)TRADE;

			//		cout<<"TRADE_CONFIRMATION_TR =>"<<"step20"<<endl;
                }


			// cout<<"Trade Far  _TKN: "<<_TKN<<" _BS: "<<_BS<<endl;
			// cout<<"TRADE_CONFIRMATION_TR =>"<<"step21"<<endl;
			}
			 else
			 {
			  cout<<"Trade Near  _TKN: "<<_TKN<<" _BS: "<<_BS<<endl;

			  }


		}




		 void StrategyClass::ORDER_CXL_REJ_OUT (char *buffer) //-- 2072
		{//MS_OE_REQUEST 240
//cout<<"ORDER_CXL_REJ_OUT =>"<<"step1"<<endl;
			 MS_OE_REQUEST obj;
	//		 cout<<"ORDER_CXL_REJ_OUT =>"<<"step2"<<endl;
			 memset(&obj,0,240);
		//	 cout<<"ORDER_CXL_REJ_OUT =>"<<"step3"<<endl;
             memcpy(&obj,buffer,240);
			// cout<<"ORDER_CXL_REJ_OUT =>"<<"step4"<<endl;
						    short _BS =htons(obj.Buy_SellIndicator);
				//					cout<<"ORDER_CXL_REJ_OUT =>"<<"step5"<<endl;
							int _TKN =htonl(obj.TokenNo);
					//		cout<<"ORDER_CXL_REJ_OUT =>"<<"step6"<<endl;
							short _Error = htons(obj.ErrorCode);
						//	cout<<"ORDER_CXL_REJ_OUT =>"<<"step7"<<endl;
							switch(_BS)
							{
							case (BUYSELL)BUY:
							//cout<<"ORDER_CXL_REJ_OUT =>"<<"step8"<<endl;
							if (_Error == 16273)

							BuyFarStatus = (OrderStatus)CANCEL;

							else if(BuyFarStatus != (OrderStatus)TRADE)
							BuyFarStatus = (OrderStatus)REPLACED;
							break;
							case (BUYSELL)SELL:
							if (_Error == 16273)
							SellFarStatus = (OrderStatus)CANCEL;
							else if(SellFarStatus != (OrderStatus)TRADE)
							SellFarStatus = (OrderStatus)REPLACED;
							break;

							}
						//	cout<<"ORDER_CXL_REJ_OUT =>"<<"step9"<<endl;
		}


		void StrategyClass::ORDER_MOD_REJ_OUT (char *buffer) //-- 2042
		{//MS_OE_REQUEST 240
//cout<<"ORDER_MOD_REJ_OUT =>"<<"step1"<<endl;
			 MS_OE_REQUEST obj;
	//		 cout<<"ORDER_MOD_REJ_OUT =>"<<"step2"<<endl;
			 memset(&obj,0,240);
		//	 cout<<"ORDER_MOD_REJ_OUT =>"<<"step3"<<endl;
			  memcpy(&obj,buffer,240);
			//  cout<<"ORDER_MOD_REJ_OUT =>"<<"step4"<<endl;

                short _BS =htons(obj.Buy_SellIndicator);
				//cout<<"ORDER_MOD_REJ_OUT =>"<<"step5"<<endl;
				int _TKN =htonl(obj.TokenNo);
				//cout<<"ORDER_MOD_REJ_OUT =>"<<"step6"<<endl;
				short _Error = htons(obj.ErrorCode);
				//cout<<"ORDER_MOD_REJ_OUT =>"<<"step7"<<endl;


				switch(_BS)
				{
				case (BUYSELL)BUY:

				if (_Error == 16273)
				BuyFarStatus= (OrderStatus)CANCEL;
				else if(BuyFarStatus != (OrderStatus)TRADE)
				BuyFarStatus = (OrderStatus)REPLACED;
				break;
				case (BUYSELL)SELL:
				if (_Error == 16273)
				SellFarStatus = (OrderStatus)CANCEL;
				else if(SellFarStatus != (OrderStatus)TRADE)
				SellFarStatus = (OrderStatus)REPLACED;
				break;
				}
				//cout<<"ORDER_MOD_REJ_OUT =>"<<"step8"<<endl;
	}

    void StrategyClass::ORDER_ERROR_OUT (char *buffer) //-- 2231
		{//MS_OE_REQUEST 240
//cout<<"ORDER_ERROR_OUT =>"<<"step1"<<endl;
		MS_OE_REQUEST obj;
	//	cout<<"ORDER_ERROR_OUT =>"<<"step2"<<endl;
		memset(&obj,0,240);
		//cout<<"ORDER_ERROR_OUT =>"<<"step3"<<endl;
        memcpy(&obj,buffer,240);
//cout<<"ORDER_ERROR_OUT =>"<<"step4"<<endl;
        short _BS =htons(obj.Buy_SellIndicator);
	//	cout<<"ORDER_ERROR_OUT =>"<<"step5"<<endl;
		int _TKN =htonl(obj.TokenNo);
		//cout<<"ORDER_ERROR_OUT =>"<<"step6"<<endl;

		switch(_BS)
		{
		case (BUYSELL)BUY:
		BuyFarStatus = (OrderStatus)REJECTED;

     //   cout<<"ORDER_ERROR_OUT BUY\n";
		break;
		case (BUYSELL)SELL:
		SellFarStatus = (OrderStatus)REJECTED;

	//	cout<<"ORDER_ERROR_OUT SELL\n";
		break;
		}
cout<<"ORDER_ERROR_OUT =>"<<"step7"<<endl;
		}


