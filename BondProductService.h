//
//  Preprocessing.h
//  MTH 9815 Final
//
//  Created by Yang on 12/9/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#ifndef BondProductService_h
#define BondProductService_h

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include "SOA_base.h"
#include "Products_base.h"

class BondProductService
{
private:
	std::map<std::string, Bond> bondMap; // cache of bond products

								// BondProductService ctor
	BondProductService() 
	{
		bondMap = map<string, Bond>();
	};

public:
	static BondProductService* create_service() 
	{
		static BondProductService service;
		return &service;
	}

	// Return the bond data for a particular bond product identifier
	Bond& GetData(std::string key) 
	{
		return bondMap[key];
	};

	// Add a bond to the service (convenience method)
	void Add(Bond &bond) 
	{
		bondMap.insert(std::make_pair(bond.GetProductId(), bond));
	};

	// Get all Bonds with the specified ticker
	std::vector<Bond> GetBonds(const std::string& tik) const 
	{
		std::vector<Bond> bondVec;
		for (auto& bd : bondMap) {
			if (bd.second.GetTicker() == tik) bondVec.push_back(bd.second);
		}
		return bondVec;
	};
};

#endif 
/* BondProductService_h */
