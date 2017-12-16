//
//  GenerateTradeFile.h
//  MTH 9815 Final
//
//  Created by Yang on 12/3/17.
//  Copyright © 2017 Yang. All rights reserved.
//

#ifndef GenerateTradeFile_h
#define GenerateTradeFile_h

#include <iostream>
#include <fstream>
#include <string>
#include <random>

using namespace std;

// US Treasury CUSIP
std::string cusips [] = {"9128283H1", // 2yr
                     "9128283G3", // 3yr
                     "912828M80", // 5yr
                     "9128283J7", // 7yr
                     "9128283F5", // 10yr
                     "912810RZ3"}; // 30yr


// create trades.txt file,
// with attributes of product, tradeId, book, quantity, and side
void generate_trades ()
{
    cout << "Generating trade data." << endl;
    
    fstream tradesFile;
    tradesFile.open("trades.txt");
    
    // add header of the trades
    tradesFile << "product, tradeId, book, price, quantity, side" << endl;
    
    // create trades of 6 securities
    for (int i = 0; i < 6; ++i)
    {
        string cusip = cusips[i];
        
        // 10 trades for each security
        for (int j = 0; j < 10; ++j)
        {
            // tradeId
            int tradeid = i * 1000 + j * 30;
            
            // book
            int book = rand() % 3 + 1;

			// price
			int num = rand() % (256 * 2 + 1);
			int num1 = num / 256; 
			int num2 = num % 256;
			int num3 = num2 / 8; 
			int num4 = num2 % 8;
			string str1 = std::to_string(99 + num1) + "-";
			string str2 = std::to_string(num3);
			string str3 = std::to_string(num4);
			if (num4 == 4)	str3 = "+";
			if (num3 < 10) str2 = "0" + str2;
            
            // quantity, 1 million to 5 million
            int quantity = 1000000 * (rand() % 5 + 1);
            
            // side
            string side;
            if (rand() % 2 == 0) {side = "SELL";}
            else {side = "BUY";}
            
            // output data
            tradesFile << cusip << ",T" << tradeid << ",TRSY" << book << "," << str1 + str2 + str3 << "," << quantity << "," << side << endl;
        }
    }
    
    tradesFile.close();
    cout << "Generating trades.txt file is done." << endl;
}

#endif /* GenerateTradeFile_h */
