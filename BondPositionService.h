#pragma once
//
//  BondPositionService.h
//  MTH 9815
//

#ifndef BondPositionService_h
#define BondPositionService_h

#include <iostream>
#include <string>
#include <map>
#include <string>
#include <map>
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
	map<string, long> positions;

};

/**
* Position Service to manage positions across multiple books and secruties.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class PositionService : public Service<string, Position <T> >
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

class BondPositionService : public Service<std::string, Position<Bond>>
{
private:
	// a map for pos data, cusip -> position
	std::map<std::string, Position<Bond>> posData; 
	std::vector<ServiceListener<Position<Bond>>*> posListeners;
	BondPositionService() {};

public:
	// add a position
	void Add(Position<Bond>& pos)
	{
		posData.insert(std::make_pair(pos.GetProduct().GetProductId(), pos));
	};

	// add a trade to the service
	void AddTrade(const Trade<Bond>& trade)
	{
		// store the data
		std::string prodId = trade.GetProduct().GetProductId();
		long quantity = trade.GetQuantity();
		long Tquantity;
		if (trade.GetSide() == Side::BUY) { Tquantity = quantity; }
		else { Tquantity = -quantity; }
		posData[prodId].AddPosition(trade.GetBook(), Tquantity);
		// notify listeners 
		//Position<Bond> pb = posData[productId];
		for (auto& listener : posListeners) listener->ProcessAdd(posData[prodId]);
	};

	// get price info given cusip
	Position<Bond>& GetData(std::string cusip) override
	{
		return posData.at(cusip);
	};

	// override virtual
	void OnMessage(Position<Bond> &data) override {};

	// add a listener add, remove, and update operations
	void AddListener(ServiceListener<Position<Bond>>* listener) override
	{
		posListeners.push_back(listener);
	};

	// get all listeners
	const std::vector<ServiceListener<Position<Bond>>*>& GetListeners() const override
	{
		return posListeners;
	};

	// create BondPositionService object as a pointer
	static BondPositionService *create_service()
	{
		static BondPositionService service;
		return &service;
	};
};


class BondPositionServiceListener : public ServiceListener<Trade<Bond>>
{
private:
	// initilaize BondRiskService object pointer
	BondPositionService *bondPosSer;

	// create object pointers of BondRiskService
	BondPositionServiceListener()
	{
		bondPosSer = BondPositionService::create_service();
	};

public:
	// add data to service, override 
	void ProcessAdd(Trade<Bond>& data) override
	{
		bondPosSer->AddTrade(data);
	};

	// override the virtual function, no implementation
	void ProcessRemove(Trade<Bond>& data) override {};

	// override the virtual function, no implementation
	void ProcessUpdate(Trade<Bond>& data) override {};

	// get service of a listener
	BondPositionService* GetService()
	{
		return bondPosSer;
	};

	// create the BondpositionserviceListener object as a pointer
	static BondPositionServiceListener* create_listener()
	{
		static BondPositionServiceListener listener;
		return &listener;
		std::cout << "A position service listener is created." << std::endl;
	};
};

#endif /* BondPositionService_h */
