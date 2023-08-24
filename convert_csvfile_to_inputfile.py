#!/bin/python3

import csv
filename = input()

with open(filename, newline='') as csvfile:
	spamreader = csv.reader(csvfile, delimiter='\t')
	spamreader = list(spamreader)
	print(sum(1 for _ in spamreader))
	for row in spamreader:
		try:
			print(','.join(row))
		except:
			print(row)
