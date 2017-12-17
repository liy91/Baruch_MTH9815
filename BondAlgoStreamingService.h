//
//  BondAlgoStreamingService.h
//  MTH 9815 
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
	// price streaming member data
    PriceStream<Bond> priceStream;
    
public:
	// default ctor
	BondAlgoStream() {};

    // ctor with price
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
    
	// pull price streaming data
    PriceStream<Bond> GetPriceStream() const
    {
        return priceStream;
	};
};

/**
 * class BondAlgoExecutionService
 */
class BondAlgoStreamingService: public Service<string, BondAlgoStream>
{
private:
	// map to store algo stream data
    std::map<std::string, BondAlgoStream> algStrData;           
	// member listeners
    std::vector<ServiceListener<BondAlgoStream>*> alStrListeners;      
    BondAlgoStreamingService() {};
    
public:
	// add a price
    void AddPrice(Price<Bond>& price)
    {
        std::cout << "flow the data from bondalgostreaming to the listener." << std::endl;
        
        // add information
        std::string prodId = price.GetProduct().GetProductId();
        BondAlgoStream newBStream(price);
        algStrData.insert(std::make_pair(prodId, newBStream));
        
        // notify all listeners 
        BondAlgoStream bStream = algStrData[prodId];
        for (auto& listener: alStrListeners) listener->ProcessAdd(bStream);
    };
    
	// no implementation
    void OnMessage(BondAlgoStream& str) {};   
    
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
    // add price process
    void ProcessAdd(Price<Bond>& price)
    {
        bAlgStrSer->AddPrice(price);
    };
    
	// no implementation
    void ProcessRemove(Price<Bond>& data) {}; 
    void ProcessUpdate(Price<Bond>& data) {};  
    
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

#endif 
/* BondAlgoStreamingService_h */
