//
//  main.cpp
//  MTH 9815 Final
//
//  Created by Yang on 12/3/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#include <iostream>
#include "BondTradeBookingService.h"
#include "BondPositionService.h"
#include "BondInquiryService.h"
#include "BondRiskService.h"
#include "BondMarketDataService.h"
#include "BondAlgoExecutionService.h"
#include "BondExecutionService.h"
#include "BondAlgoStreamingService.h"
#include "BondStreamingService.h"
#include "BondInquiryService.h"
#include "BondHistoricalDataService.h"
#include "GenerateTradeFile.h"
#include "GeneratePriceFile.h"
#include "GenerateMarketDataFile.h"
#include "GenerateInquiryFile.h"

void create_bondMap()
{
	// Initialize maps
	std::map<std::string, Bond> bondMap;

	if (bondMap.size() == 0)
	{
		// Initialize the Bond type vector
		// Review: Bond constructor
		// Bond(string _productId, BondIdType _bondIdType, string _ticker, float _coupon, date _maturityDate);
		// For each product, initialize one Bond object
		Bond bonds[] =
		{
			Bond(cusips[0], BondIdType::CUSIP, "T", 0.002100, date(2019,12,28)),
			Bond(cusips[1], BondIdType::CUSIP, "T", 0.002600, date(2020,12,28)),
			Bond(cusips[2], BondIdType::CUSIP, "T", 0.002900, date(2022,12,28)),
			Bond(cusips[3], BondIdType::CUSIP, "T", 0.003500, date(2024,12,28)),
			Bond(cusips[4], BondIdType::CUSIP, "T", 0.003800, date(2027,12,18)),
			Bond(cusips[5], BondIdType::CUSIP, "T", 0.005000, date(2047,12,28))
		};

		auto BondPositionServ = BondPositionService::create_service();
		auto BondRiskServ = BondRiskService::create_service();
		auto BondProdServ = BondProductService::create_service();

		for (int i = 0; i < 6; i++)
		{
			// For each product type, assign the elements in the temp vector to the type
			//bondMap.insert(std::pair<string, Bond>(cusips[i], bonds[i]));
			BondProdServ->Add(bonds[i]);
			Position<Bond> posTemp(bonds[i]);
			PV01<Bond> pv01Temp(bonds[i], (rand() % 1000)/1000000.0, posTemp.GetAggregatePosition());
			BondPositionServ->Add(posTemp);
			BondRiskServ->Add(pv01Temp);
		};
	}
	std::cout << "Finished the initializing..." << std::endl;
};

int main() {
    
    // generate trades.txt file
    generate_trades();
    // generate prices.txt file
    generate_prices();
    // generate marketdata.txt file
    generate_marketdata();
    // generate inquiries.txt file
    generate_inquiry();
	// initialize
	create_bondMap();

    /* tradingbookingservice -> positionservice -> riskservice -> historicaldataservice*/
    auto BondTradeBookingServConn = BondTradeBookingConnector::create_connector();
	// read the data and output risk.txt
    // connect positionservice with tradebookingservice via listener
    auto BondTradeBookingServ = BondTradeBookingServConn->GetService();
    auto BondPosServListener = BondPositionServiceListener::create_listener();
    BondTradeBookingServ->AddListener(BondPosServListener);
    // connect riskservice with positionservice via listener
    auto BondPosServ = BondPosServListener->GetService();
    auto BondRiskServListener = BondRiskServiceListener::create_listener();
    BondPosServ->AddListener(BondRiskServListener);
    // connect bondhistoricalPV01Service with riskservice via listener
    auto BondRiskServ = BondRiskServListener->GetService();
    auto BondHisRiskServListener = BondHisRiskServiceListener::create_listener();
    BondRiskServ->AddListener(BondHisRiskServListener);
	BondTradeBookingServConn->Subscribe();
    
    /*marketdataservice ->algoexecution -> execution -> historicaldataservice*/
    /*auto BondMarketDataServConn = BondMarketDataConnector::create_connector();
    // connect bondalgoexecuteservice with pricingservice via listener
    auto BondMarketDataServ = BondMarketDataServConn->GetService();
    auto BondAlgoExeServListener = BondAlgoExecutionServiceListener::create_listener();
    BondMarketDataServ->AddListener(BondAlgoExeServListener);
    // connect execution with bondalgoexcution via listener
    auto BondAlgoExeServ = BondAlgoExeServListener->GetService();
    auto BondExeServListener = BondExecutionServiceListener::create_listener();
    BondAlgoExeServ->AddListener(BondExeServListener);
    // connect bondhistoricalExecutionService with riskservice via listener
    auto BondExeServ = BondExeServListener->GetService();
    auto BondHisExeServListener = BondHisExecutionServiceListener::create_listener();
	BondExeServ->AddListener(BondHisExeServListener);
    // read the data and output risk.txt
    BondMarketDataServConn->Subscribe();*/


    /*priceservice ->algostreaming ->streaming ->historicaldataservice*/
    /*auto BondPrServConn = BondPricingConnector::create_connector();
    // connect bondalgostreamingservice with priceservice via listener
    auto BondPrServ = BondPrServConn->GetService();
    auto BondAlStreamServListener = BondAlgoStreamingServiceListener::create_listener();
    BondPrServ->AddListener(BondAlStreamServListener);
    // connect algostreaming with streaming via listener
    auto BondAlStreamServ = BondAlStreamServListener->GetService();
    auto BondStreamServListener = BondStreamingServiceListener::create_listener();
    BondAlStreamServ->AddListener(BondStreamServListener);
    // connect bondhistoricalStreamingService with streaming via listener
    auto BondStreamServ = BondStreamServListener->GetService();
    auto BondHisStreamServListener = BondHisStreamingServiceListener::create_listener();
	BondStreamServ->AddListener(BondHisStreamServListener);
    // read the data and output stream.txt
    BondPrServConn->Subscribe();*/
    


    
    /*inquiryservice -> historicaldataservice*/
    /*auto BondInqServConn = BondInquiryConnector::create_connector();
    // connect bondhistoricalInquiryService with inquiryservice via listener
    auto BondInqServ = BondInqServConn->GetService();
    auto BondHisInqServListener = BondHisInquiryServiceListener::create_listener();
	BondInqServ->AddListener(BondHisInqServListener);
    // read the data and output stream.txt
    BondInqServConn->Subscribe();*/

	std::system("pause");
};
