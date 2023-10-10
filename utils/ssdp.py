#!/bin/python3
from os import system
import numpy
import heapq
import copy
import random
import multiprocessing

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
		key = tuple([key])
		count_dict[(key)] = freq
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
	# print(topk_itemsets)
	return topk_itemsets

def RandomDraw(R: int):
	mask = 0
	while R >> mask > 0:
		mask += 1
	mask = 2 ** mask - 1

	for _ in range(2**20):
		r = random.randint(0, mask)
		if r < R:
			return r
	exit()

def select(topk_itemsets: list[tuple], eps):
	topk_itemsets = [('', 0)] + topk_itemsets[::-1] + topk_itemsets + [('', 0)]
	length = len(topk_itemsets)
	gap  = [0] * length
	mass = [0] * length
	t    = 0
	for i in range(1, length-1):
		utility = i - length/2 + 1 if i < length/2 else length/2 - i
		weight = numpy.exp(eps * utility)
		if i <= length/2 - 1:
			gap[i] = topk_itemsets[i][1] - topk_itemsets[i-1][1]
		else:
			gap[i] = topk_itemsets[i][1] - topk_itemsets[i+1][1]
		mass[i] = t + weight * gap[i]
		t = mass[i]
	mass[-1] = mass[-2]

	# print('\n', gap)
	# print('\n', mass)

	R = mass[-1]
	r = RandomDraw(int(R))
	j = -1
	for i in range(length):
		if r < mass[i] and j == -1:
			j = i
			break
	# print(r, '\t', mass[j], mass[j-1], '\t', topk_itemsets[j])
	return topk_itemsets[j]

def multi_select(topk_itemsets: list[tuple], eps, k):
	selection = []
	topk_itemsets = copy.deepcopy(topk_itemsets)
	for _ in range(k):
		sel = select(copy.deepcopy(topk_itemsets), eps / k)
		try:
			topk_itemsets.remove(sel)
		except:
			pass
		selection.append(sel)
		# exit()
	return selection

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

def run(eps: float, k: int, foldername: str):
	print('eps', eps, 'k', k, foldername)
	server_itemsets = calculate_topk_itemsets(0, 3 * k)
	client_itemsets = calculate_topk_itemsets(1, 3 * k)
	selection_srv   = multi_select(server_itemsets, eps, k)
	selection_cli   = multi_select(client_itemsets, eps, k)
	merge_itemsets  = merge(selection_srv, selection_cli)
	sorted_itemsets = sort(merge_itemsets)

	system(f'mkdir -p {foldername}')
	outfile = open(f'{foldername}/ssdp.txt', 'w')
	for itemset, freq in sorted_itemsets[:k]:
		itemset = list(itemset)
		outfile.write(','.join(itemset) + f',\t{freq}\n')
	outfile.close()

def multi_run():
	times    = 10

	def set_var_eps():
		eps_list = [5 * i / 10 for i in range(1,9)]
		k_list = [32]
		result_f = 'result_eps'
		return eps_list, k_list, result_f

	def set_var_k():
		eps_list = [4.0]
		k_list = [8, 16, 32, 48, 64, 80, 96, 112, 128]
		result_f = 'result_k'
		return eps_list, k_list, result_f

	def set_var_test():
		eps_list = [1.0]
		k_list = [8]
		result_f = 'test'
		return eps_list, k_list, result_f

	eps_list, k_list, result_f = set_var_k()

	args = [
		(eps, k, f'ssdp/{result_f}/eps_{eps}_k_{k}/{t}')
		for eps in eps_list
		for k in k_list
		for t in range(times)
	]

	pool = multiprocessing.Pool(multiprocessing.cpu_count() - 4)
	pool.starmap(run, args)
	pool.close()
	pool.join()

if __name__ == '__main__':
	multi_run()
