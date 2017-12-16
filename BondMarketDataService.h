//
//  BondMarketDataService.h
//  MTH 9815 Final
//
//  Created by Yang on 12/10/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#ifndef BondMarketDataService_h
#define BondMarketDataService_h

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include "BondProductService.h"

/*******************************************************************************/
/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Breman Thuraisingham
 */

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{
    
public:
    
	// default ctor
	Order() {};

    // ctor for an order
    Order(double _price, long _quantity, PricingSide _side);
    
    // Get the price on the order
    double GetPrice() const;
    
    // Get the quantity on the order
    long GetQuantity() const;
    
    // Get the side on the order
    PricingSide GetSide() const;
    
private:
    double price;
    long quantity;
    PricingSide side;
    
};

/**
 * Class representing a bid and offer order
 */
class BidOffer
{
    
public:
    
	BidOffer() {};

    // ctor for bid/offer
    BidOffer(const Order &_bidOrder, const Order &_offerOrder);
    
    // Get the bid order
    const Order& GetBidOrder() const;
    
    // Get the offer order
    const Order& GetOfferOrder() const;
    
private:
    Order bidOrder;
    Order offerOrder;
    
};

/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{
    
public:
    
    // ctor for the order book
    OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);
    
    // Get the product
    const T& GetProduct() const;
    
    // Get the bid stack
    const vector<Order>& GetBidStack() const;
    
    // Get the offer stack
    const vector<Order>& GetOfferStack() const;
    
private:
    T product;
    vector<Order> bidStack;
    vector<Order> offerStack;
    
};

/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string,OrderBook <T> >
{
    
public:
    
    // Get the best bid/offer order
    virtual const BidOffer& GetBestBidOffer(const string &productId) = 0;
    
    // Aggregate the order book
    virtual const OrderBook<T>& AggregateDepth(const string &productId) = 0;
    
};

Order::Order(double _price, long _quantity, PricingSide _side)
{
    price = _price;
    quantity = _quantity;
    side = _side;
}

double Order::GetPrice() const
{
    return price;
}

long Order::GetQuantity() const
{
    return quantity;
}

PricingSide Order::GetSide() const
{
    return side;
}

BidOffer::BidOffer(const Order &_bidOrder, const Order &_offerOrder) :
bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

const Order& BidOffer::GetBidOrder() const
{
    return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
    return offerOrder;
}

template<typename T>
OrderBook<T>::OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack) :
product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

template<typename T>
const T& OrderBook<T>::GetProduct() const
{
    return product;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetBidStack() const
{
    return bidStack;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetOfferStack() const
{
    return offerStack;
}

/*********************************** Code for derived classes ******************************************/

using namespace std;

class BondMarketDataService: public Service<std::string, OrderBook<Bond>>
{
private:
    std::map<std::string, OrderBook<Bond>> marketData;
    std::vector<ServiceListener<OrderBook<Bond>>*> mdListeners;
    BondMarketDataService() {};
    
public:
    // override the virtual function
    //virtual OrderBook<Bond>& GetData(string key) override {};
    
    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(OrderBook<Bond>& data) 
    {
        for(auto& listener: mdListeners)
        {
            listener->ProcessUpdate(data);
        }
    };
    
    OrderBook<Bond>& GetData(std::string key) override
    {
        return marketData.at(key);
    };
    
    void AddListener(ServiceListener<OrderBook<Bond>> *listener) override
    {
        mdListeners.push_back(listener);
    };
    
    const std::vector<ServiceListener<OrderBook<Bond>>*>& GetListeners() const override
    {
        return mdListeners;
    };
    
    // Initialize the BondMarketDataService object as a pointer
    static BondMarketDataService* create_service()
    {
        static BondMarketDataService service;
        return &service;
    };
};

class BondMarketDataConnector: public Connector<OrderBook<Bond>>
{
private:
    // define the bond_service
    BondMarketDataService *bondMDSer;
	BondProductService *productService;
    
    // ctor : initialize the bond_service as the BondMarketDataService as object pointer
    BondMarketDataConnector()
    {
        bondMDSer = BondMarketDataService::create_service();
		productService = BondProductService::create_service();
	};
    
public:
    // override the virtual function
    void Publish(OrderBook<Bond> &data) {};
    
    // Read the data from marketdata.txt
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
            size_t index = str.find_first_of('-');
            int midDigit = std::stoi(str.substr(index + 1, 2));
            
            // covert the first integer
            double firstDigit = std::stoi(str.substr(0, index));
            
            // combination of each part
            double res = firstDigit + midDigit / 32.0 + lstDigit / 256.0;
            return res;
        };
        
        std::cout << "Reading market data from marketdata.txt" << std::endl;
        ifstream myfile("marketdata.txt");
        string row;
        
        PricingSide bidSide = BID; //initialize the bid price side
        PricingSide offerSide = OFFER; //initialize the offer price side
        std::vector<Order> bidOrder; //initialize the bid order vector
        std::vector<Order> offerOrder; //initialize the offer order vector
        string cusip;
        double bidPrice;
        double offerPrice;
        long bidQuantity;
        long offerQuantity;
        int bidIndex = 1;
        int offerIndex = 11;
        
        getline(myfile, row);
        while (getline(myfile, row))
        {
            bidIndex = 1;
            offerIndex = 11;
            
            // initialzie the string vector to store the data read from the txt file
            std::vector<std::string> data = readLine(row);
            cusip = data[0];
            for (int i = 0; i < 5; ++i)
            {
                // bidOrder
                // translate the price
                bidPrice = strToPrice(data[bidIndex++]);
                bidQuantity = std::stol(data[bidIndex++]);  //convert string to long
                // Review of Order ctor:
                // Order(double _price, long _quantity, PricingSide _side);
                Order bid_order(bidPrice, bidQuantity, bidSide);
                // push back the Order object to the vector
                bidOrder.push_back(bid_order);
                
                // offerOrder
                // translate the price
                offerPrice = strToPrice(data[offerIndex++]);
                offerQuantity = std::stol(data[offerIndex++]); //convert string to long
                Order offer_order(offerPrice, offerQuantity, offerSide);
                // push back the Order object to the vector
                offerOrder.push_back(offer_order);
            }
            // Initialize the Bond object based on the product type
            //Bond& bond = bondMap[cusip];
			Bond &bond = productService->GetData(cusip);
            // Review of OrderBook ctor:
            // OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);
            OrderBook<Bond> bondOrderBook(bond, bidOrder, offerOrder);
			bondMDSer->OnMessage(bondOrderBook);
        }
        
        std::cout << "Reading market data is done." << std::endl;
	};
    
    
    BondMarketDataService *GetService()
    {
        return bondMDSer;
	};
    
    // Initialize the BondMarketDataConnector object as a pointer
    static BondMarketDataConnector* create_connector()
    {
        static BondMarketDataConnector connector;
        return &connector;
    };
};

#endif /* BondMarketDataService_h */
