#!/bin/python3
import numpy
import heapq

def readfile(filename):
	f = open(filename)
	ls = f.readlines()
	ls = [l.strip('\n').split(' ') for l in ls]
	return ls

def realtopk(data: list[list[str]]):
	countdict = {}
	for line in data:
		for item in line:
			if countdict.get(item) != None:
				countdict[item] += 1
			else:
				countdict[item] = 1

	countsorted = sorted(countdict.items(), key=lambda item: item[1], reverse=True)
	countsorted = countsorted[:k]
	return countsorted

def build_tuple_cand_bfs(cand_dict_prob, cand_dict, new_cand_inv, keys, values):
	ret = []
	cur = []
	for i in range(len(keys)):
		heapq.heappush(ret, (values[i], tuple([i])))
		heapq.heappush(cur, (-values[i], tuple([i])))
	while len(cur) > 0:
		new_cur = []
		while len(cur) > 0:
			(prob, t) = heapq.heappop(cur)
			for j in range(t[-1] + 1, len(keys)):
				if len(ret) >= 3 * len(keys):
					if -prob * values[j] > ret[0][0]:
						heapq.heappop(ret)
						l = list(t)
						l.append(j)
						heapq.heappush(ret, (-prob * values[j], tuple(l)))
						heapq.heappush(new_cur, (prob * values[j], tuple(l)))
				else:
					l = list(t)
					l.append(j)
					heapq.heappush(ret, (-prob * values[j], tuple(l)))
					heapq.heappush(new_cur, (prob * values[j], tuple(l)))
		cur = new_cur

	while len(ret) > 0:
		(prob, t) = heapq.heappop(ret)
		if len(t) == 1:
			continue
		l = list(t)
		new_l = []
		for i in l:
			new_l.append(keys[i])
		new_t = tuple(new_l)
		cand_dict[new_t] = len(new_cand_inv)
		cand_dict_prob[new_t] = prob
		new_cand_inv.append(new_t)

def build_candidate_itemsets(keyfreqlist: list[tuple], k):
	key_list = []
	freq_list = []
	for key, freq in keyfreqlist:
		key_list.append(key)
		freq_list.append(freq)
	length = len(key_list)

	cand_dict = {}
	for i in range(length):
		cand_dict[key_list[i]] = freq_list[i]

	normalized_values = numpy.zeros(length)
	for i in range(length):
		normalized_values[i] = freq_list[i] * 0.8 / freq_list[0]

	cand_dict = {}
	cand_dict_prob = {}
	cand_set_list = []
	build_tuple_cand_bfs(cand_dict_prob, cand_dict, cand_set_list, key_list, normalized_values)
	cand_list = list(cand_dict.keys())
	cand_value = list(cand_dict_prob.values())
	sorted_indices = numpy.argsort(cand_value)
	cand_set_map = {}
	cand_set_list = []
	for j in sorted_indices[-k:]:
		cand_set_map[cand_list[j]] = len(cand_set_list)
		cand_set_list.append(tuple(cand_list[j]))
	return cand_set_map, cand_set_list

def count_candidate_itemsets_freq(data: list[list[str]], cand_set_list: list):
	cand_set_freq_list = {}
	for cand_set in cand_set_list:
		cand_set_freq_list[cand_set] = 0

	for userdata in data:
		userdata = set(userdata)
		for cand_set in cand_set_list:
			_cand_set = set(cand_set)
			if _cand_set.issubset(userdata):
				cand_set_freq_list[cand_set] += 1

	return cand_set_freq_list

def combine_items_with_itemsets(topkitem: list, topkitemset: dict, k):
	count_dict = {}
	for key, freq in topkitem:
		count_dict[key] = freq
	for key, freq in topkitemset.items():
		count_dict[key] = freq

	topk_itemsets = sorted(count_dict.items(), key=lambda item: item[1], reverse=True)
	return topk_itemsets[:k]

def calculate_topk_itemsets(role):
	filename = ''
	if role == 0:
		filename = 'server.txt'
	elif role == 1:
		filename = 'client.txt'
	else:
		assert(0)

	data = readfile(filename)
	keyfreqlist = realtopk(data)
	cand_set_map, cand_set_list = build_candidate_itemsets(keyfreqlist, k)
	cand_set_freq_list = count_candidate_itemsets_freq(data, cand_set_list)
	topk_itemsets = combine_items_with_itemsets(keyfreqlist, cand_set_freq_list, k)
	print(topk_itemsets)
	return topk_itemsets

if __name__ == '__main__':
	eps = 4.0
	k   = 32
	server_dict = calculate_topk_itemsets(0)
	client_dict = calculate_topk_itemsets(1)
