//
//  GenerateMarketDataFile.h
//  MTH 9815 
//

#ifndef GenerateMarketDataFile_h
#define GenerateMarketDataFile_h

#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include "GenerateTradeFile.h"

using namespace std;

void generate_marketdata()
{
    cout << "Generating market data." << endl;
    
    ofstream MarketDataFile;
    MarketDataFile.open("input/marketdata.txt");
    
    // add header of the market data
    MarketDataFile << "product,"
               << "bid_price1,"
               << "bid_position1,"
               << "bid_price2,"
               << "bid_position2,"
               << "bid_price3,"
               << "bid_position3,"
               << "bid_price4,"
               << "bid_position4,"
               << "bid_price5,"
               << "bid_position5,"
               << "offer_price1,"
               << "offer_position1,"
               << "offer_price2,"
               << "offer_position2,"
               << "offer_price3,"
               << "offer_position3,"
               << "offer_price4,"
               << "offer_position4,"
               << "offer_price5,"
               << "offer_position5,"
    << endl;

    // create market data of 6 securities
    for (int i = 0; i < 6; i++)
    {
        string cusip = cusips[i];
        
        // 1,000,000 order book updates for each security
        for (int j = 0; j < 100; j++)
        {
            MarketDataFile << cusip;
            
            // generate an integer between 0 and 256 * 2
            int spd = rand() % (256 * 2 +1);
            
            // generate bid prices
            int bid_spd = spd - 1;
            
            // 5 orders deep on bid stacks
            for (int k1 = 0; k1 < 5; ++k1)
            {
                int bid_quantity = 10000000 * (k1 + 1);
                
                int bdigit1 = bid_spd / 256;
                int bdigit2 = bid_spd % 256;
                int bdigit3 = bdigit2 / 8; // integer between 0 and 31
                int bdigit4 = bdigit2 % 8; // integer between 0 and 7

                int bint = 99 + bdigit1;
                string bint_s = to_string(bint) + "-";
                
                string bxy_s;
                if (bdigit3 < 10) { bxy_s = "0" + to_string(bdigit3); }
                else { bxy_s = to_string(bdigit3); }
                
                string bz_s;
                if (bdigit4 == 4) { bz_s = "+"; }
                else { bz_s = to_string(bdigit4); }
               
                // create bid price string
                string bid_price = bint_s + bxy_s + bz_s;
                MarketDataFile << "," << bid_price << "," << bid_quantity;
                
                // increase bid spread
                --bid_spd;
            }
            //MarketDataFile << endl;
            
            // generate offer prices
            int offer_spd = spd + 1;
            
            // 5 orders deep on offer stacks
            for (int k2 = 0; k2 < 5; ++k2)
            {
                int offer_quantity = 10000000 * (k2 + 1);
                
                int odigit1 = offer_spd / 256;
                int odigit2 = offer_spd % 256;
                int odigit3 = odigit2 / 8; // integer between 0 and 31
                int odigit4 = odigit2 % 8; // integer between 0 and 7

                int oint = 99 + odigit1;
                string oint_s = to_string(oint) + "-";
                
                string oxy_s;
                if (odigit3 < 10) { oxy_s = "0" + to_string(odigit3); }
                else { oxy_s = to_string(odigit3); }
                
                string oz_s;
                if (odigit4 == 4) { oz_s = "+"; }
                else { oz_s = to_string(odigit4); }
                
                // create offer price string
                string offer_price = oint_s + oxy_s + oz_s;
                MarketDataFile << "," << offer_price << "," << offer_quantity;
                
                // increase offer spread
                ++offer_spd;
            }
            MarketDataFile << endl;
        }
    }
    
    MarketDataFile.close();
    cout << "Generating marketdata.txt file is done." << endl;
}

#endif /* GenerateMarketDataFile_h */
