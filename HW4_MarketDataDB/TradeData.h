#pragma once
#ifndef TradeData_h
#define TradeData_h

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <utility>
#include <tchar.h>

using namespace std;

class TradeData
{
private:
	double dClose;
	double dHigh;
	double dLow;
	double dOpen;
	long lVolume;
	string sDate;
public:
	double getClose()
	{
		return dClose;
	}
	string getDate()
	{
		return sDate;
	}
	double getHigh()
	{
		return dHigh;
	}
	double getLow()
	{
		return dLow;
	}
	double getOpen()
	{
		return dOpen;
	}
	long getVolume()
	{
		return lVolume;
	}
	TradeData operator=(const TradeData& TradeData_)
	{
		return TradeData(TradeData_);
	}
	TradeData() {};
	TradeData(const TradeData& TradeData_) :dClose(TradeData_.dClose), dHigh(TradeData_.dHigh), dLow(TradeData_.dLow),
		dOpen(TradeData_.dOpen), lVolume(TradeData_.lVolume), sDate(TradeData_.sDate) {};
	TradeData(string sDate_, double dOpen_, double dClose_, double dHigh_, double dLow_, long lVolume_) : dClose(dClose_), dHigh(dHigh_), dLow(dLow_),
		dOpen(dOpen_), lVolume(lVolume_), sDate(sDate_) {};
};
#endif
