//
//  main.cpp
//  MTH 9815 
//

#include "products.hpp"
#include "BondTradeBookingService.h"
#include "BondPositionService.h"
#include "BondRiskService.h"
#include "BondPricingService.h"
#include "BondMarketDataService.h"
#include "BondHistoricalDataService.h"
#include "GenerateTradeFile.h"
#include "GeneratePriceFile.h"
#include "GenerateMarketDataFile.h"
#include "GenerateInquiryFile.h"

/****************** function for initialization *************************/
void initialize_bondMap() 
{

	Bond bonds[] =
	{
		Bond(cusips[0], BondIdType::CUSIP, "T", 0.002100, date(2019,12,28)),
		Bond(cusips[1], BondIdType::CUSIP, "T", 0.002600, date(2020,12,28)),
		Bond(cusips[2], BondIdType::CUSIP, "T", 0.002900, date(2022,12,28)),
		Bond(cusips[3], BondIdType::CUSIP, "T", 0.003500, date(2024,12,28)),
		Bond(cusips[4], BondIdType::CUSIP, "T", 0.003800, date(2027,12,18)),
		Bond(cusips[5], BondIdType::CUSIP, "T", 0.005000, date(2047,12,28))
	};

	auto BondProdServ = BondProductService::create_service();
	auto BondPositionServ = BondPositionService::create_service();
	auto BondRiskServ = BondRiskService::create_service();


	for (int i = 0; i < 6; i++)
	{
		// For each product type, assign the elements in the temp vector to the type
		//bondMap.insert(std::pair<string, Bond>(cusips[i], bonds[i]));
		BondProdServ->Add(bonds[i]);
		Position<Bond> posTemp(bonds[i]);
		PV01<Bond> pv01Temp(bonds[i], (rand() % 1000) / 1000000.0, posTemp.GetAggregatePosition());
		BondPositionServ->Add(posTemp);
		BondRiskServ->Add(pv01Temp);
	};
	std::cout << "Finished the initializing..." << std::endl;
};

/************************** Main Function *******************************/
int main()
{
	// initialize bond information
	initialize_bondMap();
	// generate trades.txt file
	generate_trades();
	// generate prices.txt file
	generate_prices();
	// generate marketdata.txt file
	generate_marketdata();
	// generate inquiries.txt file
	generate_inquiry();

	// BondTradeBookingService -> BondPositionService -> BondRiskService -> BondHisRiskService
	// connect BondTradeBookingService with BondPositionService 
	auto BondTradeBookingServConn = BondTradeBookingConnector::create_connector();
	auto BondTradeBookingServ = BondTradeBookingServConn->GetService();
	auto BondPosServListener = BondPositionServiceListener::create_listener();
	BondTradeBookingServ->AddListener(BondPosServListener);
	// connect BondRiskService with BondRiskService 
	auto BondPosServ = BondPosServListener->GetService();
	auto BondRiskServListener = BondRiskServiceListener::create_listener();
	BondPosServ->AddListener(BondRiskServListener);
	auto BondRiskServ = BondRiskServListener->GetService();
	// connect BondHisRiskService with BondRiskService
	auto BondHisRiskServListener = BondHisRiskServiceListener::create_listener();
	BondRiskServ->AddListener(BondHisRiskServListener);
	// output risk data
	BondTradeBookingServConn->Subscribe();

	// BondMarketDataService -> BondAlgoExecutionService -> BondExecutionService -> BondHisExecutionService
	// connect BondAlgoExecutionService with BondMarketDataService
	auto BondMarketDataServConn = BondMarketDataConnector::create_connector();
	auto BondMarketDataServ = BondMarketDataServConn->GetService();
	auto BondAlgoExeServListener = BondAlgoExecutionServiceListener::create_listener();
	BondMarketDataServ->AddListener(BondAlgoExeServListener);
	auto BondAlgoExeServ = BondAlgoExeServListener->GetService();
	// connect BondExecutionService with BondAlgoExecutionService
	auto BondExeServListener = BondExecutionServiceListener::create_listener();
	BondAlgoExeServ->AddListener(BondExeServListener);
	auto BondExeServ = BondExeServListener->GetService();
	// connect BondHisExecutionService with BondExecutionService
	auto BondHisExeServListener = BondHisExecutionServiceListener::create_listener();
	BondExeServ->AddListener(BondHisExeServListener);
	// output execution data
	BondMarketDataServConn->Subscribe();

	// BondPricingService -> BondAlgoStreamingService -> BondStreamingService -> BondHisStreamingService
	// connect BondAlgoStreamingService with BondPricingService
	auto BondPrServConn = BondPricingConnector::create_connector();
	auto BondPrServ = BondPrServConn->GetService();
	auto BondAlStreamServListener = BondAlgoStreamingServiceListener::create_listener();
	BondPrServ->AddListener(BondAlStreamServListener);
	auto BondAlStreamServ = BondAlStreamServListener->GetService();
	// connect BondStreamingService with BondAlgoStreamingService
	auto BondStreamServListener = BondStreamingServiceListener::create_listener();
	BondAlStreamServ->AddListener(BondStreamServListener);
	auto BondStreamServ = BondStreamServListener->GetService();
	// connect BondHisStreamingService with BondStreamingService
	auto BondHisStreamServListener = BondHisStreamingServiceListener::create_listener();
	BondStreamServ->AddListener(BondHisStreamServListener);
	// output streaming data
	BondPrServConn->Subscribe();

	// BondInquiryService -> BondHisInquiryService
	// connect BondHisInquiryService with BondInquiryService
	auto BondInqServConn = BondInquiryConnector::create_connector();
	auto BondInqServ = BondInqServConn->GetService();
	auto BondHisInqServListener = BondHisInquiryServiceListener::create_listener();
	BondInqServ->AddListener(BondHisInqServListener);
	// output inquiry data
	BondInqServConn->Subscribe();


	system("pause");

};