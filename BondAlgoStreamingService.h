//
//  BondAlgoStreamingService.h
//  MTH 9815 Final
//
//  Created by Yang on 12/10/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#ifndef BondAlgoStreamingService_h
#define BondAlgoStreamingService_h

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "BondMarketDataService.h"
#include "BondPricingService.h"

/**************************************************************************************/

/**
* streamingservice.hpp
* Defines the data types and Service for price streams.
*
* @author Breman Thuraisingham
*/

/**
* A price stream order with price and quantity (visible and hidden)
*/
class PriceStreamOrder
{

public:

	// default ctor
	PriceStreamOrder() {};

	// ctor for an order
	PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

	// The side on this order
	PricingSide GetSide() const;

	// Get the price on this order
	double GetPrice() const;

	// Get the visible quantity on this order
	long GetVisibleQuantity() const;

	// Get the hidden quantity on this order
	long GetHiddenQuantity() const;

private:
	double price;
	long visibleQuantity;
	long hiddenQuantity;
	PricingSide side;

};

/**
* Price Stream with a two-way market.
* Type T is the product type.
*/
template<typename T>
class PriceStream
{

public:

	// default ctor
	PriceStream() {};

	// ctor
	PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);

	// Get the product
	const T& GetProduct() const;

	// Get the bid order
	const PriceStreamOrder& GetBidOrder() const;

	// Get the offer order
	const PriceStreamOrder& GetOfferOrder() const;

private:
	T product;
	PriceStreamOrder bidOrder;
	PriceStreamOrder offerOrder;

};

/**
* Streaming service to publish two-way prices.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class StreamingService : public Service<string, PriceStream <T> >
{

public:

	// Publish two-way prices
	virtual void PublishPrice(const PriceStream<T>& priceStream) = 0;

};

PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
{
	price = _price;
	visibleQuantity = _visibleQuantity;
	hiddenQuantity = _hiddenQuantity;
	side = _side;
}

double PriceStreamOrder::GetPrice() const
{
	return price;
}

PricingSide PriceStreamOrder::GetSide() const
{
	return side;
}

long PriceStreamOrder::GetVisibleQuantity() const
{
	return visibleQuantity;
}

long PriceStreamOrder::GetHiddenQuantity() const
{
	return hiddenQuantity;
}

template<typename T>
PriceStream<T>::PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder) :
	product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

template<typename T>
const T& PriceStream<T>::GetProduct() const
{
	return product;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const
{
	return bidOrder;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const
{
	return offerOrder;
}

/******************************** Code for derived classes ********************************************************/
using namespace std;

class BondAlgoStream
{
private:
    //	AlgoStream<Bond> _executionOrder;
    PriceStream<Bond> priceStream;
    
public:
	// default ctor
	BondAlgoStream() {};

    // use algorithm to transform book message to ExecutionOrder classes
    BondAlgoStream (Price<Bond>& price)
	{
		Bond bond = price.GetProduct();
        double midPrice = price.GetMid();
        double spread = price.GetBidOfferSpread();
        double bidPrice = midPrice - (spread / 2);
        double offerPrice = midPrice + (spread / 2);
        long bidVisQ = 1000000;  // some algorithm
        long bidHidQ = 2000000;
        long offerVisQ = 5000000;
        long offerHidQ = 3000000;
        
        PriceStreamOrder psBid(bidPrice, bidVisQ, bidHidQ, BID);
        PriceStreamOrder psOffer(offerPrice, offerVisQ, offerHidQ, OFFER);
        PriceStream<Bond> psBond(bond, psBid, psOffer);
		priceStream = psBond;
	};
    
    PriceStream<Bond> GetPriceStream() const
    {
        return priceStream;
	};
    
    /*
    // update OrderBook information using some algorithm
    void UpdatePrice(const Price<Bond> &p) {
        if (p.GetProduct().GetProductId() != _priceStream.GetProduct().GetProductId()) return;
        double mid = p.GetMid();
        double spread = p.GetBidOfferSpread();
        double bid = mid - spread / 2;
        double ask = mid + spread / 2;
        long bid_visibleQuantity = 10445000;  // some algorithm
        long bid_hiddenQuantity = 23120000;
        long ask_visibleQuantity = 3072100;
        long ask_hiddenQuantity = 43120000;
        PriceStreamOrder ps_bid(bid, bid_visibleQuantity, bid_hiddenQuantity, BID);
        PriceStreamOrder ps_ask(ask, ask_visibleQuantity, ask_hiddenQuantity, OFFER);
        PriceStream<Bond> ps(p.GetProduct(), ps_bid, ps_ask);
        _priceStream = ps;
    }*/
};

/**
 * class BondAlgoExecutionService
 */
class BondAlgoStreamingService: public Service<string, BondAlgoStream>
{
private:
    std::map<std::string, BondAlgoStream> algStrData;              // store algo stream data
    std::vector<ServiceListener<BondAlgoStream>*> alStrListeners;      // member data for listeners
    BondAlgoStreamingService() {};
    
public:
    void AddPrice(Price<Bond>& price)
    {
        std::cout << "flow the data from bondalgostreaming to the listener." << std::endl;
        
        // insert information
        std::string prodId = price.GetProduct().GetProductId();
        /*auto it = _algoExeData.find(product_ID);
        if (it == _algoExeData.end()) {*/
        BondAlgoStream newBStream(price);
        algStrData.insert(std::make_pair(prodId, newBStream));
        /*}
        else {
            _algoExeData[product_ID].UpdatePrice(price);
        }*/
        
        // notify listeners | the following ProcessAdd should send data to history
        BondAlgoStream bStream = algStrData[prodId];
        for (auto& listener: alStrListeners) listener->ProcessAdd(bStream);
    };
    
    void OnMessage(BondAlgoStream& str) {};    // no implementation
    
    BondAlgoStream& GetData(std::string key)
    {
        return algStrData.at(key);
    };
    
    void AddListener(ServiceListener<BondAlgoStream> *listener) 
    {
        alStrListeners.push_back(listener);
    };
    
    const std::vector<ServiceListener<BondAlgoStream>*>& GetListeners() const 
    {
        return alStrListeners;
    };
    
    static BondAlgoStreamingService *create_service()
    {
        static BondAlgoStreamingService service;
        return &service;
    };
};


class BondAlgoStreamingServiceListener: public ServiceListener<Price<Bond>>
{
private:
    BondAlgoStreamingService* bAlgStrSer;
    BondAlgoStreamingServiceListener()
    {
        bAlgStrSer = BondAlgoStreamingService::create_service();
	};
    
public:
    // function overloading
    void ProcessAdd(Price<Bond>& price)
    {
        bAlgStrSer->AddPrice(price);
    };
    
    void ProcessRemove(Price<Bond>& data) {};  // No implementation
    void ProcessUpdate(Price<Bond>& data) {};  // No implementation
    
    BondAlgoStreamingService *GetService()
    {
        return bAlgStrSer;
    };
    
    static BondAlgoStreamingServiceListener *create_listener()
    {
        static BondAlgoStreamingServiceListener listener;
        return &listener;
    };
};

/*
class BondAlgoStreamingService : public Service<string, PriceStream<Bond>>
{
public:
    // Initialize BondHistoricalDataService object pointer and use stream connector to publish data
    void PublishPrice(PriceStream<Bond>& priceStream)
    {
        BondHistoricalDataService::Add_example()->Persist_Streaming(priceStream);
    }
    
    // Override virtual function
    // virtual PriceStream<Bond>& GetData(string key) override {};
    
    // Override virtual function
    virtual void OnMessage(PriceStream<Bond> &data) override {};
    
    // Initialize the BondHistoricalDataListener object as a pointer
    static BondAlgoStreamingService* Add_example()
    {
        static BondAlgoStreamingService example;
        return &example;
    }
    
private:
    // BondPricingService object pointer
    BondPricingService* bond_service;
    
    // ctor : initialize bond_service as object pointer of BondPricingService
    BondAlgoStreamingService()
    {
        bond_service = BondPricingService::Add_example();
    }
};*/

#endif 
/* BondAlgoStreamingService_h */
