//
//  BondAlgoExecutionService.h
//  MTH 9815 Final
//
//  Created by Yang on 12/10/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#ifndef BondAlgoExecutionService_h
#define BondAlgoExecutionService_h

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "BondMarketDataService.h"

/**************************************************************************/
/**
* executionservice.hpp
* Defines the data types and Service for executions.
*
* @author Breman Thuraisingham
*/

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

/**
* An execution order that can be placed on an exchange.
* Type T is the product type.
*/
template<typename T>
class ExecutionOrder
{

public:
	// default ctor
	ExecutionOrder() {};

	// ctor for an order
	ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

	// Get the product
	const T& GetProduct() const;

	// Get the order ID
	const string& GetOrderId() const;

	// Get the order type on this order
	OrderType GetOrderType() const;

	// Get the price on this order
	double GetPrice() const;

	// Get the visible quantity on this order
	long GetVisibleQuantity() const;

	// Get the hidden quantity
	long GetHiddenQuantity() const;

	// Get the parent order ID
	const string& GetParentOrderId() const;

	// Is child order?
	bool IsChildOrder() const;

private:
	T product;
	PricingSide side;
	string orderId;
	OrderType orderType;
	double price;
	double visibleQuantity;
	double hiddenQuantity;
	string parentOrderId;
	bool isChildOrder;

};

/**
* Service for executing orders on an exchange.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class ExecutionService : public Service<string, ExecutionOrder <T> >
{

public:

	// Execute an order on a market
	virtual void ExecuteOrder(const ExecutionOrder<T>& order, Market market) = 0;

};

template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
	product(_product)
{
	side = _side;
	orderId = _orderId;
	orderType = _orderType;
	price = _price;
	visibleQuantity = _visibleQuantity;
	hiddenQuantity = _hiddenQuantity;
	parentOrderId = _parentOrderId;
	isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
	return product;
}

template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
	return orderId;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
	return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
	return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
	return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
	return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
	return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
	return isChildOrder;
}

/************************************************ Code for derived classes **********************************************/
using namespace std;

class BondAlgoExecution
{
private:
    ExecutionOrder<Bond> exeOrder;
    
public:
    // default
	BondAlgoExecution() {};
    // // use algorithm to transform book message to ExecutionOrder classes
    BondAlgoExecution(OrderBook<Bond>& order)
    {
        auto bond = order.GetProduct();
        // some strategy
        static int id1 = 0;
        static int id2 = 1;
        
        std::string orderID = std::to_string(id1++);
        std::string parentID;
        bool isChild = false;
        PricingSide side;
        Order bidOrder;
        Order offerOrder;
        double bidPrice;
        double offerPrice;
        long bidVisQ;
        long offerVisQ;
        long hidQ = 0;
        OrderType orderType;
        switch (id1 % 5)
        {
            case 0: orderType = FOK;
                    break;
            case 1: orderType = MARKET;
                    break;
            case 2: orderType = LIMIT;
                    break;
            case 3: orderType = STOP;
                    break;
            case 4: orderType = IOC;
        }
        
        id2++;
        if (id2 % 2 == 0)
        {
            bidOrder = order.GetBidStack()[0];
            bidPrice = bidOrder.GetPrice();
            bidVisQ = bidOrder.GetQuantity();
            side = BID;
            parentID = "P" + orderID;
            ExecutionOrder<Bond> Ex_bid(bond, side, orderID, orderType, bidPrice, bidVisQ, hidQ, parentID, isChild);
            exeOrder = Ex_bid;
        }
        else
        {
            offerOrder = order.GetOfferStack()[0];
            offerPrice = offerOrder.GetPrice();
            offerVisQ = offerOrder.GetQuantity();
            side = OFFER;
            parentID = "P" + orderID;
            ExecutionOrder<Bond> Ex_offer(bond, side, orderID, orderType, offerPrice, offerVisQ, hidQ, parentID, isChild);
            exeOrder = Ex_offer;
        }
    };
    
    ExecutionOrder<Bond>& GetExecutionOrder() 
    {
        return exeOrder;
	};
    
    // update OrderBook information using some algorithm
   /* void UpdateOrderBook(const OrderBook<Bond>& order)
    {
        if (o.GetProduct().GetProductId() != _executionOrder.GetProduct().GetProductId()) return;
        // some strategy
        static int id = 0;
        ++id;
        string orderID = "O" + to_string(id);
        PricingSide side = ((id % 2) ? BID : OFFER);
        OrderType orderType;
        switch (id % 5) {
            case 0: orderType = FOK; break;
            case 1: orderType = MARKET; break;
            case 2: orderType = LIMIT; break;
            case 3: orderType = STOP; break;
            case 4: orderType = IOC;
        }
        auto ask = o.GetOfferStack().begin();
        auto bid = o.GetBidStack().begin();
        double price = (side == BID ? ask->GetPrice() : bid->GetPrice());
        long vQ = (side == BID ? ask->GetQuantity() : bid->GetQuantity());
        long hQ = 0;
        string parentID = orderID + "P";
        bool isChild = true;
        ExecutionOrder<Bond> myEx(_executionOrder.GetProduct(), side, orderID, orderType, price, vQ, hQ, parentID, isChild);
        _executionOrder = myEx;
    };*/
};

/**
 * class BondAlgoExecutionService
 */
class BondAlgoExecutionService: public Service<std::string, BondAlgoExecution>
{
private:
    std::map<std::string, BondAlgoExecution> algoExeData;              // store algo stream data
    std::vector<ServiceListener<BondAlgoExecution>*> alExListeners;      // member data for listeners
    BondAlgoExecutionService() {};
    
public:
    // function overloading
    BondAlgoExecution& GetData(std::string key) 
    {
        return algoExeData.at(key);
    }
    
    void OnMessage(BondAlgoExecution& exe) {};    // no implementation
    
    void AddListener(ServiceListener<BondAlgoExecution> *listener) 
    {
        alExListeners.push_back(listener);
    };
    
    const std::vector<ServiceListener<BondAlgoExecution>*>& GetListeners() const 
    {
        return alExListeners;
    };
    
    void AddBook(OrderBook<Bond>& order)
    {
        // std::cout << "flow the data from bondalgoexcution to the listener." << std::endl;
        // insert information
        //Bond thisBond = od.GetProduct();
        string proId = order.GetProduct().GetProductId();
        BondAlgoExecution newExe(order);
        algoExeData.insert(std::make_pair(proId, newExe));

        /*auto it = _algoExeData.find(product_ID);
        if (it == _algoExeData.end()){
            AlgoExecution newExecution(od);
            _algoExeData.insert(std::make_pair(product_ID, newExecution));
        }
        else{
            _algoExeData[product_ID].UpdateOrderBook(od);
        }*/
        
        // notify listeners | the following ProcessAdd should send data to history
        BondAlgoExecution algExe = algoExeData[proId];
        for (auto& listener: alExListeners) listener->ProcessAdd(algExe);
    };
    
    static BondAlgoExecutionService* create_service()
    {
        static BondAlgoExecutionService service;
        return &service;
    }
};

class BondAlgoExecutionServiceListener: public ServiceListener<OrderBook<Bond>>
{
private:
    BondAlgoExecutionService* bAlgoExeSer;
    BondAlgoExecutionServiceListener()
    {
        bAlgoExeSer = BondAlgoExecutionService::create_service();
    }
    
public:
    // function overloading
    void ProcessAdd(OrderBook<Bond>& data) 
    {
        bAlgoExeSer->AddBook(data);
    };
    
    void ProcessRemove(OrderBook<Bond> &data) {};  // No implementation
    void ProcessUpdate(OrderBook<Bond> &data) {};  // No implementation
    
    BondAlgoExecutionService* GetService()
    {
        return bAlgoExeSer;
    };
    
    static BondAlgoExecutionServiceListener* create_listener()
    {
        static BondAlgoExecutionServiceListener listener;
        return &listener;
    };
};

#endif /* BondAlgoExecutionService_h */
