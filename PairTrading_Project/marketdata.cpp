//
//  marketdata.cpp
//

#include "marketdata.hpp"
#include "database.hpp"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

int RetrieveMarketData(string url_request, string & read_buffer)
{
	curl_global_init(CURL_GLOBAL_ALL);
	CURL * myHandle;
	CURLcode result;
	myHandle = curl_easy_init();

	curl_easy_setopt(myHandle, CURLOPT_URL, url_request.c_str());

	//adding a user agent
	curl_easy_setopt(myHandle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 6.1; rv:2.2) Gecko/20110201");
	curl_easy_setopt(myHandle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(myHandle, CURLOPT_SSL_VERIFYHOST, 0);
	//curl_easy_setopt(myHandle, CURLOPT_VERBOSE, 1);

	// send all data to this function  
	curl_easy_setopt(myHandle, CURLOPT_WRITEFUNCTION, WriteCallback);

	// we pass our 'chunk' struct to the callback function 
	curl_easy_setopt(myHandle, CURLOPT_WRITEDATA, &read_buffer);

	//perform a blocking file transfer
	result = curl_easy_perform(myHandle);

	// check for errors 
	if (result != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
		return -1;
	}
	curl_easy_cleanup(myHandle);
	return 0;
}

int PopulatePairTable(sqlite3 *db, const map<int, pair<string, string>> & pairs)
{
    for (map<int, pair<string, string>>::const_iterator itr = pairs.begin(); itr != pairs.end(); itr++)
    {
        char pair_insert_table[512];
        sprintf(pair_insert_table, "INSERT INTO PAIRS (id, symbol1, symbol2, variance, profit) VALUES(%d, \"%s\", \"%s\", %.2f, %.2f)", itr->first, itr->second.first.c_str(), itr->second.second.c_str(), 0.0, 0.0);
        if (InsertTable(pair_insert_table, db) == -1)
            return -1;
    }
    return 0;
}

int PopulatePairStockTable(sqlite3 *db, const map<string, Stock> & stocks)
{
	for (auto stock_itr = stocks.begin(); stock_itr != stocks.end(); ++stock_itr)
	{
		vector<TradeData> trades = stock_itr->second.getTrades();
		for (auto trade_itr = trades.begin(); trade_itr != trades.end(); ++trade_itr)
		{
			char pair_insert_table[512];
			sprintf(pair_insert_table, "Insert into PairStocks (symbol, date, open, high, low, close, adjusted_close, volume) \
					VALUES(\"%s\", \"%s\", %.2f, %.2f, %.2f, %.2f, %.2f, %ld)",
				stock_itr->first.c_str(), trade_itr->getDate().c_str(), trade_itr->getOpen(), trade_itr->getHigh(), trade_itr->getLow(),
				trade_itr->getClose(), trade_itr->getAdjClose(), trade_itr->getVolume());
			if (InsertTable(pair_insert_table, db) == -1)
				return -1;
		}
	}
	return 0;
}
