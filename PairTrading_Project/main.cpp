#define _HAS_STD_BYTE 0
#include "PairTrading.h"
#include "database.hpp"
#include "marketdata.hpp"
#include <iostream>
#include <istream>
#include <fstream>
#include <set>
#include <string>
#include <math.h>
using namespace std;


string back_testing_start_date = "2020-01-02";
string back_testing_end_date = "2020-01-31";

map<int, pair<string, string>>  stockPairs;
set<string> pairSymbols;

map<pair<string, string>, PairPriceStd> PairPriceStdTable;
map<pair<string, string>, vector<Trade>> TradeTable;

int RetrievePairPriceStdTable(const char *sql_select, sqlite3 *db, int k = 1)
{
	// populate PairPriceStdTable map
	int rc = 0;
	char *error = NULL;
	cout << "Retrieving values in PairPriceStd table ..." << endl;
	char **results = NULL;
	int rows, columns;
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}
	// out of range?
	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		string symbol1 = results[rowCtr * columns + 1];
		string symbol2 = results[rowCtr * columns + 2];
		double vol = atof(results[rowCtr * columns + 3]);
		pair<string, string> aKey = make_pair(symbol1, symbol2);
		PairPriceStdTable[aKey] = PairPriceStd(symbol1, symbol2, vol, k);
	}
	sqlite3_free_table(results);
	return 0;
}


int RetrieveTradeTable(const char *sql_select, sqlite3 *db)
{
	// populate TradeTable map
	int rc = 0;
	char *error = NULL;
	cout << "Retrieving values in Trades table ..." << endl;
	char **results = NULL;
	int rows, columns;
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}

	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		int id = atoi(results[rowCtr * columns]);
		string symbol1 = results[rowCtr * columns + 1];
		string symbol2 = results[rowCtr * columns + 2];
		string date = results[rowCtr * columns + 3];
		double open1= atof(results[rowCtr * columns + 4]);
		double close1 = atof(results[rowCtr * columns + 5]);
		double open2 = atof(results[rowCtr * columns + 6]);
		double close2 = atof(results[rowCtr * columns + 7]);
		int qty1 = atoi(results[rowCtr * columns + 8]);
		int qty2 = atoi(results[rowCtr * columns + 9]);
		double profit_loss = atof(results[rowCtr * columns + 10]);

		pair<string, string> aKey = make_pair(symbol1, symbol2);
		TradeTable[aKey].push_back(Trade(id, symbol1, symbol2, open1, close1, open2, close2, qty1, qty2, date, profit_loss));
	}
	sqlite3_free_table(results);
	return 0;
}

static int callback(void *data, int argc, char **argv, char **azColName) 
{
	int i;
	fprintf(stderr, "%s: ", (const char*)data);

	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

void UpdateTradeTable(const char *name, sqlite3 *db)
{
	// implement trading logic
	// update TradeTable and Trades
	char *zErrMsg = 0;
	int rc;
	char *sql_update;
	const char* data = "Callback function called";

	OpenDatabase(name, db);

	for (map<pair<string, string>, vector<Trade>>::iterator mlt = TradeTable.begin(); mlt != TradeTable.end(); mlt++)
	{
		for (vector<Trade>::iterator vlt = mlt->second.begin(); vlt != mlt->second.end()-1; vlt++)
		{
			double vol = pow(PairPriceStdTable[mlt->first].dGetVol(), 0.5);
			int k = PairPriceStdTable[mlt->first].iGetk();
			double spread = abs(vlt->fGetTicker1Close() / vlt->fGetTicker2Close() - next(vlt)->fGetTicker1Open() / next(vlt)->fGetTicker2Open());
			int direction = 0;
			if (spread > (k * vol))
			{
				// short the pair
				direction = -1;
			}
			if (spread < (k * vol))
			{
				// long the pair
				direction = 1;
			}
			int qty1 = direction * 10000;
			int qty2 = (int)(-1) * direction * 10000 * (next(vlt)->fGetTicker1Open() / next(vlt)->fGetTicker2Open());
			double profit = qty1 * (next(vlt)->fGetTicker1Close() - next(vlt)->fGetTicker1Open()) + qty2 * (next(vlt)->fGetTicker2Close() - next(vlt)->fGetTicker2Open());
			next(vlt)->SetQty1(qty1);
			next(vlt)->SetQty2(qty2);
			next(vlt)->SetProfit(profit);

			string ticker1 = next(vlt)->sGetTicker1();
			string ticker2 = next(vlt)->sGetTicker2();
			string date = next(vlt)->sGetDate();

			string sql_update = "UPDATE Trades SET qty1 = '" + to_string(qty1) + "', qty2 = '" + to_string(qty2) + "', profit = '" + to_string(profit)
				+ "' WHERE (((symbol1 = '" + ticker1 + "') AND (symbol2 = '" + ticker2 + "')) AND (date = '" + date + "'));";
			rc = sqlite3_exec(db, sql_update.c_str(), callback, (void*)data, &zErrMsg);
			if (rc != SQLITE_OK)
			{
				fprintf(stderr, "SQL error: %s\n", zErrMsg);
				sqlite3_free(zErrMsg);
			}
			else
			{
				fprintf(stdout, "Operation done successfully\n");
			}
		}
	}
}

// Pairs
int RetrievePairSymbols(sqlite3 *db)
{
	// retrieve pair symbols
	cout << "Retrieve PAIRS symbol and save to database..." << endl;
	string pairs_drop_table = "DROP TABLE IF EXISTS Pairs;";
	if (DropTable(pairs_drop_table.c_str(), db) == -1)
		return -1;

	string pairs_create_table = "CREATE TABLE Pairs (id INT PRIMARY KEY NOT NULL, symbol1 CHAR(20) NOT NULL, symbol2 CHAR(20) NOT NULL,\
                                                     variance REAL NOT NULL, profit REAL NOT NULL);";
	if (CreateTable(pairs_create_table.c_str(), db) == -1)
		return -1;

	ifstream file("PairTrading.txt");
	string line;
	int index = 1;
	while (getline(file, line)) {
		stringstream ss(line);
		string ticker1, ticker2;
		getline(ss, ticker1, ',');
		getline(ss, ticker2);
		stockPairs[index] = make_pair(ticker1, ticker2);
		pairSymbols.insert(ticker1);
		pairSymbols.insert(ticker2);
		index++;
	}
	file.close();

	if (PopulatePairTable(db, stockPairs) == -1)
		return -1;

	string pairs_select_table = "SELECT * FROM Pairs;";
	if (DisplayTable(pairs_select_table.c_str(), db) == -1)
		return -1;
	cout << "Retrieve successfully! Thank you for your patience!" << endl;
}

int RetrievePairPrices(sqlite3 *db)
{
	// retrieve historical prices
	map<string, Stock> stocks_;
	for (set<string>::const_iterator itr = pairSymbols.begin(); itr != pairSymbols.end(); itr++)
	{
		string readBuffer;
		string stock_url_common = "https://eodhistoricaldata.com/api/eod/";
		string stock_start_date = "2019-01-02";
		string stock_end_date = "2020-01-31";
		string api_token = "5ba84ea974ab42.45160048";
		string pair_stock_retrieve_url = stock_url_common + *itr + ".US?" +
			"from=" + stock_start_date + "&to=" + stock_end_date + "&api_token=" + api_token + "&period=d";
		if (RetrieveMarketData(pair_stock_retrieve_url, readBuffer) == -1)
			return -1;

		string sSymbol_ = *itr;
		vector<TradeData> trades_;

		size_t lastNL = readBuffer.rfind('\n');
		readBuffer.erase(lastNL);
		stringstream sData(readBuffer);
		string line;
		getline(sData, line);
		while (getline(sData, line))
		{
			int n = line.length();
			char * char_arr = new char[n + 1];
			strcpy(char_arr, line.c_str());
			char * p;
			uint8_t j;
			string row[7];
			for (j = 0, p = strtok(char_arr, ","); p != NULL; p = strtok(NULL, ","), j++)
			{
				row[j] = p;
			}
			TradeData TradeData_(row[0], atof(row[1].c_str()), atof(row[2].c_str()), atof(row[3].c_str()),
				atof(row[4].c_str()), atof(row[5].c_str()), atol(row[6].c_str()));
			trades_.push_back(TradeData_);
		}
		Stock Stock_(sSymbol_, trades_);
		stocks_[*itr] = Stock_;

		cout << readBuffer << endl;
	}

	string pair_stocks_drop_table = "DROP TABLE IF EXISTS PairStocks;";
	if (DropTable(pair_stocks_drop_table.c_str(), db) == -1)
		return -1;

	string pair_stocks_create_table = "CREATE TABLE PairStocks (symbol CHAR(20) NOT NULL,date CHAR(20) NOT NULL,\
                                               open REAL NOT NULL,high REAL NOT NULL,low REAL NOT NULL,close REAL NOT NULL,\
                                               adjusted_close REAL NOT NULL,volume INT NOT NULL,PRIMARY KEY(symbol, date));";
	if (CreateTable(pair_stocks_create_table.c_str(), db) == -1)
		return -1;

	if (PopulatePairStockTable(db, stocks_) == -1)
		return -1;
	cout << "Retrieve successfully! Thank you for your patience!" << endl << endl;
}

// Pair1Stocks
int CreatePair1StocksTable(sqlite3 *db)
{
	string pair1_stocks_drop_table = "DROP TABLE IF EXISTS Pair1Stocks;";
	if (DropTable(pair1_stocks_drop_table.c_str(), db) == -1)
		return -1;
	string pair1_stocks_create_table = "CREATE TABLE Pair1Stocks (symbol CHAR(20) NOT NULL,date CHAR(20) NOT NULL,\
                                               open REAL NOT NULL,high REAL NOT NULL,low REAL NOT NULL,close REAL NOT NULL,\
                                               adjusted_close REAL NOT NULL,volume INT NOT NULL,PRIMARY KEY(symbol, date));";
	if (CreateTable(pair1_stocks_create_table.c_str(), db) == -1)
		return -1;
	string pair1_stocks_insert_table = string("Insert into Pair1Stocks ")
		+ "Select Distinct Pairs.symbol1 as symbol, PairStocks.date as date, PairStocks.open as open, "
		+ "PairStocks.high as high, PairStocks.low as low, PairStocks.close as close, "
		+ "PairStocks.adjusted_close as adjusted_close, PairStocks.volume as volume "
		+ "From Pairs, PairStocks "
		+ "Where Pairs.symbol1 = PairStocks.symbol ORDER BY symbol;";
	if (InsertTable(pair1_stocks_insert_table.c_str(), db) == -1)
		return -1;
	cout << "Insert Pair1Stocks table successfully!" << endl;
}

// Pair2Stocks
int CreatePair2StocksTable(sqlite3 *db)
{
	string pair2_stocks_drop_table = "DROP TABLE IF EXISTS Pair2Stocks;";
	if (DropTable(pair2_stocks_drop_table.c_str(), db) == -1)
		return -1;
	string pair2_stocks_create_table = "CREATE TABLE Pair2Stocks (symbol CHAR(20) NOT NULL,date CHAR(20) NOT NULL,\
                                               open REAL NOT NULL,high REAL NOT NULL,low REAL NOT NULL,close REAL NOT NULL,\
                                               adjusted_close REAL NOT NULL,volume INT NOT NULL,PRIMARY KEY(symbol, date));";
	if (CreateTable(pair2_stocks_create_table.c_str(), db) == -1)
		return -1;
	string pair2_stocks_insert_table = string("Insert into Pair2Stocks ")
		+ "Select Distinct Pairs.symbol2 as symbol, PairStocks.date as date, PairStocks.open as open, "
		+ "PairStocks.high as high, PairStocks.low as low, PairStocks.close as close, "
		+ "PairStocks.adjusted_close as adjusted_close, PairStocks.volume as volume "
		+ "From Pairs, PairStocks "
		+ "Where Pairs.symbol2 = PairStocks.symbol ORDER BY symbol;";
	if (InsertTable(pair2_stocks_insert_table.c_str(), db) == -1)
		return -1;
	cout << "Insert Pair2Stocks table successfully!" << endl;
}

// PairPrices
int CreatePairPricesTable(sqlite3 *db)
{
	string pair_prices_drop_table = "DROP TABLE IF EXISTS PairPrices;";
	if (DropTable(pair_prices_drop_table.c_str(), db) == -1)
		return -1;
	string pair_prices_create_table = "CREATE TABLE PairPrices (id INT NOT NULL, symbol1 CHAR(20) NOT NULL, symbol2 CHAR(20) NOT NULL,\
				                               date CHAR(20) NOT NULL, open1 REAL NOT NULL, close1 REAL NOT NULL,\
                                               open2 REAL NOT NULL, close2 REAL NOT NULL, PRIMARY KEY(symbol1, symbol2, date),\
                                               FOREIGN KEY(symbol1) REFERENCES Pair1Stocks(symbol), FOREIGN KEY(symbol2) REFERENCES Pair2Stocks(symbol));";
	if (CreateTable(pair_prices_create_table.c_str(), db) == -1)
		return -1;
	string pair_prices_insert_table = string("Insert into PairPrices ")
		+ "Select Pairs.id as id, Pairs.symbol1 as symbol1, Pairs.symbol2 as symbol2, "
		+ "Pair1Stocks.date as date, Pair1Stocks.open as open1, Pair1Stocks.adjusted_close as close1, "
		+ "Pair2Stocks.open as open2, Pair2Stocks.adjusted_close as close2 "
		+ "From Pairs, Pair1Stocks, Pair2Stocks "
		+ "Where (((Pairs.symbol1 = Pair1Stocks.symbol) and (Pairs.symbol2 = Pair2Stocks.symbol)) and "
		+ "(Pair1Stocks.date = Pair2Stocks.date)) ORDER BY symbol1, symbol2;";
	if (InsertTable(pair_prices_insert_table.c_str(), db) == -1)
		return -1;
	cout << "Insert PairPrices table successfully!" << endl;
}

int CreatePairPriceRatioTable(sqlite3 *db)
{
	string pair_price_ratio_drop_table = "DROP TABLE IF EXISTS PairPriceRatio;";
	if (DropTable(pair_price_ratio_drop_table.c_str(), db) == -1)
		return -1;
	string pair_price_ratio_create_table = "CREATE TABLE PairPriceRatio (id INT PRIMARY KEY NOT NULL, symbol1 CHAR(20) NOT NULL,\
                                                    symbol2 CHAR(20) NOT NULL, priceratio REAL NOT NULL);";
	if (CreateTable(pair_price_ratio_create_table.c_str(), db) == -1)
		return -1;
	string pair_price_ratio_insert_table = string("Insert into PairPriceRatio ")
		+ "Select Pairs.id as id, Pairs.symbol1 as symbol1, Pairs.symbol2 as symbol2, "
		+ "(AVG((Pair1Stocks.adjusted_close/Pair2Stocks.adjusted_close)*(Pair1Stocks.adjusted_close/Pair2Stocks.adjusted_close)) - AVG(Pair1Stocks.adjusted_close/Pair2Stocks.adjusted_close)*AVG(Pair1Stocks.adjusted_close/Pair2Stocks.adjusted_close)) as priceratio "
		+ "From Pairs, Pair1Stocks, Pair2Stocks "
		+ "Where ((((Pairs.symbol1 = Pair1Stocks.symbol) and (Pairs.symbol2 = Pair2Stocks.symbol)) and (Pair1Stocks.date < " + "\"" + back_testing_start_date + "\"" + "))"
		+ "and (Pair1Stocks.date = Pair2Stocks.date)) GROUP BY Pairs.id, Pairs.symbol1, Pairs.symbol2 "
		+ "ORDER BY Pairs.id, Pairs.symbol1, Pairs.symbol2;";
	if (InsertTable(pair_price_ratio_insert_table.c_str(), db) == -1)
		return -1;
	cout << "Insert PairPriceRatio table successfully!" << endl;

	char *zErrMsg = 0;
	int rc;
	const char *data = "Callback function called";

	string update_variance = "UPDATE Pairs SET variance = (SELECT priceratio FROM PairPriceRatio WHERE (PairPriceRatio.id = Pairs.id));";

	rc = sqlite3_exec(db, update_variance.c_str(), callback, (void*)data, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Operation done successfully\n");
	}
}

// Trades
int CreateTradesTable(sqlite3 *db)
{
	string drop_trades_table = "DROP TABLE IF EXISTS Trades;";
	if (DropTable(drop_trades_table.c_str(), db) == -1)
		return -1;
	string create_trades_table = "CREATE TABLE Trades (id INT NOT NULL, symbol1 CHAR(20) NOT NULL, symbol2 CHAR(20) NOT NULL, date CHAR(20) NOT NULL,\
                                   open1 REAL NOT NULL, close1 REAL NOT NULL, open2 REAL NOT NULL, close2 REAL NOT NULL,\
                                   qty1 INT NOT NULL, qty2 INT NOT NULL, profit REAL NOT NULL,\
                                   PRIMARY KEY(symbol1, symbol2, date), FOREIGN KEY(symbol1) REFERENCES Pair1Stocks(symbol),\
                                   FOREIGN KEY(symbol2) REFERENCES Pair2Stocks(symbol));";
	if (CreateTable(create_trades_table.c_str(), db) == -1)
		return -1;
	string trades_insert_table = string("Insert Into Trades ")
		+ "Select Pairs.id, Pairs.symbol1, Pairs.symbol2, Pair1Stocks.date as date, "
		+ "Pair1Stocks.open as open1, Pair1Stocks.close as close1, Pair2Stocks.open as open2, Pair2Stocks.close as close2, 0, 0, 0 "
		+ "From Pairs, Pair1Stocks, Pair2Stocks Where ((((Pairs.symbol1=Pair1Stocks.symbol) and (Pairs.symbol2=Pair2Stocks.symbol)) and (Pair1Stocks.date=Pair2Stocks.date)) and (Pair1Stocks.date >= " + "\"" + back_testing_start_date + "\"" + ") and (Pair1Stocks.date <= " + "\"" + back_testing_end_date + "\"" + ")) "
		+ "ORDER BY Pairs.id, Pairs.symbol1, Pairs.symbol2;";
	if (InsertTable(trades_insert_table.c_str(), db) == -1)
		return -1;
	cout << "Insert Trades table successfully!" << endl;
}

int CreateQueryProfitableTradesTable(sqlite3 *db)
{
	string drop_query_profitable_trades_table = "DROP TABLE IF EXISTS QueryProfitableTrades;";
	if (DropTable(drop_query_profitable_trades_table.c_str(), db) == -1)
		return -1;
	string create_query_profitable_trades_table = "CREATE TABLE QueryProfitableTrades (id INT NOT NULL, symbol1 CHAR(20) NOT NULL, symbol2 CHAR(20) NOT NULL,\
		                                           TotalProfit REAL NOT NULL, ProfitTrades INT NOT NULL, LossTrades INT NOT NULL, TotalTrades INT NOT NULL, ProfitableRatio FLOAT NOT NULL,\
                                                   PRIMARY KEY(symbol1, symbol2), FOREIGN KEY(symbol1) REFERENCES Pairs(symbol1), FOREIGN KEY(symbol2) REFERENCES Pairs(symbol2));";
	if (CreateTable(create_query_profitable_trades_table.c_str(), db) == -1)
		return -1;
	string query_profitable_trades_insert_table = string("Insert Into QueryProfitableTrades ")
		+ "Select Pairs.id, Pairs.symbol1, Pairs.symbol2, SUM(Trades.profit) AS TotalProfit, SUM(CASE WHEN Trades.profit>0 THEN 1 ELSE 0 END) AS ProfitTrades, "
		+ "SUM(CASE WHEN Trades.profit<0 THEN 1 ELSE 0 END) AS LossTrades, COUNT(Trades.profit) AS TotalTrades, "
		+ "(SUM(CASE WHEN Trades.profit>0 THEN 1 ELSE 0 END)*1.0/SUM(CASE WHEN Trades.profit<0 THEN 1 ELSE 0 END)) AS ProfitableRatio "
		+ "From Pairs, Trades Where (Pairs.id = Trades.id) GROUP BY Pairs.id, Pairs.symbol1, Pairs.symbol2;";
	if (InsertTable(query_profitable_trades_insert_table.c_str(), db) == -1)
		return -1;
	cout << "Insert QueryProfitableTrades table successfully!" << endl;

	char *zErrMsg = 0;
	int rc;
	const char *data = "Callback function called";

	string update_profit = "UPDATE Pairs SET profit = (SELECT TotalProfit FROM QueryProfitableTrades WHERE (QueryProfitableTrades.id = Pairs.id));";

	rc = sqlite3_exec(db, update_profit.c_str(), callback, (void*)data, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Operation done successfully\n");
	}
}

void EnterPairTrade(sqlite3 *db)
{
	cout << "Enter a pair Trade:" << endl;

	string symbol1, symbol2;
	float close1, close2, open1, open2, pre_close1, pre_close2;
	int k;
	cout << "Enter symbol 1: ";
	cin >> symbol1;
	cout << endl;
	cout << "Enter symbol 2: ";
	cin >> symbol2;
	cout << endl;
	cout << "Enter symbol1 close: ";
	cin >> close1;
	cout << endl;
	cout << "Enter symbol2 close: ";
	cin >> close2;
	cout << endl;
	cout << "Enter symbol1 open: ";
	cin >> open1;
	cout << endl;
	cout << "Enter symbol2 open: ";
	cin >> open2;
	cout << endl;
	cout << "Enter symbol1 previous close: ";
	cin >> pre_close1;
	cout << endl;
	cout << "Enter symbol2 previous close: ";
	cin >> pre_close2;
	cout << endl;
	cout << "Enter k: ";
	cin >> k;
	cout << endl;

	string select_st = "Select priceratio from PairPriceRatio where symbol1 = \"" + symbol1 + "\" and symbol2 = \"" + symbol2 + "\";";

	float variance = 0;

	int rc = 0;
	char *error = NULL;
	char **results = NULL;
	int rows, columns;
	sqlite3_get_table(db, select_st.c_str(), &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		system("pause");
	}

	variance = atof(results[1]);

	sqlite3_free_table(results);


	if (abs(pre_close1 / pre_close2 - open1 / open2) > k * sqrt(variance))
	{

		int vol1 = -10000;
		int vol2 = int(10000 * (open1 / open2));
	}
	else
	{
		int vol1 = 10000;
		int vol2 = int(-10000 * (open1 / open2));
		float profit_loss = round(vol1 * (open1 - close1) + vol2 * (open2 - close2));
		cout << symbol1 << " " << symbol2 << endl;
		cout << "delta = " << sqrt(variance) << " " << "k = " << k << endl;
		cout << "vol1 = " << vol1 << " " << "vol2 = " << vol2 << " " << "profit_loss = " << profit_loss << endl;
	}

}

int main(void)
{
	const char * stockDB_name = "myPairTrading.db";
	sqlite3 * stockDB = NULL;

	bool a = true;
	int attempt = 0;

	while (a)
	{
		string str;
		cout << endl;
		cout << "Pick a choice from the list:" << endl << endl
			<< "1.Retrieve Pair Symbols and Save to Database" << endl
			<< "2.Retrieve Historical Trade Data and Save to Database" << endl
			<< "3.Create Pair1, Pair2, and PairPrices tables in Database" << endl
			<< "4.Calculate Price Ratio Std and Save to Database" << endl
			<< "5.Create Trades Table and Run Backtest" << endl
			<< "6.Calculate Profitable Ratio and Save to Database" << endl
			<< "7.Enter a Pair Trade" << endl
			<< "8.Exit your program" << endl;
		cin >> str;

		int answer;
		answer = std::stoi(str);
		switch (answer)
		{
		case 1:
		{
			OpenDatabase(stockDB_name, stockDB);
			RetrievePairSymbols(stockDB);
			CloseDatabase(stockDB);
			attempt++;
			break;
		}


		case 2:
		{
			OpenDatabase(stockDB_name, stockDB);
			RetrievePairPrices(stockDB);
			CloseDatabase(stockDB);
			attempt++;
			break;
		}

		case 3:
		{
			OpenDatabase(stockDB_name, stockDB);
			// Pair1Stocks table
			CreatePair1StocksTable(stockDB);
			// Pair2Stocks table
			CreatePair2StocksTable(stockDB);
			// PairPrices table
			CreatePairPricesTable(stockDB);
			CloseDatabase(stockDB);
			attempt++;
			break;
		}

		case 4:
		{
			OpenDatabase(stockDB_name, stockDB);
			// PairPriceRatio table
			CreatePairPriceRatioTable(stockDB);
			CloseDatabase(stockDB);
			attempt++;
			break;
		}

		case 5:
		{
			OpenDatabase(stockDB_name, stockDB);
			// Trades table
			CreateTradesTable(stockDB);

			// PairPriceStdTable
			string pair_price_ratio_select_table = "Select * From PairPriceRatio;";
			RetrievePairPriceStdTable(pair_price_ratio_select_table.c_str(), stockDB);

			// TradeTable
			string trades_select_table = "Select * From Trades;";
			RetrieveTradeTable(trades_select_table.c_str(), stockDB);

			// update TradeTable
			UpdateTradeTable(stockDB_name, stockDB);
			CloseDatabase(stockDB);
			attempt++;
			break;
		}

		case 6:
		{
			OpenDatabase(stockDB_name, stockDB);
			// QueryProfitableTrades table
			CreateQueryProfitableTradesTable(stockDB);
			CloseDatabase(stockDB);
			attempt++;
			break;
		}

		case 7:
		{
			OpenDatabase(stockDB_name, stockDB);
			EnterPairTrade(stockDB);
			CloseDatabase(stockDB);
			attempt++;
			break;
		}

		case 8:
		{
			a = false;
			cout << endl << "Successfully exit!" << endl;
			exit(0);
			break;
		}

		default:
		{
			cout << endl << "Bad choice! Please try again: " << endl;
		}
		}
	}
	system("pause");
	return 0;
}


