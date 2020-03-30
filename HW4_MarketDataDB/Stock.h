#pragma once
#ifndef Stock_h
#define Stock_h
#include <iostream>
#include <string>
#include <vector>
#include "TradeData.h"
using namespace std;

class Stock
{
private:
	string sSymbol;
	vector<TradeData> trades;
public:
	Stock() {}
	Stock(const Stock& stock)
	{
		sSymbol = stock.sSymbol;
		trades = stock.trades;
	}
	Stock(string sSymbol_, vector<TradeData> trades_)
	{
		sSymbol = sSymbol_;
		trades = trades_;
	}
	~Stock()
	{}
	void addTrade(const TradeData& trade)
	{
		trades.push_back(trade);
	}
	string getSymbol()
	{
		return sSymbol;
	}
	const vector<TradeData>& getTrades()
	{
		return trades;
	}
	Stock operator=(const Stock& stock)
	{
		return Stock(stock);
	}

};



#endif
