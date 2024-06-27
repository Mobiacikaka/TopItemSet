#!/bin/python3
import heapq
import mlxtend.frequent_patterns.apriori as apriori

def fim(datas: list[list[str]], k: int) -> list:
	frequency_dict = {}
	for userset in datas:
		for item in userset:
			frequency_dict[item] = frequency_dict.get(item, 0) + 1
	fi = sorted(frequency_dict.items(), key=lambda x: x[1], reverse=True)
	return fi[:k]

def count_itemset(datas: list[list[str]], itemset: list[str]) -> int:
	c = 0
	for userset in datas:
		if itemset in userset:
			c += 1
	return c

class kv(object):
	def __init__(self, key: list[str], val: int) -> None:
		self.key = key
		self.val = val

	def __repr__(self):
		return f'{self.key}: {self.val}'

	def __lt__(self, other):
		return self.val < other.val

def fsm(datas: list[list[str]], fi: list[str], k) -> list[list[str]]:
	minheap = []
	Lk = []
	return []

def run():
	k = 256

	f = open('POS.dat')
	lines = f.readlines()
	datas = [l.strip('\n').split().sort() for l in lines]

if __name__ == '__main__':
	pass
