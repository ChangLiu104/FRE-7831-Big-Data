#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <time.h>
#include <locale>
#include <iomanip>

using namespace std;

int main()
{
	float Volume[26] = {0.0};
	float Price[26];

	string fileName = "C:\\Users\\steph\\source\\repos\\Big Data HW3\\SPY_May_2012.csv";
	ifstream file(fileName);
	string line;
	while (getline(file, line))
	{
		int n = line.length();
		char * char_arr = new char[n + 1];
		strcpy(char_arr, line.c_str());
		char * p;
		uint8_t j;
		string row[13];
		for (j = 0, p = strtok(char_arr, ","); p != NULL; p = strtok(NULL, ","), j++)
		{
			row[j] = p;
			if (j > 6)
			{
				continue;
			}
		}
		// filter the dates
		if (row[1] == "22-MAY-2012")
		{
			break;
		}
		// filter the "Trade" records
		if (row[4] != "Trade")
		{
			delete[] char_arr;
			continue;
		}
		string time = row[2];
		int hour = atoi(time.substr(0, 2).c_str());
		float minute = atof(time.substr(3, 2).c_str()) + atof(time.substr(6).c_str()) / 60.0;

		int day = atoi(row[1].substr(0, 2).c_str());
		int index = (hour - 9) * 4 + floor(minute / 15.0) - 2;

		if (index < 0 || index>25)
		{
			delete[] char_arr;
			continue;
		}
		if (day >= 1 && day <= 20)
		{
			// sum of volume in the interval
			Volume[index] = Volume[index] + (float)atof(row[6].c_str());
		}
		if (day == 21)
		{
			if (Price[index] < 1)
			{
				// first price of interval
				Price[index] = (float)atof(row[5].c_str());
			}
		}
		// cout << row[1] << endl;
		delete[] char_arr;  // free the memory
	}
	file.close();
	cout << "finish reading data" << endl;

	ofstream output;
	output.open("result.csv", ios::out);
	for (int i = 0; i < 26; i++)
	{
		string s = "09:30:00.0";
		std::tm t{};
		std::istringstream ss(s);
		ss >> std::get_time(&t, "%H:%M:%S");
		std::time_t time_stamp = mktime(&t);


		t.tm_min += i * 15;
		t.tm_hour += t.tm_min / 60;
		t.tm_min = t.tm_min % 60;

		time_t  t_of_day = mktime(&t);
		char MY_TIME[50];
		strftime(MY_TIME, sizeof(MY_TIME), "%H:%M:%S", &t);

		float AvgVol = Volume[i] / 14.0;

		output << MY_TIME << "," << AvgVol << "," << Price[i] << endl;
	}
	output.close();
	system("Pause");
	return 0;
}

