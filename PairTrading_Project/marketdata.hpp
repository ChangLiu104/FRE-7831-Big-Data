//
//  marketdata.hpp
//

#ifndef marketdata_hpp
#define marketdata_hpp

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iomanip>

#include "database.hpp"
#include "curl_easy.h"
#include "curl_form.h"
#include "curl_ios.h"
#include "curl_exception.h"

#include "PairTrading.h"

using namespace std;

int RetrieveMarketData(string url_request, string & read_buffer);
int PopulatePairTable(sqlite3 *db, const map<int, pair<string, string>> & symbols);
int PopulatePairStockTable(sqlite3 *db, const map<string, Stock> & stocks);

#endif
