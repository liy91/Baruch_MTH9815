//
//  BondHistoricalDataService.h
//  MTH 9815 Final
//
//  Created by Yang on 12/9/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#ifndef BondHistoricalDataService_h
#define BondHistoricalDataService_h

#include <iostream>
#include <fstream>
#include <map>
#include "BondRiskService.h"
#include "BondStreamingService.h"
#include "BondExecutionService.h"
#include "BondInquiryService.h"
#include "BondProductService.h"

/*******************************************************************************/
/**
 * historicaldataservice.hpp
 * historicaldataservice.hpp
 *
 * @author Breman Thuraisingham
 * Defines the data types and Service for historical data.
 *
 * @author Breman Thuraisingham
 */

/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist.
 */
template<typename T>
class HistoricalDataService : Service<string,T>
{
    
public:
    
    // Persist data to a store
	virtual void PersistData(string persistKey, T& data) = 0;
    
};

/********************************* Code for derived classes ***************************************************/

using namespace std;

/**************** Connectors ****************/
/**************** BondRiskConnector **************/
/**************** BondExecutionConnector **************/
/**************** BondStreamingConnector **************/
/**************** BondInquiryConnector **************/

//ios_base::app is to open an output file
class BondHisRiskConnector: public Connector<PV01<Bond>>
{
public:
    // ctor for RiskConnector
	BondHisRiskConnector() {};
    
    // Publish data into "risk.txt" file
    void Publish(PV01<Bond>& data) 
    {
        ofstream myfile("risk.txt", ios_base::app);
        std::cout << "Persisting risk data." << std::endl;
        std::string pv01 = std::to_string(data.GetPV01());
        std::string output = "PV01 is: " + pv01;
        myfile << output << std::endl;
	};

	void Subscribe() {};  // implement nothing, publish-only
    
    // Initialize the RiskConnector object as a pointer
    static BondHisRiskConnector *create_connector()
    {
        static BondHisRiskConnector connector;
        return &connector;
	};
};

class BondHisExecutionConnector: public Connector<ExecutionOrder<Bond>>
{
public:
    // ctor for ExecutionConnector
	BondHisExecutionConnector() {};
    
    // Publish data into "execution.txt" file
    void Publish(ExecutionOrder<Bond>& data) 
    {
        ofstream myfile("execution.txt", ios_base::app);
        std::cout << "Persisting execution data." << std::endl;
		std::string output = "Order Id is: " + data.GetOrderId() + ", CUSIP Id is: " + data.GetProduct().GetProductId();
        myfile << output << std::endl;
	};

	void Subscribe() {};  // implement nothing, publish-only
    
    // Initialize the ExecutionConnector object as a pointer
    static BondHisExecutionConnector *create_connector()
    {
        static BondHisExecutionConnector connector;
        return &connector;
	};
};

class BondHisStreamingConnector: public Connector<PriceStream<Bond>>
{
public:
    // ctor for StreamingConnector
	BondHisStreamingConnector() {};
    
    // Publish data into "execution.txt" file
    void Publish(PriceStream<Bond>& data) 
    {
        ofstream myfile("streaming.txt", ios_base::app);
        std::cout << "Persisting streaming data." << std::endl;
        
        std::string productId = data.GetProduct().GetProductId();
        std::string offer_price = std::to_string(data.GetOfferOrder().GetPrice());
        std::string bid_price = std::to_string(data.GetBidOrder().GetPrice());
        std::string output = "Product Id is: " + productId + ", Bid price is: " + bid_price + ", Offer price is: "+ offer_price + ";";

        myfile << output << std::endl;
	};
    
	void Subscribe() {};  // implement nothing, publish-only

    // Initialize the StreamingConnector object as a pointer
    static BondHisStreamingConnector *create_connector()
    {
        static BondHisStreamingConnector connector;
        return &connector;
	};
};

class BondHisInquiryConnector: public Connector<Inquiry<Bond>>
{
public:
    // ctor for InquiryConnector
	BondHisInquiryConnector() {};
    
    // Publish data into "allinquiries.txt" file
    void Publish(Inquiry<Bond> &data) 
    {
        ofstream myfile("allinquiries.txt", ios_base::app);
        std::cout << "Persisting inquiry data." << std::endl;
        
        std::string productId = data.GetProduct().GetProductId();
        std::string inquiryId = data.GetInquiryId();
        std::string price = std::to_string(data.GetPrice());
        std::string quantity = std::to_string(data.GetQuantity());
        std::string side;
        if (data.GetSide() == Side::BUY)
        {
            side = "BUY";
        }
        else
        {
            side = "SELL";
        }
        std::string state;
        if (data.GetState() == InquiryState::RECEIVED)
        {
            state = "RECEIVED";
        }
        else
        {
            state = "OTHERS";
        }
        std::string output = "Product Id: " + productId + ", Inquiry Id: " + inquiryId + ", Price: " + price + ", Quantity: " + quantity + ", Side: " + side +", State: " + state + ";";
        myfile << output << std::endl;
	};

	void Subscribe() {};  // implement nothing, publish-only
    
    // Initialize the InquiryConnector object as a pointer
    static BondHisInquiryConnector *create_connector()
    {
        static BondHisInquiryConnector connector;
        return &connector;
	};
};


/********************* BondRiskService ******************/
/********************* BondRiskServiceListener *****************/

class BondHisRiskService: public HistoricalDataService<PV01<Bond>>
{
private:
    std::map<std::string, PV01<Bond>> riskData;                       // store the type data to persist
    std::vector<ServiceListener<PV01<Bond>>*> riskListeners;      // member data for listeners
    BondHisRiskConnector *bondRiskConn; // call connector to write
    
	BondHisRiskService()
    {
        bondRiskConn = BondHisRiskConnector::create_connector();
	};
    
public:
    // function overloading
    PV01<Bond>& GetData(std::string key)
    {
        return riskData.at(key);
	};
    
    void OnMessage(PV01<Bond>& trade) 
    {
        auto persistKey = trade.GetProduct().GetProductId();
        riskData[persistKey] = trade;
        std::cout << "flow the data from BondHistoricalPV01Service to the listener." << std::endl;
        for (auto& listener: riskListeners) listener->ProcessAdd(trade); // notify listeners
	};
    
    void PersistData(std::string key, PV01<Bond>& data) 
    {
        bondRiskConn->Publish(data);
	};
    
    void AddListener(ServiceListener<PV01<Bond>> *listener) 
    {
        riskListeners.push_back(listener);
	};
    
    const std::vector<ServiceListener<PV01<Bond>>*>& GetListeners() const
    {
        return riskListeners;
	};
    
    static BondHisRiskService *create_service()
    {
        static BondHisRiskService service;
        return &service;
	};
};

class BondHisRiskServiceListener: public ServiceListener<PV01<Bond>>
{
private:
	BondHisRiskService *bondRiskSer;
	BondHisRiskServiceListener()
    {
        bondRiskSer = BondHisRiskService::create_service();
    }

public:
    // function overloading
    void ProcessAdd(PV01<Bond>& data) 
    {
        bondRiskSer->OnMessage(data);
        bondRiskSer->PersistData(data.GetProduct().GetProductId(), data); // to write.
	};
    
	void ProcessRemove(PV01<Bond>& data) {};
	void ProcessUpdate(PV01<Bond>& data) {};
    
    static BondHisRiskServiceListener *create_listener()
    {
        static BondHisRiskServiceListener listener;
        return &listener;
	};
};


/********************* BondExecutionService ******************/
/********************* BondExecutionServiceListener *****************/

class BondHisExecutionService: public HistoricalDataService<ExecutionOrder<Bond>>
{
private:
    std::map<std::string, ExecutionOrder<Bond>> exeData;                       // store the type data to persist
    std::vector<ServiceListener<ExecutionOrder<Bond>>*> exeListeners;      // member data for listeners
    BondHisExecutionConnector *bondExeConn; // call connector to write
    
	BondHisExecutionService()
    {
        bondExeConn = BondHisExecutionConnector::create_connector();
	};
    
public:
    // function overloading
    ExecutionOrder<Bond>& GetData(string key)
    {
        return exeData.at(key);
	};
    
    void OnMessage(ExecutionOrder<Bond>& trade)
    {
        auto persistKey = trade.GetProduct().GetProductId();
        exeData[persistKey] = trade;
        for (auto& listener: exeListeners) listener->ProcessAdd(trade); // notify listeners
	};
    
    void PersistData(std::string persistKey, ExecutionOrder<Bond>& data) 
    {
        bondExeConn->Publish(data);
	};
    
    void AddListener(ServiceListener<ExecutionOrder<Bond>> *listener) 
    {
        exeListeners.push_back(listener);
	};
    
    const vector<ServiceListener<ExecutionOrder<Bond>>*>& GetListeners() const
    {
        return exeListeners;
	};
    
    static BondHisExecutionService *create_service()
    {
        static BondHisExecutionService service;
        return &service;
    }
};

class BondHisExecutionServiceListener: public ServiceListener<ExecutionOrder<Bond>>
{
private:
    BondHisExecutionService *bondExeSer;
	BondHisExecutionServiceListener()
    {
        bondExeSer = BondHisExecutionService::create_service();
	};
    
public:
    // function overloading
    void ProcessAdd(ExecutionOrder<Bond>& data)
    {
        bondExeSer->OnMessage(data);
        bondExeSer->PersistData(data.GetProduct().GetProductId(), data); // to write.
	};
    
	void ProcessRemove(ExecutionOrder<Bond>& data) {};
	void ProcessUpdate(ExecutionOrder<Bond>& data) {};
    
    static BondHisExecutionServiceListener *create_listener()
    {
        static BondHisExecutionServiceListener listener;
        return &listener;
	};
};


/********************* BondStreamingService ******************/
/********************* BondStreamingServiceListener *****************/

class BondHisStreamingService: public HistoricalDataService<PriceStream<Bond>>
{
private:
    std::map<std::string, PriceStream<Bond>> streamData;                       // store the type data to persist
    std::vector<ServiceListener<PriceStream<Bond>>*> streamListeners;      // member data for listeners
    
    BondHisStreamingConnector *bondStreamConn; // call connector to write
	BondHisStreamingService()
    {
        bondStreamConn = BondHisStreamingConnector::create_connector();
	};
    
public:
    // function overloading
    PriceStream<Bond>& GetData(string key)
    {
        return streamData.at(key);
	};
    
    void OnMessage(PriceStream<Bond>& trade)
    {
        auto persistKey = trade.GetProduct().GetProductId();
        streamData[persistKey] = trade;
        
        for (auto& listener: streamListeners) listener->ProcessAdd(trade); // notify listeners
	};
    
    void PersistData(std::string persistKey, PriceStream<Bond>& data)
    {
        bondStreamConn->Publish(data);
	};
    
    void AddListener(ServiceListener<PriceStream<Bond>> *listener)
    {
        streamListeners.push_back(listener);
	};
    
    const vector<ServiceListener<PriceStream<Bond>>*>& GetListeners() const
    {
        return streamListeners;
	};
    
    static BondHisStreamingService* create_service()
    {
        static BondHisStreamingService service;
        return &service;
    }
};

class BondHisStreamingServiceListener: public ServiceListener<PriceStream<Bond>>
{
private:
    BondHisStreamingService *bondStreamSer;
    BondHisStreamingServiceListener()
    {
        bondStreamSer = BondHisStreamingService::create_service();
	};
    
public:
    // function overloading
    void ProcessAdd(PriceStream<Bond>& data)
    {
        bondStreamSer->OnMessage(data);
        bondStreamSer->PersistData(data.GetProduct().GetProductId(), data); // to write.
	};
    
	void ProcessRemove(PriceStream<Bond>& data) {};
	void ProcessUpdate(PriceStream<Bond>& data) {};
    
    static BondHisStreamingServiceListener *create_listener()
    {
        static BondHisStreamingServiceListener listener;
        return &listener;
	};
};


/********************* BondStreamingService ******************/
/********************* BondStreamingServiceListener *****************/

class BondHisInquiryService: public HistoricalDataService<Inquiry<Bond>>
{
private:
    std::map<std::string, Inquiry<Bond>> inquiryData;                       // store the type data to persist
    std::vector<ServiceListener<Inquiry<Bond>>*> inquiryListeners;      // member data for listeners
    BondHisInquiryConnector *bondInqConn; // call connector to write
    
    BondHisInquiryService()
    {
        bondInqConn = BondHisInquiryConnector::create_connector();
	};
    
public:
    // function overloading
    Inquiry<Bond>& GetData(std::string key)
    {
        return inquiryData.at(key);
	};
    
    void OnMessage(Inquiry<Bond>& trade)
    {
        auto persistKey = trade.GetProduct().GetProductId();
        inquiryData[persistKey] = trade;
        
        for (auto& listener: inquiryListeners) listener->ProcessAdd(trade); // notify listeners
	};
    
    void PersistData(string persistKey, Inquiry<Bond>& data)
    {
        bondInqConn->Publish(data);
	};
    
    void AddListener(ServiceListener<Inquiry<Bond>> *listener)
    {
        inquiryListeners.push_back(listener);
	};
    
    const vector<ServiceListener<Inquiry<Bond>>*>& GetListeners() const
    {
        return inquiryListeners;
    }
    
    static BondHisInquiryService *create_service()
    {
        static BondHisInquiryService service;
        return &service;
    }
};

class BondHisInquiryServiceListener: public ServiceListener<Inquiry<Bond>>
{
private:
    BondHisInquiryService* bondInqSer;
    BondHisInquiryServiceListener()
    {
        bondInqSer = BondHisInquiryService::create_service();
	};
    
public:
    // function overloading
    void ProcessAdd(Inquiry<Bond>& data) 
    {
        bondInqSer->OnMessage(data);
        bondInqSer->PersistData(data.GetProduct().GetProductId(), data); // to write.
	};
    
	void ProcessRemove(Inquiry<Bond>& data) {};
	void ProcessUpdate(Inquiry<Bond>& data) {};

    static BondHisInquiryServiceListener *create_listener()
    {
        static BondHisInquiryServiceListener listener;
        return &listener;
	};
};

#endif 
/* BondHistoricalDataService_h */
