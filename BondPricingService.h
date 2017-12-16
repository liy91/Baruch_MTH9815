//
//  BondPricingService.h
//  MTH 9815 Final
//
//  Created by Yang on 12/11/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#ifndef BondPricingService_h
#define BondPricingService_h

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "BondProductService.h"

/*************************************************************************************/
/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Breman Thuraisingham
 */

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{
    
public:
    
	Price() {};

    // ctor for a price
    Price(const T &_product, double _mid, double _bidOfferSpread);
    
    // Get the product
    const T& GetProduct() const;
    
    // Get the mid price
    double GetMid() const;
    
    // Get the bid/offer spread around the mid
    double GetBidOfferSpread() const;
    
private:
    const T& product;
    double mid;
    double bidOfferSpread;
    
};

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string,Price <T> >
{
};

template<typename T>
Price<T>::Price(const T &_product, double _mid, double _bidOfferSpread) :
product(_product)
{
    mid = _mid;
    bidOfferSpread = _bidOfferSpread;
}

template<typename T>
const T& Price<T>::GetProduct() const
{
    return product;
}

template<typename T>
double Price<T>::GetMid() const
{
    return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
    return bidOfferSpread;
}

/********************************** Code for derived classes **************************************************/

using namespace std;

class BondPricingService: public Service<string, Price<Bond>>
{
private:
    std::map<std::string, Price<Bond>> priceData;
    std::vector<ServiceListener<Price<Bond>>*> priceListeners;      // member data for listeners
    BondPricingService() {};
    
public:
    // override the virtual function
    //virtual Price<Bond>& GetData(string key) override {};
    
    // Review Price<T> ctor:
    // Price(const T &_product, double _mid, double _bidOfferSpread);
    void OnMessage(Price<Bond>& price) override
    {
        // get the bid offer spread
        //double price_spread = data.GetBidOfferSpread();
        // get the mid price
        //double mid_price = data.GetMid();
        // calculate the bid price and offer price
        //double offerPrice = mid_price + price_spread / 2;
        //double bidPrice = mid_price - price_spread / 2;
        
        // Initialize two PriceStreamOrder object
        // Review PriceStreamOrder ctor:
        // PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);
        //PriceStreamOrder Offer(offerPrice, 0, 0, PricingSide::OFFER);
        //PriceStreamOrder Bid(bidPrice, 0, 0, PricingSide::BID);
        
        // Initialize one PriceStream object
        // Review PriceStream ctor:
        // PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);
        //PriceStream<Bond> price_stream(data.GetProduct(), Bid, Offer);
        
        // Initialize one BondStreamService object pointer and use stream connector to publish data
        //BondStreamingService::Add_example()->PublishPrice(price_stream);
        
        auto prodId = price.GetProduct().GetProductId();
        priceData.insert(std::make_pair(prodId, price));
        
        std::cout << "flow the data from pricingservice to the listener." << std::endl;
        for (auto& listener: priceListeners) listener->ProcessAdd(price);
    };
    
    Price<Bond>& GetData(std::string key) override
    {
        return priceData.at(key);
    };
    
    void AddListener(ServiceListener<Price<Bond>> *listener) override
    {
        priceListeners.push_back(listener);
    };
    
    const vector<ServiceListener<Price<Bond>>*>& GetListeners() const override
    {
        return priceListeners;
    };
    
    // Initialize the BondPricingService object as a pointer
    static BondPricingService *create_service()
    {
        static BondPricingService service;
        return &service;
    };
};


class BondPricingConnector: public Connector<Price<Bond>>
{
private:
    // BondPricingService object pointer
    BondPricingService *bondServ;
	BondProductService *productService;
    // ctor : initialize bond_service as object pointer of BondPricingService
    BondPricingConnector()
    {
        bondServ = BondPricingService::create_service();
		productService = BondProductService::create_service();
    };
    
public:
    // override the virtual function
    void Publish(Price<Bond>& data) {};
    
    // Read data from price.txt file
    void Subscribe()
    {
        std::cout << "Reading pricing data from prices.txt" << std::endl;
        ifstream myfile("prices.txt");
        
        string row;
        string cusip, midPrice, bid_offer_spread;
        
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
        
        getline(myfile, row);
        
        while (getline(myfile, row))
        {
            // Initialize a vector to store the string it reads from price.txt
            std::vector<std::string> data = readLine(row);
            
            // Assign elements to the string attributes
            //tie(product_type, mid_price, bid_offer_spread) = make_tuple(data[0], data[1], data[2]);
            cusip = data[0];
            midPrice = data[1];
            bid_offer_spread = data[2];
            
            // translate the price
            double mid = strToPrice(midPrice);
            double spread = strToPrice(bid_offer_spread);
            
            // Initialize one Bond object based on the product type
            //Bond& bond = bondMap[cusip];
			Bond &bond = productService->GetData(cusip);
            
            // Review Price ctor:
            // Price(const T &_product, double _mid, double _bidOfferSpread);
            Price<Bond> bPrice(bond, mid, spread);
            
            // Publish data
            bondServ->OnMessage(bPrice);
        }
        std::cout << "Reading pricing data is done." << std::endl;
    };
    
    BondPricingService* GetService()
    {
        return bondServ;
    };
    
    // Initialize the BondPricingConnector object as a pointer
    static BondPricingConnector *create_connector()
    {
        static BondPricingConnector connector;
        return &connector;
    };
};

#endif /* BondPricingService_h */
