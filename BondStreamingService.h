//
//  BondStreamingService.h
//  MTH 9815 Final
//
//  Created by Yang on 12/10/17.
//  Copyright © 2017 Yang. All rights reserved.
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
    std::map<std::string, PriceStream<Bond>> streamData;
    std::vector<ServiceListener<PriceStream<Bond>>*> strListeners;
    BondStreamingService() {};
    
public:
    // Publish two-way prices
    void PublishPrice(const PriceStream<Bond>& priceStream)
    {
        // insert information
        //Bond thisBond = priceStream.GetProduct();
        std::string prodId = priceStream.GetProduct().GetProductId();
        
        /*auto it = _streamingData.find(product_ID);
        if (it == _streamingData.end()) {*/
        PriceStream<Bond> newStr(priceStream);
        streamData.insert(std::make_pair(prodId, newStr));
        //}
        //else {
        //	_executionData[product_ID].UpdateOrderBook(od);
        //}
        
        // notify listeners | the following ProcessAdd should send data to history
        PriceStream<Bond> ps = streamData[prodId];
        for (auto& listener: strListeners) listener->ProcessAdd(ps);
    };
    
    void PassBondAlgoStream(const BondAlgoStream& algStr)
    {
        // insert the information
        auto ps = algStr.GetPriceStream();
        std::string prodId = ps.GetProduct().GetProductId();
        streamData[prodId] = ps;
        std::cout << "flow the data from bondstreamingservice to the listener." << std::endl;
        for (auto& listener: strListeners) listener->ProcessAdd(ps);
    };
    
    void OnMessage(PriceStream<Bond>& str) {}; // no implementation
    
    void AddListener(ServiceListener<PriceStream<Bond>> *listener)
    {
        strListeners.push_back(listener);
    };
    
    const std::vector<ServiceListener<PriceStream<Bond>>*>& GetListeners() const
    {
        return strListeners;
    };
    
    PriceStream<Bond>& GetData(std::string key)
    {
        return streamData.at(key);
    };
    
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
    // function overloading
    void ProcessAdd(BondAlgoStream& str)
    {
        auto ps = str.GetPriceStream();
        bondStrServ->PassBondAlgoStream(str);
        bondStrServ->PublishPrice(ps);
    };
    
    void ProcessRemove(BondAlgoStream& data) {};  // No implementation
    void ProcessUpdate(BondAlgoStream& data) {};  // No implementation
    
    BondStreamingService *GetService()
    {
        return bondStrServ;
    };
    
    static BondStreamingServiceListener *create_listener()
    {
        static BondStreamingServiceListener listener;
        return &listener;
    };
};

#endif /* BondStreamingService_h */
