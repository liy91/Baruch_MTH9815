//
//  BondExecutionService.h
//  MTH 9815 
//


#ifndef BondExecutionService_h
#define BondExecutionService_h

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "BondAlgoExecutionService.h"

/******************************* Code for derived classes ********************************************/

class BondExecutionService: public ExecutionService<Bond>
{
private:
	// map for store data
    std::map<std::string, ExecutionOrder<Bond>> exeData;
    std::vector<ServiceListener<ExecutionOrder<Bond>>*> bExeListeners;
    BondExecutionService() {};
    
public:
    // Override virtual function
    //virtual ExecutionOrder<Bond>& GetData(string key) override {};
    
    // execute an order on a market
    void ExecuteOrder(const ExecutionOrder<Bond>& order, Market market) override
    {
        std::cout << "Executing an order." << std::endl;
        
        std::string prodId = order.GetProduct().GetProductId();
        ExecutionOrder<Bond> bExeNew(order);
        exeData.insert(std::make_pair(prodId, bExeNew));
        
        // notify all listeners 
        ExecutionOrder<Bond> bExe = exeData[prodId];
        for (auto& listener: bExeListeners) listener->ProcessAdd(bExe);
    };
    
	// execute algo strategy
    void ExecuteAlgOrder(BondAlgoExecution &exe)
    {
        auto order = exe.GetExecutionOrder();
        std::string prodId = order.GetProduct().GetProductId();
        exeData[prodId] = order;
        for (auto& listener: bExeListeners) listener->ProcessAdd(order);
    };
    
    // override virtual function
    void OnMessage(ExecutionOrder<Bond>& data) {};
    
    void AddListener(ServiceListener<ExecutionOrder<Bond>> *listener)
    {
        bExeListeners.push_back(listener);
    };
    
    const std::vector<ServiceListener<ExecutionOrder<Bond>>*>& GetListeners() const
    {
        return bExeListeners;
    };
    
	// pull order info
    ExecutionOrder<Bond>& GetData(std::string key) 
    {
        return exeData.at(key);
    };
    
    // create BondHistoricalDataListener object pointer
    static BondExecutionService *create_service()
    {
        static BondExecutionService service;
        return &service;
    };
};

class BondExecutionServiceListener: public ServiceListener<BondAlgoExecution>
{
private:
    BondExecutionService* bExeSer;
    BondExecutionServiceListener()
    {
        bExeSer = BondExecutionService::create_service();
    }

public:
    // add a process
    void ProcessAdd(BondAlgoExecution& exe)
    {
        auto order = exe.GetExecutionOrder();
        bExeSer->ExecuteAlgOrder(exe);
        bExeSer->ExecuteOrder(order, BROKERTEC);
    }
    
	// no implementation
    void ProcessRemove(BondAlgoExecution& data) {};  
    void ProcessUpdate(BondAlgoExecution& data) {};  
    
	// get service
    BondExecutionService* GetService()
    {
        return bExeSer;
    }
    
    static BondExecutionServiceListener* create_listener()
    {
        static BondExecutionServiceListener listener;
        return &listener;
    }
};

#endif /* BondExecutionService_h */
