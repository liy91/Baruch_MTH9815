#pragma once
//
//  BondTradeBookingService.h
//  MTH 9815 Final
//

#ifndef BondTradeBookingService_h
#define BondTradeBookingService_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "soa.hpp"
#include "products.hpp"

/***************************************************************************/

/**
* tradebookingservice.hpp
* Defines the data types and Service for trade booking.
*
* @author Breman Thuraisingham
*/

// Trade sides
enum Side { BUY, SELL };

/**
* Trade object with a price, side, and quantity on a particular book.
* Type T is the product type.
*/
template<typename T>
class Trade
{

public:

	Trade() {};
	// ctor for a trade
	Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side);

	// Get the product
	const T& GetProduct() const;

	// Get the trade ID
	const string& GetTradeId() const;

	// Get the mid price
	double GetPrice() const;

	// Get the book
	const string& GetBook() const;

	// Get the quantity
	long GetQuantity() const;

	// Get the side
	Side GetSide() const;

private:
	T product;
	string tradeId;
	double price;
	string book;
	long quantity;
	Side side;

};

/**
* Trade Booking Service to book trades to a particular book.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class TradeBookingService : public Service<string, Trade <T> >
{

public:

	// Book the trade
	void BookTrade(const Trade<T> &trade) = 0;

};

template<typename T>
Trade<T>::Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
	product(_product)
{
	tradeId = _tradeId;
	price = _price;
	book = _book;
	quantity = _quantity;
	side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
	return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
	return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
	return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
	return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
	return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
	return side;
}

template<typename T>
void TradeBookingService<T>::BookTrade(const Trade<T> &trade)
{
}

/****************************** Code for derived classes ***********************************/

using namespace std;

// bond booking service class
class BondTradeBookingService : public Service<std::string, Trade<Bond>>
{
private:
	// ctor
	BondTradeBookingService() {};
	// member bond trade data
	std::map<std::string, Trade<Bond>> tradeData;
	// member bond listeners
	std::vector<ServiceListener<Trade<Bond>>*> bondListeners;

public:
	// book a trade, passing trade data to listeners
	void BookTrade(Trade<Bond>& trade)
	{
		for (auto& listener : bondListeners)
		{
			listener->ProcessAdd(trade);
		}
	};

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Trade<Bond>& trade) override
	{
		auto cusip = trade.GetProduct().GetProductId();
		tradeData.insert(std::make_pair(cusip, trade));
		BookTrade(trade);
	};

	// get bond trade data given cusip
	Trade<Bond>& GetData(std::string cusip) override
	{
		return tradeData.at(cusip);
	};

	// add a listener
	void AddListener(ServiceListener<Trade<Bond>>* listener) override
	{
		bondListeners.push_back(listener);
	};

	// get all listeners
	const vector<ServiceListener<Trade<Bond>>*>& GetListeners() const override
	{
		return bondListeners;
	};

	// return the Bondtradeservice object as a pointer
	static BondTradeBookingService* create_service()
	{
		static BondTradeBookingService service;
		return &service;
	};
};


// connector
class BondTradeBookingConnector : public Connector<Trade<Bond>>
{
private:
	// Bondtradeservice object pointer
	BondTradeBookingService *bookingService;
	BondProductService *productService;

	// ctor: initialzie the bond_service as object pointer of Bondtradeservice
	BondTradeBookingConnector()
	{
		bookingService = BondTradeBookingService::create_service();
		productService = BondProductService::create_service();
	};

public:
	// read data from txt file
	void Subscribe()
	{
		// read data from each line
		auto readLine = [](std::string& row)
		{
			stringstream line(row);
			std::vector<std::string> mystring;
			std::string str;

			while (getline(line, str, ','))
			{
				mystring.push_back(str);
			}
			return mystring;
		};

		// convert string to actual price
		auto strToPrice = [](std::string& str)
		{
			// get size of the string
			int size = str.size();
			// get the last digit of the string
			char lstChar = str[size - 1];
			// convert the last character
			int lstDigit;
			if (lstChar == '+') { lstDigit = 4; }
			else { lstDigit = lstChar - '0'; } // converts a character to the integer value

											   // convert string in the middle
			auto index = str.find_first_of("-");
			int midDigit = std::stoi(str.substr(index + 1, 2));

			// covert the first integer
			double firstDigit = std::stoi(str.substr(0, index));

			// combination of each part
			double res = firstDigit + midDigit / 32.0 + lstDigit / 256.0;
			return res;
		};

		std::cout << "Reading data from trades.txt" << std::endl;
		ifstream myfile("input/trades.txt");

		// bond trade attributes
		std::string cusip, tradeId, book, price, quantity, side;
		std::string row;

		// skip the first line
		getline(myfile, row);
		while (getline(myfile, row))
		{
			// read the line and store the string into a vector
			std::vector<std::string> data = readLine(row);
			// pass the data to each bond attribute
			cusip = data[0];
			tradeId = data[1];
			book = data[2];
			price = data[3];
			quantity = data[4];
			side = data[5];
			// Initialize a Bond object based on the product type
			//Bond &bond = bondMap[cusip];
			Bond &bond = productService->GetData(cusip);
			Side tradeSide;
			if (side == "BUY") { tradeSide = Side::BUY; }
			else { tradeSide = Side::SELL; }
			double tradePrice = strToPrice(price);
			Trade<Bond> bondTrade(bond, tradeId, tradePrice, book, stol(quantity), tradeSide);
			bookingService->OnMessage(bondTrade);
		}
		std::cout << "Risk data is outputed." << std::endl;
	};

	// override the virtual function, subscribe-only connector
	void Publish(Trade<Bond>& data) {};

	BondTradeBookingService *GetService()
	{
		return bookingService;
	};

	// create a connector object, return it as a pointer
	static BondTradeBookingConnector *create_connector()
	{
		static BondTradeBookingConnector connector;
		return &connector;
		std::cout << "A trade booking connector is created." << std::endl;
	};
};

#endif /* BondTradeBookingService_h */
