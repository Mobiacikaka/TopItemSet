#!/bin/python3
import numpy
import heapq

def readfile(filename):
	f = open(filename)
	ls = f.readlines()
	ls = [l.strip('\n').split(' ') for l in ls]
	return ls

def realtopk(data: list[list[str]], k):
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
		sorted_cand_list = sorted(cand_list[j])
		cand_set_list.append(tuple(sorted_cand_list))
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

def calculate_topk_itemsets(role, k):
	filename = ''
	if role == 0:
		filename = 'server.txt'
	elif role == 1:
		filename = 'client.txt'
	else:
		assert(0)

	data = readfile(filename)
	keyfreqlist = realtopk(data, k)
	cand_set_map, cand_set_list = build_candidate_itemsets(keyfreqlist, k)
	cand_set_freq_list = count_candidate_itemsets_freq(data, cand_set_list)
	topk_itemsets = combine_items_with_itemsets(keyfreqlist, cand_set_freq_list, k)
	print(topk_itemsets)
	return topk_itemsets

def merge(server_dict: list, client_dict: list):
	merge_dict = {}
	for key, _ in server_dict:
		merge_dict[key] = 0
	for key, _ in client_dict:
		merge_dict[key] = 0
	for key, freq in server_dict:
		merge_dict[key] += freq
	for key, freq in client_dict:
		merge_dict[key] += freq
	return merge_dict

def sort(merge_dict: dict):
	sorted_merge_list = sorted(merge_dict.items(), key=lambda item: item[1], reverse=True)
	return sorted_merge_list

def select(sorted_itemsets, eps, k):
	eps_one = eps / k
	utility = list(range(len(sorted_itemsets)))
	weight = [eps_one * uti for uti in utility]
	gap = numpy.zeros(len(sorted_itemsets)).tolist()
	for i in range(len(sorted_itemsets) - 1):
		gap[i] = sorted_itemsets[i][1] - sorted_itemsets[i+1][1]
	gap[-1] = sorted_itemsets[-1][1]
	print(utility)
	print(weight)
	print(gap)

def run():
	eps = 4.0
	k   = 32 * 3
	server_dict = calculate_topk_itemsets(0, k)
	client_dict = calculate_topk_itemsets(1, k)
	# merge_dict = merge(server_dict, client_dict)
	# sorted_itemsets = sort(merge_dict)
	# print(sorted_itemsets)
	# select(sorted_itemsets, eps, k)

if __name__ == '__main__':
	run()
