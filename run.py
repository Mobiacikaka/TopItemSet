#!/bin/python3
import os
from src.gm import generate_key

dataset = 'gowalla'

def build_topitemset():
	os.system('mkdir -p build')
	os.system('cd build && cmake .. && make -j$(nproc)')

def copy_file(folder_name):
	os.system(f'cp ./build/topitemset {dataset}/topitemset')
	os.system(f'cp ./datasets/{dataset}/server.txt {dataset}/')
	os.system(f'cp ./datasets/{dataset}/client.txt {dataset}/')

	os.system(f'mkdir -p {folder_name}')
	os.system(f'mkdir -p {folder_name}/server')
	os.system(f'mkdir -p {folder_name}/client')
	os.system(f'cp ./build/topitemset {folder_name}/server')
	os.system(f'cp ./build/topitemset {folder_name}/client')
	os.system(f'cp ./datasets/{dataset}/server.txt {folder_name}/server/dataset.txt')
	os.system(f'cp ./datasets/{dataset}/client.txt {folder_name}/client/dataset.txt')
	os.system(f'touch ./{folder_name}/server/Selection.out')
	os.system(f'touch ./{folder_name}/client/Selection.out')

def copy_key(folder_name):
	def print_key():
		f = open('key.txt', 'w')
		key_A = generate_key()
		n, y = key_A['pub']
		p, q = key_A['priv']
		f.write(str(int(n)) + '\n')
		f.write(str(int(y)) + '\n')
		f.write(str(int(p)) + '\n')
		f.write(str(int(q)) + '\n')
		f.close()

	print_key()
	os.system(f'mv key.txt {folder_name}/server/')

def run_topitemset(folder_name, eps, k, kbar, mu):
	os.system(f'cd ./{folder_name}/server && ./topitemset -2 {eps} -k {k} -1 {kbar} -m {mu} -r 0 >/dev/null 2>&1 &')
	os.system(f'cd ./{folder_name}/client && ./topitemset -2 {eps} -k {k} -1 {kbar} -m {mu} -r 1 >/dev/null 2>&1')

def cal_metric(folder_name, k):
	sel_srv = open(f'./{folder_name}/server/Selection.out')
	sel_cli = open(f'./{folder_name}/client/Selection.out')
	ttl_srv = open(f'./{folder_name}/server/dataset.txt')
	ttl_cli = open(f'./{folder_name}/client/dataset.txt')

	sel_srv_lines = sel_srv.readlines()
	sel_cli_lines = sel_cli.readlines()
	ttl_srv_lines = ttl_srv.readlines()[1:]
	ttl_cli_lines = ttl_cli.readlines()[1:]

	my_top_k = []
	for item in sel_cli_lines + sel_srv_lines:
		my_top_k.append(item.strip('\n'))

	ttl_dict = {}
	for item in ttl_srv_lines + ttl_cli_lines:
		name, num = item.strip('\n').split('\t')
		if name in ttl_dict:
			ttl_dict[name] += int(num)
		else:
			ttl_dict[name] = int(num)
	real_top_k = sorted(ttl_dict, key=lambda x: ttl_dict.get(x), reverse=True)[:k]

	delta = 0
	for item in my_top_k:
		if item in real_top_k:
			delta += 1
	ji = delta / (len(my_top_k) - delta + k)

	score = k
	tt_score = 0
	for item in real_top_k:
		if item in my_top_k:
			tt_score += score
		score -= 1
	ncr = tt_score * 2 / (k * (k+1))

	sel_srv.close()
	sel_cli.close()
	ttl_srv.close()
	ttl_cli.close()

	return ji, ncr

def remove_files(eps, k, kbar, mu, times):
	folder_name = f'eps_{eps}_k_{k}_kbar_{kbar}_mu_{mu}_times_{times}'
	os.system(f'rm {folder_name}/server/dataset.txt')
	os.system(f'rm {folder_name}/server/topitemset')
	os.system(f'rm {folder_name}/client/dataset.txt')
	os.system(f'rm {folder_name}/client/topitemset')

def one_run(eps, k, kbar, mu, times):
	folder_name = f'eps_{eps}_k_{k}_kbar_{kbar}_mu_{mu}_times_{times}'
	copy_file(folder_name)
	copy_key(folder_name)
	run_topitemset(folder_name, eps, k, kbar, mu)
	return cal_metric(folder_name, k)

if __name__ == '__main__':
	eps_list = [1.0]
	k_list = [20]
	mu_list = list(range(1, 11))
	mu_list = [float(mu)/10 for mu in mu_list]
	kbar_list = [20]

	args = [
		(eps, k, kbar, mu)
		for eps in eps_list
		for k in k_list
		for kbar in kbar_list
		for mu in mu_list
	]

	os.system(f'mkdir -p {dataset}')
	build_topitemset()

	for arg in args:
		eps, k, kbar, mu = arg
		ji_list = []
		ncr_list = []
		run_times = 10
		print(f'eps: {eps}, k: {k}, kbar: {kbar}, mu: {mu}, times: {run_times}')
		for times in range(run_times):
			ji, ncr = one_run(eps, k, kbar, mu, times)
			remove_files(eps, k, kbar, mu, times)
			ji_list.append(ji)
			ncr_list.append(ncr)
		print(f'avg_ji: {sum(ji_list)/run_times:.3f}, avg_ncr: {sum(ncr_list)/run_times:.3f}')

	os.system(f'mv eps_* {dataset}')

