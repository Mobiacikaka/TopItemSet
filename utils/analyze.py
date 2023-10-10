#!/bin/python3

import logging
from numpy import average
import os

def ReadRealTopK():
	f = open('./realtopk.txt')
	return [line.strip('\n') for line in f.readlines()][:128]

def ReadMineTopK(arg, time) -> list | bool:
	eps, k, mu = arg
	folder = f'{result_folder_name}/eps_{eps}_k_{k}_kbar_{k}_mu_{mu}/{time}'

	try:
		srv_file = open(f'{folder}/server/itemset_2_select.out')
		cli_file = open(f'{folder}/client/itemset_2_select.out')
	except:
		logging.error(f'{folder} has error!')
		return False

	srv_lines = srv_file.readlines()
	cli_lines = cli_file.readlines()
	assert(len(srv_lines) == len(cli_lines))

	length = len(srv_lines)
	mine_topk_list = []
	for i in range(length):
		id_srv = srv_lines[i].split('\t')[0]
		id_cli = cli_lines[i].split('\t')[0]
		id_srv_flag = len(id_srv) != 0
		id_cli_flag = len(id_cli) != 0
		assert(id_srv_flag ^ id_cli_flag)
		mine_topk_list.append(id_srv or id_cli)

	return mine_topk_list

def analyze():
	args = [
		(eps, k, mu)
		for eps in eps_list
		for k in k_list
		for mu in mu_list
	]

	real_topk_list = ReadRealTopK()
	ji_list  = []
	ncr_list = []

	def cal_ji(real: list, mine: list) -> float:
		c = 0
		for item in real:
			if item in mine:
				c += 1
		return c / (len(real) + len(mine) - c)

	def cal_ncr(real: list, mine: list) -> float:
		s = 0
		length = len(real)
		for i in range(length):
			if real[i] in mine:
				s += (len(real) - i)
		return s * 2 / (length * (length + 1))

	for arg in args:
		_, k, _ = arg
		ji_list_time = []
		ncr_list_time = []
		for time in range(run_times):
			mine_topk_list = ReadMineTopK(arg, time)
			if mine_topk_list != False:
				assert(type(mine_topk_list) == list)
				ji_list_time.append(cal_ji(real_topk_list[:k], mine_topk_list))
				ncr_list_time.append(cal_ncr(real_topk_list[:k], mine_topk_list))
		ji_list.append(average(ji_list_time))
		ncr_list.append(average(ncr_list_time))

	print('ji: ', ji_list)
	print('ncr: ', ncr_list)

if __name__ == '__main__':
	dataset = input('Dataset: ')
	os.chdir(f'./datasets/{dataset}')
	var = input('eps or k: ') or 'eps'
	print(os.getcwd())
	result_folder_name = ''
	eps_list = []
	k_list = []
	mu_list = [0.9]
	kbar_list = []
	run_times = 10

	def set_global_to_var_eps():
		global result_folder_name, eps_list, k_list
		result_folder_name = 'result_eps'
		eps_list = [5 * (i+1) / 10 for i in range(8)]
		k_list = [32]

	def set_global_to_var_k():
		global result_folder_name, eps_list, k_list
		result_folder_name = 'result_k'
		eps_list = [4.0]
		k_list = [8, 16, 32, 48, 64, 80, 96, 112, 128]

	if var == 'eps':
		set_global_to_var_eps()
	else:
		set_global_to_var_k()
	analyze()
