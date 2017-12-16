//
//  BondInquiryService.h
//  MTH 9815 Final
//
//  Created by Yang on 12/11/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#ifndef BondInquiryService_h
#define BondInquiryService_h

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "BondProductService.h"

/***********************************************************************************************/
/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Breman Thuraisingham
 */

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{
    
public:
	// default ctor
	Inquiry() {};

    // ctor for an inquiry
    Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state);
    
    // Get the inquiry ID
    const string& GetInquiryId() const;
    
    // Get the product
    const T& GetProduct() const;
    
    // Get the side on the inquiry
    Side GetSide() const;
    
    // Get the quantity that the client is inquiring for
    long GetQuantity() const;
    
    // Get the price that we have responded back with
    double GetPrice() const;
    
    // Get the current state on the inquiry
    InquiryState GetState() const;
    
    // Set state
    void SetState(double stPrice, InquiryState st);
    
private:
    string inquiryId;
    T product;
    Side side;
    long quantity;
    double price;
    InquiryState state;
    
};

/**
 * Service for customer inquirry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string,Inquiry <T> >
{
    
public:
    
    // Send a quote back to the client
    void SendQuote(const string &inquiryId, double price) = 0;
    
    // Reject an inquiry from the client
    void RejectInquiry(const string &inquiryId) = 0;
    
};

template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state) :
product(_product)
{
    inquiryId = _inquiryId;
    side = _side;
    quantity = _quantity;
    price = _price;
    state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
    return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
    return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
    return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
    return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
    return price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
    return state;
}

template<typename T>
void Inquiry<T>::SetState(double stPrice, InquiryState st)
{
    price = stPrice;
    state = st;
}

/********************************** Code for derived classes ***************************************************/

using namespace std;

class BondInquiryService: public Service<std::string, Inquiry<Bond>>
{
private:
    std::map<std::string, Inquiry<Bond>> inquiryData;
    std::vector<ServiceListener<Inquiry<Bond>>*> inqListeners;
    BondInquiryService() {};
    
public:
    // override the virtual function
    //virtual Inquiry<Bond>& GetData(string key) override {};
    
    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Inquiry<Bond>& data) override
    {
        cout << "You are now in the inquiry service, sending an inquiry object with QUOTED state" << endl;
        data.SetState(data.GetPrice(), DONE);
        
        // Initialzie a BondHistoricalDataService object pointer and use inquiry connector to publish data
        for (auto& listener: inqListeners) listener->ProcessAdd(data);
	};
    
    Inquiry<Bond>& GetData(std::string key) override
    {
        return inquiryData.at(key);
    };
    
    // override the virtual function
    void SendQuote(const std::string& inquiryId, double price) {};
    
    // Reject an inquiry from the client
    void RejectInquiry(const std::string& inquiryId) {};
    
    void AddListener(ServiceListener<Inquiry<Bond>> *listener) override
    {
        inqListeners.push_back(listener);
    };
    
    const std::vector<ServiceListener<Inquiry<Bond>>*>& GetListeners() const override
    {
        return inqListeners;
    };
    
    // Initialize the BondInquiryService object as a pointer
    static BondInquiryService *create_service()
    {
        static BondInquiryService service;
        return &service;
    };
};

class BondInquiryConnector: public Connector<Inquiry<Bond>>
{
private:
    BondInquiryService *bondInqServ;
	BondProductService *productService;

    BondInquiryConnector()
    {
        bondInqServ = BondInquiryService::create_service();
		productService = BondProductService::create_service();
    };
    
public:
    // override the virtual function
    void Publish(Inquiry<Bond>& data) {};
    
    // Read the inquiries.txt
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

        std::cout << "Reading inquiry data from inquiries.txt" << std::endl;
        
        static int inqIndex = 1;
        ifstream myfile("inquiries.txt");
        string row;
        getline(myfile, row);
        
        InquiryState inqState;
        Side inqSide;
        std::string cusip, price, quantity, side, state;
        
        while (getline(myfile, row))
        {
            // create a string vector to store the data read from the txt file
            std::vector<string> data = readLine(row);
            cusip = data[0];
            price = data[1];
            quantity = data[2];
            side = data[3];
            state = data[4];
            
            // convert to string
            std::string inquiryId = std::to_string(inqIndex);
            
            // Initialize a Bond object based on the product type
            // Review of Bond ctor:
            // Bond(string _productId, BondIdType _bondIdType, string _ticker, float _coupon, date _maturityDate);
            //Bond& bond = bondMap[cusip];
			Bond &bond = productService->GetData(cusip);
            
            long inqQuantity = std::stol(quantity);  //convert to the long type
            double inqPrice = std::stod(price);  //convert to the long type
            
            if (side == "BUY")
            {
                inqSide = Side::BUY;
            }
            else
            {
                inqSide = Side::SELL;
            }
            
            if (state == "RECEIVED")
            {
                inqState = InquiryState::RECEIVED;
            }
            else
            {
                inqState = InquiryState::DONE;
            }
            
            // initialize the Inquiry object based on the attributes
            // Review of Inquiry ctor
            // Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state);
            Inquiry<Bond> inqB(inquiryId, bond, inqSide, inqQuantity, inqPrice, inqState);
            bondInqServ->OnMessage(inqB);
        };
        
        inqIndex++;
        std::cout << "Reading inquiry data is done." << std::endl;
    };
    
    BondInquiryService *GetService()
    {
        return bondInqServ;
    };
    
    // Initialize the BondInquiryConnector object as a pointer
    static BondInquiryConnector *create_connector()
    {
        static BondInquiryConnector connector;
        return &connector;
    };
};

#endif 
/* BondInquiryService_h */
