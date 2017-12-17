//
//  GeneratePriceFile.h
//  MTH 9815 
//

#ifndef GeneratePriceFile_h
#define GeneratePriceFile_h

#include <iostream>
#include <fstream>
#include <string>
#include <string>
#include "GenerateTradeFile.h"

using namespace std;

/* US Treasury CUSIP
string cusips [6] = {"9128283H1", // 2yr
    "9128283G3", // 3yr
    "912828M80", // 5yr
    "9128283J7", // 7yr
    "9128283F5", // 10yr
    "912810RZ3"}; // 30yr
*/

// create prices.txt file,
// with attributes of product, mid, bid_offer_spread
void generate_prices()
{
    cout << "Generating price data." << endl;
    
    ofstream pricesFile;
    pricesFile.open("input/prices.txt");
    
    // add header of the prices
    pricesFile << "product, mid, bid_offer_spread" << endl;
    
    // create prices of 6 securities
    for (int i = 0; i < 6; i++)
    {
        string cusip = cusips[i];
        
        // 10 trades for each security
        for (int j = 0; j < 10; j++)
        {
            // generate an integer between 99 and 101
            int integer = rand() % 3 + 99;
            string int_s = to_string(integer) + "-";
            
            // generate an integer between 0 and 31
            int xy = rand() % 32;
            string xy_s;
            if (xy < 10) { xy_s = "0" + to_string(xy); }
            else { xy_s = to_string(xy); }
            
            // generate an integer between 0 and 7
            int z = rand() % 8;
            string z_s;
            if (z == 4) { z_s = "+"; }
            else { z_s = to_string(z); }
    
            // create mid price string
            string mid = int_s + xy_s + z_s;
            
            // bid offer spread oscillate between 1/128 and 1/64
            // the US treasuries trade in 1/256 increments
            // generate an integer between 2 and 4
            int sp = rand() % 3 + 2;
            string sp_s;
            if (sp == 4) { sp_s = "0-00+"; }
            else { sp_s = "0-00" + to_string(sp); }
            
            // output data
            pricesFile << cusip << "," << mid << "," << sp_s << endl;
        }
    }
    
    pricesFile.close();
    cout << "Generating prices.txt file is done." << endl;
}

#endif /* GeneratePriceFile_h */
