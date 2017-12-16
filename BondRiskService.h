//
//  BondRiskService.h
//  MTH 9815 Final
//
//  Created by Yang on 12/12/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#ifndef BondRiskService_h
#define BondRiskService_h

#include <iostream>
#include <fstream>
#include <vector>
//#include "SOA_base.h"
//#include "Products_base.h"
#include "BondPositionService.h"

/******************************************************************************************/

/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Breman Thuraisingham
 */

/**
 * PV01 risk.
 * Type T is the product type.
 */
template<typename T>
class PV01
{
    
public:
    
	// default ctor
	PV01() {};

    // ctor for a PV01 value
    PV01(const T &_product, double _pv01, long _quantity);
    
    // Get the product on this PV01 value
	const T& GetProduct() const
	{
		return product;
	}
    
    // Get the PV01 value
	double GetPV01() const
	{
		return pv01;
	}
    
    // Get the quantity that this risk value is associated with
	long GetQuantity() const
	{
		return quantity;
	}
    
    void AddQuantity(long q);
    
    void UpdatePV01(double pv01);
    
private:
    T product;
    double pv01;
    long quantity;
    
};

/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{
    
public:
    
    // ctor for a bucket sector
    BucketedSector(const vector<T> &_products, string _name);
    
    // Get the products associated with this bucket
    const vector<T>& GetProducts() const;
    
    // Get the name of the bucket
    const string& GetName() const;
    
private:
    vector<T> products;
    string name;
    
};

/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class RiskService : public Service<string,PV01 <T> >
{
    
public:
    
    // Add a position that the service will risk
    void AddPosition(Position<T> &position) = 0;
    
    // Get the bucketed risk for the bucket sector
    const PV01<T>& GetBucketedRisk(const BucketedSector<T> &sector) const = 0;
    
};

template<typename T>
PV01<T>::PV01(const T &_product, double _pv01, long _quantity) :
product(_product)
{
    pv01 = _pv01;
    quantity = _quantity;
}

template<typename T>
void PV01<T>::AddQuantity(long q)
{
    quantity = quantity + q;
}

template<typename T>
void PV01<T>::UpdatePV01(double pv)
{
    pv01 = pv01 + pv;
}

template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
products(_products)
{
    name = _name;
}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
    return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
    return name;
}

/*********************************** Code for derived classes *****************************************************/

using namespace std;

class BondRiskServiceListener;

class BondRiskService: public Service<string, PV01<Bond>>
{
private:
    std::map<std::string, PV01<Bond>> riskData;
    std::vector<ServiceListener<PV01<Bond>>*> riskListeners;
    BondRiskService() {};
    
public:
    // override the virtual function
    //virtual PV01<Bond>& GetData(string key) override {};
    
    // override the virtual function
    void OnMessage(PV01<Bond>& data) override {};
    
    // Add new position
    void AddPosition(Position<Bond>& position)
    {
        std::cout << "Adding a position." << std::endl;
        /*int n = riskMap.size();
        // Initialize the "myRiskMap"
        if (n == 0)
        {
            riskMap["912828U40"] = 0.1237;
            riskMap["912828U32"] = 0.1352;
            riskMap["912828U65"] = 0.1532;
            riskMap["912828U57"] = 0.1586;
            riskMap["912828U24"] = 0.1627;
            riskMap["912828U40"] = 0.1695;
        }*/
        std::string prodId = position.GetProduct().GetProductId();
        // get the pv01 risk based on the productID it retrieve
		double addPV01 = (rand() % 1000) / 100000.0;
        riskData[prodId].UpdatePV01(addPV01);
        // get the position it holds
        long addQ = position.GetAggregatePosition();
        riskData[prodId].AddQuantity(addQ);
		std::cout << "Updating risk." << std::endl;
        // Review of PV01 ctor:
        // PV01(const vector<T> &_products, double _pv01, long _quantity);
        PV01<Bond>& pv01B = riskData[prodId];
        // Initialize a BondHistoricalDataService object pointer and use risk connector to publish the data
		for (auto& listener : riskListeners) { listener->ProcessAdd(riskData[prodId]); }
    }
    
    // Get the bucketed risk for the bucket sector
    double GetBucketedRisk(const BucketedSector<Bond>& sector)
    {
        double pv01 = 0;
        for (auto& bond: sector.GetProducts()) pv01 = pv01 + riskData.at(bond.GetProductId()).GetPV01();
        return pv01;
    };
    
    void Add(PV01<Bond>& data)
    {
        riskData.insert(std::make_pair(data.GetProduct().GetProductId(), data));
    };
    
    // Get data on our service given a key
    PV01<Bond>& GetData(std::string key) override
    {
        return riskData.at(key);
    };
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<PV01<Bond>> *listener) override
    {
        riskListeners.push_back(listener);
    };
    
    // Get all listeners on the Service.
    const vector<ServiceListener<PV01<Bond>>*>& GetListeners() const override
    {
        return riskListeners;
    };
    
    // Initialize the BondRiskService object as a pointer
    static BondRiskService *create_service()
    {
        static BondRiskService service;
        return &service;
    };
};

class BondRiskServiceListener: public ServiceListener<Position<Bond>>
{
private:
    BondRiskService *bondRiskSer;
    BondRiskServiceListener()
    {
        bondRiskSer = BondRiskService::create_service();
	};
    
public:
    void ProcessAdd(Position<Bond>& data)
    {
        bondRiskSer->AddPosition(data);
	};
    
    void ProcessRemove(Position<Bond>& data) {};
    void ProcessUpdate(Position<Bond>& data) {};
    
    BondRiskService *GetService()
    {
        return bondRiskSer;
	};
    
    static BondRiskServiceListener *create_listener()
    {
        static BondRiskServiceListener listener;
        return &listener;
	};
};

#endif /* BondRiskService_h */
