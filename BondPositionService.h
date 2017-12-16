//
//  BondPositionService.h
//  MTH 9815 Final
//
//  Created by Yang on 12/9/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#ifndef BondPositionService_h
#define BondPositionService_h

#include <iostream>
#include <string>
#include <map>
/*#include "SOA_base.h"
#include "Products_base.h"*/
#include "BondTradeBookingService.h"

/****************************************************************************/

/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham
 */

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{
    
public:
    
    // add a default ctor
    Position() {};
    
    // ctor for a position
    Position(const T &_product);
    
    // Get the product
    const T& GetProduct() const;
    
    // Get the position quantity
    long GetPosition(string &book);
    
    // Get the aggregate position
    long GetAggregatePosition();
    
    // Get the position quantity
    void AddPosition(const std::string& book, long& pos);
    
private:
    T product;
    map<string,long> positions;
    
};

/**
 * Position Service to manage positions across multiple books and secruties.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string,Position <T> >
{
    
public:
    
    // Add a trade to the service
    virtual void AddTrade(Trade<T>& trade) = 0;
    
};

template<typename T>
Position<T>::Position(const T &_product) :
product(_product)
{
    for (int i = 0; i < 3; ++i)
    {
        positions["TRSY" + std::to_string(i + 1)] = 10000000;
    }
}

template<typename T>
const T& Position<T>::GetProduct() const
{
    return product;
}

template<typename T>
long Position<T>::GetPosition(string &book)
{
    return positions[book];
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
    // No-op implementation - should be filled out for implementations
    long agg = 0;
    for (int i = 0; i < 3; ++i)
    {
        std::string book = "TRSY" + std::to_string(i + 1);
        agg += positions.at(book);
    }
    return agg;
}

template<typename T>
void Position<T>::AddPosition(const std::string& book, long& pos)
{
    positions[book] += pos;
}

/***************************** Code for derived classes **********************************************/

class BondPositionServiceListener;

class BondPositionService: public Service<string, Position<Bond>>
{
private:
    std::map<std::string, Position<Bond>> posData; // cusip -> position
    std::vector<ServiceListener<Position<Bond>>*> posListeners;
	BondPositionService() {};
    
public:
    
    void Add(Position<Bond>& pos)
    {
        posData.insert(std::make_pair(pos.GetProduct().GetProductId(), pos));
    };
    
    // Add a trade to the service
    void AddTrade(const Trade<Bond>& trade)
    {
        // store the data
        std::string prodId = trade.GetProduct().GetProductId();
        long quantity = trade.GetQuantity();
		long Tquantity;
        if (trade.GetSide() == Side::BUY) { Tquantity = quantity; }
        else { Tquantity = -quantity; }
        posData[prodId].AddPosition(trade.GetBook(), Tquantity);
        // notify listeners | the following ProcessAdd should send data to riskservice
        //Position<Bond> pb = posData[productId];
        for (auto& listener: posListeners) listener->ProcessAdd(posData[prodId]);
    };
    
    Position<Bond>& GetData(std::string cusip) override
    {
        return posData.at(cusip);
    };
    
    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Position<Bond> &data) override {};
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<Position<Bond>>* listener) override
    {
        posListeners.push_back(listener);
    };
    
    // Get all listeners on the Service.
    const std::vector<ServiceListener<Position<Bond>>*>& GetListeners() const override
    {
        return posListeners;
    };

    // Initialize the BondPositionService object as a pointer
    static BondPositionService *create_service()
    {
        static BondPositionService service;
        return &service;
    };
};

class BondPositionServiceListener: public ServiceListener<Trade<Bond>>
{
private:
    // BondRiskService object pointer
    BondPositionService *bondPosSer;
    
    // ctor : initialzie bond_service as object pointer of BondRiskService
    BondPositionServiceListener()
    {
        bondPosSer = BondPositionService::create_service();
    };
    
public:
    // Add data to the RiskService
    void ProcessAdd(Trade<Bond>& data) override
    {
        bondPosSer->AddTrade(data);
    };
    
    // override the virtual function
    void ProcessRemove(Trade<Bond>& data) override {};
    
    // override the virtual function
    void ProcessUpdate(Trade<Bond>& data) override {};
    
    // get service
    BondPositionService* GetService()
    {
        return bondPosSer;
    };
    
    // Initialize the BondpositionserviceListener object as a pointer
    static BondPositionServiceListener* create_listener()
    {
        static BondPositionServiceListener listener;
        return &listener;
		std::cout << "A position service listener is created." << std::endl;
    };
};

#endif /* BondPositionService_h */
