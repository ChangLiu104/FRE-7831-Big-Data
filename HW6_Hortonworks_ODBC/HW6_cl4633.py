# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""

import pyodbc

pyodbc.autocommit = True
conn = pyodbc.connect("DSN=Sample Hortonworks Hive DSN; Host=40.123.36.202; Port=10000;", autocommit=True)

cursor = conn.cursor();

cursor.execute("select year, symbol, CAST(total_dividends AS DECIMAL(4, 3)) from default.yearly_aggregates where symbol = 'IBM' and year = '1996'")

result = cursor.fetchall() 
for r in result:
    print(r)
    
'''
('1996', 'IBM', Decimal('0.325'))
'''

