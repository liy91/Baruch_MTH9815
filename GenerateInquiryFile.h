//
//  GenerateInquiryFile.h
//  MTH 9815 
//

#ifndef GenerateInquiryFile_h
#define GenerateInquiryFile_h

#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include "GenerateTradeFile.h"

using namespace std;

void generate_inquiry()
{
    cout << "Generating inquiry data." << endl;
    
    ofstream inquiryFile;
    inquiryFile.open("input/inquiries.txt");

    // add header of the inquiries data
    inquiryFile << "product, side, quantity, price, state" << endl;
    
    // create inquiries of 6 securities
    for (int i = 0; i < 6; i++)
    {
        string cusip = cusips[i];
        
        // 10 inquiries for each security
        for (int j = 0; j < 10; j++)
        {
            // set price default as 100
            string price = "100";
            
            // quantity of the inquiry
            int quantity = 100 * (rand() % 9 + 1);
            
            // side of the inquiry
            string side;
            if (rand() % 2 == 0) { side = "BUY"; }
            else {side = "SELL"; }
            
            // set state default as received
            string state = "RECEIVED";
            
            // output 
            inquiryFile << cusip << "," << side << "," << quantity << "," << price << "," << state << std::endl;
        }
    }
    
    inquiryFile.close();
    cout << "Generating inquiries.txt file is done." << endl;
}

#endif /* GenerateInquiryFile_h */
