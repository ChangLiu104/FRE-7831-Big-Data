#pragma once
#include <string>
#include <iostream>
#include <vector>

using namespace std;

class TradeData
{
private:
	string sDate;
	double dOpen;
	double dHigh;
	double dLow;
	double dClose;
	double dAdjClose;
	long lVolume;
public:
	TradeData() : sDate(""), dOpen(0), dClose(0), dHigh(0), dLow(0), dAdjClose(0), lVolume(0) {}
	TradeData(string sDate_, double dOpen_, double dHigh_, double dLow_, double dClose_, double dAdjClose_, long lVolume_) :
		sDate(sDate_), dOpen(dOpen_), dHigh(dHigh_), dLow(dLow_), dClose(dClose_), dAdjClose(dAdjClose_), lVolume(lVolume_) {}
	TradeData(const TradeData & TradeData) :sDate(TradeData.sDate), dOpen(TradeData.dOpen),
		dHigh(TradeData.dHigh), dLow(TradeData.dLow), dClose(TradeData.dClose), dAdjClose(TradeData.dAdjClose), lVolume(TradeData.lVolume) {}
	TradeData operator=(const TradeData & TradeData)
	{
		sDate = TradeData.sDate;
		dOpen = TradeData.dOpen;
		dHigh = TradeData.dHigh;
		dLow = TradeData.dLow;
		dClose = TradeData.dClose;
		dAdjClose = TradeData.dAdjClose;
		lVolume = TradeData.lVolume;

		return *this;
	}
	string getDate() { return sDate; }
	double getOpen() { return dOpen; }
	double getHigh() { return dHigh; }
	double getLow() { return dLow; }
	double getClose() { return dClose; }
	double getAdjClose() { return dAdjClose; }
	long getVolume() { return lVolume; }
	friend ostream & operator<<(ostream & ostr, const TradeData & TradeData)
	{
		ostr << TradeData.sDate << " " << TradeData.dOpen << " " << TradeData.dHigh << " " 
			 << TradeData.dLow << " " << TradeData.dClose << " " << TradeData.dAdjClose << " "
			 << TradeData.lVolume << endl;
		return ostr;
	}
};

class Stock
{
private:
	string sSymbol;
	vector<TradeData> trades;

public:
	Stock() : sSymbol("") {}
	Stock(string sSymbol_, const vector<TradeData> trades_) :sSymbol(sSymbol_), trades(trades_) {}
	Stock(const Stock & stock) :sSymbol(stock.sSymbol), trades(stock.trades) {}
	Stock operator=(const Stock & stock)
	{
		sSymbol = stock.sSymbol;
		trades = stock.trades;

		return *this;
	}

	void addTrade(const TradeData & trade) { trades.push_back(trade); }
	string getSymbol() { return sSymbol; }
	const vector<TradeData> & getTrades() const { return trades; }

	friend ostream & operator<<(ostream & ostr, const Stock & stock)
	{
		ostr << "Symbol: " << stock.sSymbol << endl;
		for (vector<TradeData>::const_iterator itr = stock.trades.begin(); itr != stock.trades.end(); itr++)
			ostr << *itr;
		return ostr;
	}
};

class Trade
{
private:
	int iPairID;
	string sTicker1;
	string sTicker2;
	float fTicker1Open;
	float fTicker1Close;
	float fTicker2Open;
	float fTicker2Close;
	int iQty1;
	int iQty2;
	string sDate;
	float fProfit;
public:
	Trade(){}
	~Trade() {}
	Trade(int id, string pTicker1, string pTicker2, float open1, float close1, float open2, float close2, int qty1, int qty2, string pDate, float profit) :
		iPairID(id), sTicker1(pTicker1), sTicker2(pTicker2), fTicker1Open(open1), fTicker1Close(close1), fTicker2Open(open2), fTicker2Close(close2), iQty1(qty1), iQty2(qty2), sDate(pDate), fProfit(profit) {}
	int iGetPairID() { return iPairID; }
	void SetPairID(int id) { iPairID = id; }
	string sGetTicker1() { return sTicker1; }
	void SetTicker1(string Ticker1) { sTicker1 = Ticker1; }
	string sGetTicker2() { return sTicker2; }
	void SetTicker2(string Ticker2) { sTicker2 = Ticker2; }
	string sGetDate() { return sDate; }
	void SetDate(string Date) { sDate = Date; }
	float fGetTicker1Open() { return fTicker1Open; }
	float fGetTicker1Close() { return fTicker1Close; }
	float fGetTicker2Open() { return fTicker2Open; }
	float fGetTicker2Close() { return fTicker2Close; }
	int iGetQty1() { return iQty1; }
	void SetQty1(int qty1) { iQty1 = qty1; }
	int iGetQty2() { return iQty2; }
	void SetQty2(int qty2) { iQty2 = qty2; }
	float fGetProfit() { return fProfit; }
	void SetProfit(float profit) { fProfit = profit; }
	friend ostream & operator<<(ostream &out, const Trade &aTrade)
	{
		out << aTrade.iPairID << " " << aTrade.sDate << " " << aTrade.sTicker1 << " " << aTrade.sTicker2 << " "
			<< aTrade.fTicker1Open << " " << aTrade.fTicker1Close << " " << aTrade.fTicker2Open << " " << aTrade.fTicker2Close << " "
			<< aTrade.iQty1 << " " << aTrade.iQty2 << " " << aTrade.fProfit << endl;
		return out;
	}
};


class PairPriceStd
{
private:
	string sSymbol1;
	string sSymbol2;
	double dVol;
	int k;
public:
	PairPriceStd() {}
	~PairPriceStd() {}
	PairPriceStd(string sSymbol1_, string sSymbol2_, double dVol_, int k_) :sSymbol1(sSymbol1_), sSymbol2(sSymbol2_), dVol(dVol_), k(k_) {}
	string sGetSymbol1() { return sSymbol1; }
	string sGetSymbol2() { return sSymbol2; }
	double dGetVol() { return dVol; }
	int iGetk() { return k; }
};