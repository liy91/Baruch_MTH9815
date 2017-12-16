//
//  BondExecutionService.h
//  MTH 9815 Final
//
//  Created by Yang.
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
    std::map<std::string, ExecutionOrder<Bond>> exeData;
    std::vector<ServiceListener<ExecutionOrder<Bond>>*> bExeListeners;
    BondExecutionService() {};
    
public:
    // Override virtual function
    //virtual ExecutionOrder<Bond>& GetData(string key) override {};
    
    // Execute an order on a market
    void ExecuteOrder(const ExecutionOrder<Bond>& order, Market market) override
    {
        std::cout << "Executing an order." << std::endl;
        
        std::string prodId = order.GetProduct().GetProductId();
        ExecutionOrder<Bond> bExeNew(order);
        exeData.insert(std::make_pair(prodId, bExeNew));
        
        // notify listeners | the following ProcessAdd should send data to history
        ExecutionOrder<Bond> bExe = exeData[prodId];
        for (auto& listener: bExeListeners) listener->ProcessAdd(bExe);
    };
    
    void ExecuteAlgOrder(BondAlgoExecution &exe)
    {
        auto order = exe.GetExecutionOrder();
        std::string prodId = order.GetProduct().GetProductId();
        exeData[prodId] = order;
        //std::cout << "flow the data from bondexecutionservice to the listener." << std::endl;
        for (auto& listener: bExeListeners) listener->ProcessAdd(order);
    };
    
    // Override virtual function
    void OnMessage(ExecutionOrder<Bond>& data) {};
    
    void AddListener(ServiceListener<ExecutionOrder<Bond>> *listener)
    {
        bExeListeners.push_back(listener);
    };
    
    const std::vector<ServiceListener<ExecutionOrder<Bond>>*>& GetListeners() const
    {
        return bExeListeners;
    };
    
    ExecutionOrder<Bond>& GetData(std::string key) 
    {
        return exeData.at(key);
    };
    
    // Initialize the BondHistoricalDataListener object as a pointer
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
    // function overloading
    void ProcessAdd(BondAlgoExecution& exe)
    {
        auto order = exe.GetExecutionOrder();
        bExeSer->ExecuteAlgOrder(exe);
        bExeSer->ExecuteOrder(order, BROKERTEC);
    }
    
    void ProcessRemove(BondAlgoExecution& data) {};  // No implementation
    void ProcessUpdate(BondAlgoExecution& data) {};  // No implementation
    
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
