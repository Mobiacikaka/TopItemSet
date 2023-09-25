#!/bin/python3
import csv
import pandas as pd
from mlxtend.preprocessing import TransactionEncoder
from mlxtend.frequent_patterns import apriori

if __name__ == '__main__':
	dataset = input()
	assert(dataset == 'kosarak' or dataset == 'IBM' or dataset == 'POS')
	datafile = open(f'./{dataset}.dat', 'r')
	dataset = [line[:-1].split(' ') for line in datafile.readlines()]
	for line in dataset:
		if '' in line:
			line.remove('')
	tr = TransactionEncoder()
	tr_arr = tr.fit(dataset).transform(dataset)
	df = pd.DataFrame(tr_arr, columns=tr.columns_)
	frequent_itemsets = apriori(df, min_support = 0.01, use_colnames = True).to_dict('records')
	frequent_itemsets.sort(key=lambda x:x['support'], reverse=True)
	for itemset in frequent_itemsets:
		itemset = list(itemset['itemsets'])
		itemset.sort()
		for item in itemset:
			pass
			print(item + ',', end='')
		print()
