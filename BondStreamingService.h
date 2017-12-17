//
//  BondStreamingService.h
//  MTH 9815 
//


#ifndef BondStreamingService_h
#define BondStreamingService_h

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "BondAlgoStreamingService.h"

/******************************* Code for derived classes ***************************************************/

using namespace std;

class BondStreamingService: public StreamingService<Bond>
{
private:
	// a map for price streaming data
    std::map<std::string, PriceStream<Bond>> streamData;
    std::vector<ServiceListener<PriceStream<Bond>>*> strListeners;
    BondStreamingService() {};
    
public:
    // publish price streaming data
    void PublishPrice(const PriceStream<Bond>& priceStream)
    {
        // get cusip
        //Bond thisBond = priceStream.GetProduct();
        std::string prodId = priceStream.GetProduct().GetProductId();
        
        PriceStream<Bond> newStr(priceStream);
        streamData.insert(std::make_pair(prodId, newStr));
        
        // notify the listeners
        PriceStream<Bond> ps = streamData[prodId];
        for (auto& listener: strListeners) listener->ProcessAdd(ps);
    };
    
	// apply algo price stream
    void PassBondAlgoStream(const BondAlgoStream& algStr)
    {
        // get price stream data
        auto ps = algStr.GetPriceStream();
        std::string prodId = ps.GetProduct().GetProductId();
        streamData[prodId] = ps;
        std::cout << "flow the data from bondstreamingservice to the listener." << std::endl;
        for (auto& listener: strListeners) listener->ProcessAdd(ps);
    };
    
	// no implementation
    void OnMessage(PriceStream<Bond>& str) {}; 
    
    void AddListener(ServiceListener<PriceStream<Bond>> *listener)
    {
        strListeners.push_back(listener);
    };
    
    const std::vector<ServiceListener<PriceStream<Bond>>*>& GetListeners() const
    {
        return strListeners;
    };
    
	// get price stream info given cusip
    PriceStream<Bond>& GetData(std::string key)
    {
        return streamData.at(key);
    };
    
	// create pointer
    static BondStreamingService *create_service()
    {
        static BondStreamingService service;
        return &service;
    };
};

class BondStreamingServiceListener: public ServiceListener<BondAlgoStream>
{
private:
    BondStreamingService* bondStrServ;
    BondStreamingServiceListener()
    {
        bondStrServ = BondStreamingService::create_service();
    };
    
public:
    // add process to a listener
    void ProcessAdd(BondAlgoStream& str)
    {
        auto ps = str.GetPriceStream();
        bondStrServ->PassBondAlgoStream(str);
        bondStrServ->PublishPrice(ps);
    };
    
	// no implementation
    void ProcessRemove(BondAlgoStream& data) {};  
	// no implementation
    void ProcessUpdate(BondAlgoStream& data) {};  
    
	// get service of a listener
    BondStreamingService *GetService()
    {
        return bondStrServ;
    };
    
	// create listener pointer
    static BondStreamingServiceListener *create_listener()
    {
        static BondStreamingServiceListener listener;
        return &listener;
    };
};

#endif /* BondStreamingService_h */
