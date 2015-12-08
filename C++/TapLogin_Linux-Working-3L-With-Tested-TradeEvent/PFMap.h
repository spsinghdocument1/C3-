#ifndef PFMAP_H_INCLUDED
#define PFMAP_H_INCLUDED

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include "Enums.h"

using namespace Enums;

/*
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

using boost::multi_index::multi_index_container;
using boost::multi_index::ordered_non_unique;
using boost::multi_index::ordered_unique;
using boost::multi_index::indexed_by;
using boost::multi_index::member;
using boost::multi_index::tag;
using boost::multi_index::const_mem_fun;


struct Data_entry {
	PFHolder _OM;
	long _HashKey() const { return _OM.HashKey; }
	struct ByHashKey {};
    double _OrderID() const { return _OM.OrderNo; }
    struct ByOrderID {};
	struct RecordChange : public std::unary_function<Data_entry,void> {
		double p; RecordChange(const double &_p) : p(_p) {}
		void operator()(Data_entry & r) { r._OM.OrderNo = p; }
	};

};


typedef boost::multi_index_container
<Data_entry,
	indexed_by
	<
        ordered_non_unique<
                            tag<Data_entry::ByHashKey>, const_mem_fun<Data_entry,long,&Data_entry::_HashKey>
                     >,
        ordered_unique<
                            tag<Data_entry::ByOrderID>, const_mem_fun<Data_entry,double,&Data_entry::_OrderID>
                     >
    >
> Data_set;

*/


class PFHashMap
{
private:
//Data_set _store;
//typedef Data_set::index<Data_entry::ByHashKey>::type PFList;
//typedef Data_set::index<Data_entry::ByOrderID>::type OrderList;

//PFList & _PFList;
//OrderList & _OrderList;


mutable boost::mutex the_mutex;
boost::condition_variable the_condition_variable;
//PFList::const_iterator _PFIterator;
//OrderList::const_iterator _OrdIterator;

//map<long,PFHolder> _InPF;
///map<double,PFHolder> _OutPF;

//PFStore _InPF;
//PFStore _OutPF;

typedef boost::bimap< boost::bimaps::multiset_of<long>, PFHolder > In_bimap;

In_bimap _InPF;

typedef In_bimap::value_type In_position;


typedef boost::bimap< boost::bimaps::multiset_of<double>, PFHolder > Out_bimap;

Out_bimap _OutPF;

typedef Out_bimap::value_type Out_position;

public:

PFHashMap()//:_PFList(_store.get<Data_entry::ByHashKey>())
//,_OrderList(_store.get<Data_entry::ByOrderID>())
{

}

void AddRecord(PFHolder _PF,bool Order)
{
    boost::mutex::scoped_lock lock(the_mutex);

    //cout <<"\n\n &&&&&&&&&&&&&&&&&&&&&& 1 HashKey while creating holder &&&&&&&&&&&&&&&&&&&&&&"<< _PF.HashKey << " Count "<< GetEntryCount() <<endl<<endl;
   // GetEntryCount();
   // Data_entry _DataRec = { _PF};

   // _store.insert(_DataRec);
    long Key = _PF.HashKey;

    //_InPF::iterator _Init = _InPF.find(Key);

    //In_bimap::left_const_iterator left_iter = _InPF.left.find(Key);

    if(!Order)
    {
            _InPF.insert(In_position(_PF.HashKey,_PF));

          //  cout << " Data inserted No key found "<<endl<<endl;
    }
    else if(Order)
    {
      //  _InPF.erase(_Init);
      //  cout << " Key found and erased "<<endl<<endl;
        _OutPF.insert(Out_position(_PF.OrderNo,_PF));
           // cout << " Data inserted with OrderNo "<<endl<<endl;
    }
    else
    {
        cout << " Key error "<<endl;
    }
     lock.unlock();
    the_condition_variable.notify_one();

    //GetEntryCount();
    //cout <<"\n\n &&&&&&&&&&&&&&&&&&&&&& 2 HashKey after creating holder &&&&&&&&&&&&&&&&&&&&&&"<< _PF.HashKey<<  " Count "<<  GetEntryCount()<< endl<<endl;

}



PFHolder GetRecord(long Key)
    {

       // cout <<" HashKey from GetRecord "<< Key<<endl<<endl;
        PFHolder popped_value;
        memset(&popped_value,0,sizeof(PFHolder));
        boost::mutex::scoped_lock lock(the_mutex);

        //In_bimap::left_const_iterator left_iter = _InPF.left.find(Key);
        //In_bimap::left_map& AllKey = _InPF.left;
        //In_bimap::left_map::const_iterator it1 = AllKey.lower_bound(Key);
       // if(it1->first==Key)
        //{
        //    popped_value = it1->second; //_InPF.left.at(Key);
        //    _InPF.left.erase(Key);
        //}

        In_bimap::left_map& AllKey = _InPF.left;
        auto iter_pair = AllKey.equal_range(Key);
        for (auto it = iter_pair.first; it != iter_pair.second;++it)
        {
            popped_value = it->second;
            _InPF.left.erase(it);
            break;
        }


        /*else
        {
        cout << " key error "<<endl;
        }*/

        /*_PFIterator = _PFList.find(Key);
        if(_PFIterator!= _PFList.end())
        {
            popped_value= _PFIterator->_OM;
            _PFList.erase(_PFIterator);
         }*/

        return popped_value;
    }
/*
 void UpdateRecord(long Key,double OrderNumber)
{
    boost::mutex::scoped_lock lock(the_mutex);
        _PFIterator = _PFList.find(Key);
        if(_PFIterator!= _PFList.end())
        {
            _PFList.modify(_PFIterator,Data_entry::RecordChange(OrderNumber));
           // cout << " Request arrived to modify Data for token " << Omsg.Token<<endl<<endl;
        }

}*/

PFHolder GetRecordByOrder(double Key)
    {
        PFHolder popped_value;
        memset(&popped_value,0,sizeof(PFHolder));
        boost::mutex::scoped_lock lock(the_mutex);
       /* _OrdIterator = _OrderList.find(Key);
        if(_OrdIterator!= _OrderList.end())
        {
            popped_value= _OrdIterator->_OM;
        }*/


  Out_bimap::left_map& AllKey = _OutPF.left;
        Out_bimap::left_map::const_iterator it1 = AllKey.lower_bound(Key);
        if(it1->first==Key)
        {
            popped_value = it1->second;
        }
     else
     {
        cout <<  " Key Error "<<endl;
     }
        return popped_value;
    }
/*
    PFHolder GetAllEntry()
    {
    for(PFList::reverse_iterator it = _PFList.rbegin(), it_end(_PFList.rend()); it != it_end; ++it)
    {
           cout << " HashKey "<< it->_OM.HashKey<<endl<<endl;
    }

    }
*/

    int GetEntryCount()
    {
    /*int iloc;
    for(PFList::reverse_iterator it = _PFList.rbegin(), it_end(_PFList.rend()); it != it_end; ++it)
    {
          iloc++;
    }*/
    //cout << " In count "<< _InPF.size() << " Out count "<<_OutPF.size()<<endl<<endl;
    return _InPF.size();
    }

};
extern PFHashMap _PFHashMap;

#endif // PFMAP_H_INCLUDED
