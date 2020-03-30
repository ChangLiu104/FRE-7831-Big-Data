// MarketDataHW4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <blpapi_session.h>
#include <blpapi_eventdispatcher.h>

#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <blpapi_subscriptionlist.h>
#include <blpapi_defs.h>
#include <blpapi_exception.h>

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <utility>
#include <tchar.h>
#include "MarketData.h"
#include "Stock.h"
#include "TradeData.h"



using namespace std;
using namespace BloombergLP;
using namespace blpapi;
namespace {
	const Name SRCURITY_DATA("securityData");
	const Name SECRITY_NAME("security");
	const Name FIELD_DATA("fieldData");
}

#import <C:\\Program Files\\Common Files\\System\\ado\\msado15.dll> \
rename ("EOF","AdoNSEOF")

_bstr_t bstrConnect = "Provider=MicroSoft.Jet.OLEDB.4.0; Data Source=C:\\Users\\FREUser\\Desktop\\MarketDataHW4\\MarketDataDB.mdb;";

void MarketData::run(int argc, char** argv)
{
	sHost = "localhost";
	iPort = 8194;
	if (!parseCommandLine(argc, argv)) return;

	SessionOptions sessionOptions;
	sessionOptions.setServerHost(sHost.c_str());
	sessionOptions.setServerPort(iPort);

	std::cout << "Connecting to " << sHost << ":" << iPort << std::endl;
	Session session(sessionOptions);
	if (!session.start()) {
		std::cerr << "Failed to start session." << std::endl;
		return;
	}
	if (!session.openService("//blp/refdata")) {
		std::cerr << "Failed to open //blp/refdata" << std::endl;
		return;
	}

	Service refDataService = session.getService("//blp/refdata");
	Request request = refDataService.createRequest("HistoricalDataRequest");
	request.getElement("fields").appendValue("OPEN");
	request.getElement("fields").appendValue("LAST_TRADE");
	request.getElement("fields").appendValue("VOLUME");
	request.getElement("fields").appendValue("HIGH");
	request.getElement("fields").appendValue("LOW");
	request.set("periodicityAdjustment", "ACTUAL");
	request.set("periodicitySelection", "DAILY");
	request.set("startDate", "20190101");
	request.set("endDate", "20191231");
	request.set("maxDataPoints", 2000);

	vector<string> stockList;
	stockList.push_back("GOOG");
	stockList.push_back("IBM");
	stockList.push_back("TWTR");
	stockList.push_back("MSFT");
	
	string sSuffix = " US Equity";
	vector<string>::iterator vitr = stockList.begin();
	for (; vitr != stockList.end(); vitr++)
	{
		string sStock = *vitr + sSuffix;
		request.getElement("securities").appendValue(sStock.c_str());
	}
	std::cout << "Sending Request: " << request << std::endl;
	session.sendRequest(request);

	while (true) {
		Event event = session.nextEvent();
		MessageIterator msgIter(event);
		string sSymbol = "";
		vector<TradeData> trades;
		while (msgIter.next()) {
			Message msg = msgIter.message();
			if ((event.eventType() != Event::PARTIAL_RESPONSE) && (event.eventType() != Event::RESPONSE))
				continue;
			Element securityData = msg.getElement(SRCURITY_DATA);
			sSymbol = securityData.getElement("security").getValueAsString();
			std::size_t found = sSymbol.find(" ");
			if (found != std::string::npos)
				sSymbol = sSymbol.substr(0, found);
			Element fieldData = securityData.getElement(FIELD_DATA);
			for (int i = 0; i < int(fieldData.numValues()); i++)
			{
				string sDate = fieldData.getValueAsElement(i).getElement("date").getValueAsString();
				double fClose = fieldData.getValueAsElement(i).getElement("LAST_TRADE").getValueAsFloat64();
				double fOpen = fieldData.getValueAsElement(i).getElement("OPEN").getValueAsFloat64();
				double fHigh = fieldData.getValueAsElement(i).getElement("HIGH").getValueAsFloat64();
				double fLow = fieldData.getValueAsElement(i).getElement("LOW").getValueAsFloat64();
				long lVolume = (long)fieldData.getValueAsElement(i).getElement("VOLUME").getValueAsInt64();
				TradeData aTrade(sDate, fOpen, fClose, fHigh, fLow, lVolume);
				trades.push_back(aTrade);
			}
		}
		if (sSymbol.length() > 0) { stockMap.push_back(Stock(sSymbol, trades)); }
		if (event.eventType() == Event::RESPONSE) {
			break;
		}
	}
}

int MarketData::UpdateTradeDBTable()
{
	HRESULT hResult = CoInitialize(0);
	ADODB::_ConnectionPtr pConnect("ADODB.Connection");
	hResult = pConnect->Open(bstrConnect, "admin", "", ADODB::adConnectUnspecified);
	if (SUCCEEDED(hResult))
	{
		char sQuery[256];
		memset((void*)sQuery, '\0', 256);
		sprintf_s(sQuery, "DELETE * FROM DailyData;");
		ADODB::_RecordsetPtr pRecSet("ADODB.Recordset");
		hResult = pRecSet->Open(sQuery, _variant_t((IDispatch *)pConnect, true), ADODB::adOpenUnspecified,
			ADODB::adLockUnspecified, ADODB::adCmdText);
		memset((void*)sQuery, '\0', 256);
		for (vector<Stock>::iterator mIt = stockMap.begin(); mIt != stockMap.end(); mIt++)
		{
			string sSymbol = mIt->getSymbol();
			vector<TradeData> trades = mIt->getTrades();
			for (vector<TradeData>::iterator vIt = trades.begin(); vIt != trades.end(); vIt++)
			{
				string sDate = vIt->getDate();
				double dOpen = vIt->getOpen();
				double dClose = vIt->getClose();
				double dHigh = vIt->getHigh();
				double dLow = vIt->getLow();
				long lVolume = vIt->getVolume();
				sprintf_s(sQuery, "INSERT INTO DailyData VALUES('%s','%s',%.2f,%.2f,%.2f,%.2f,%ld);", sDate.c_str(), sSymbol.c_str(), dOpen, dClose, dHigh, dLow, lVolume);
				ADODB::_RecordsetPtr pRecSet("ADODB.Recordset");
				hResult = pRecSet->Open(sQuery, _variant_t((IDispatch *)pConnect, true), ADODB::adOpenUnspecified,
					ADODB::adLockUnspecified, ADODB::adCmdText);
			}
		}
		pConnect->Close();

	}
	return 0;
}

int main(int argc, char** argv)
{
	MarketData example;
	example.run(argc, argv);
	cout << "finish pulling data" << endl;
	example.UpdateTradeDBTable();
	cout << "finish storing data" << endl;
	example.PrintStockMap();
	system("Pause");
    return 0;
}

