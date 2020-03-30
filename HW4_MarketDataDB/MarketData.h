#pragma once
#ifndef MarketData_h
#define MarketData_h

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <utility>
#include <tchar.h>
#include "Stock.h"
using namespace std;

class MarketData
{
private:
	int iPort;
	string sHost;
	vector<Stock> stockMap;
public:
	bool parseCommandLine(int argc, char** argv)
	{
		for (int i = 1; i < argc; ++i) {
			if (!std::strcmp(argv[i], "-ip") && i + 1 < argc) {
				sHost = argv[++i];
				continue;
			}
			if (!std::strcmp(argv[i], "-p") && i + 1 < argc) {
				iPort = std::atoi(argv[++i]);
				continue;
			}
			printUsage();
			return false;
		}
		return true;
	}
	//void PrintComError(_com_error& e){}

	void printUsage()
	{
		std::cout << "Usage:" << std::endl
			<< "    Retrieve reference data " << std::endl
			<< "        [-ip        <ipAddress  = localhost>" << std::endl
			<< "        [-p         <tcpPort    = 8194>" << std::endl;
	}
	void PrintStockMap()
	{
		for (vector<Stock>::iterator mlt = stockMap.begin(); mlt != stockMap.end(); mlt++)
		{
			string sSymbol = mlt->getSymbol();
			vector<TradeData> trades = mlt->getTrades();
			for (vector<TradeData>::iterator vlt = trades.begin(); vlt != trades.end(); vlt++)
			{
				cout << "Date: " << vlt->getDate() << "\t" << "Open: " << vlt->getOpen() << "\t" << "Close: " << vlt->getClose() << "\t"
					<< "High: " << vlt->getHigh() << "\t" << "Low: " << vlt->getLow() << "\t" << "Volume: " << vlt->getVolume() << endl;
			}

		}
	}
	void run(int argc, char** argv);
	int UpdateTradeDBTable();
};
#endif
