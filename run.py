#!/bin/python3
import os
from os.path import isdir
import subprocess as sp
from src.gm import generate_key

root_dir = os.getcwd()

def build_topitemset():
	os.system('mkdir -p build')
	a = os.system('cd build && cmake .. && make clean && make')
	if a != 0:
		print("[run.py] Build Error!")
		exit()

def set_global(dataset: str):
	global binary, srv_data, cli_data
	binary = f'{root_dir}/build/topitemset'
	srv_data = f'{root_dir}/datasets/{dataset}/server.txt'
	cli_data = f'{root_dir}/datasets/{dataset}/client.txt'

def genkey():
	global keyfile
	keyfile = f'{root_dir}/key.txt'
	f = open(keyfile, 'w')
	key_A = generate_key()
	n, y = key_A['pub']
	p, q = key_A['priv']
	f.write(str(int(n)) + '\n')
	f.write(str(int(y)) + '\n')
	f.write(str(int(p)) + '\n')
	f.write(str(int(q)) + '\n')
	f.close()

def mkdir(foldername: str):
	flag = os.system(f'mkdir \"{foldername}\"')
	if flag == 256:
		print('Folder exist, please choose another name')
		exit()
	assert(flag == 0)

def cp(src: str, des: str):
	os.system(f'cp {src} {des}')

if __name__ == '__main__':
	dataset = 'kosarak'
	eps_list = [2.0]
	k_list = [64]
	mu_list = [0.9]
	kbar_list = [64]
	run_times = 1

	results_folder_name = input('Type the folder name the result located in: ')
	results_folder_name = f'{root_dir}/datasets/{dataset}/{results_folder_name}'
	if os.path.isdir(results_folder_name):
		flag = input('Folder exists, override? y or n: ') or 'n'
		assert(flag == 'y' or flag == 'n')
		if flag == 'y':
			os.system(f'rm -rf {results_folder_name}')
	mkdir(results_folder_name)

	args = [
		(eps, k, kbar, mu)
		for eps in eps_list
		for k in k_list
		for kbar in kbar_list
		for mu in mu_list
	]

	build_topitemset()
	genkey()
	set_global(dataset)

	for arg in args:
		eps, k, kbar, mu = arg
		print(f'Running args - eps: {eps}, k: {k}, kbar: {kbar}, mu: {mu}, times: {run_times}')

		def runcommand(role=0):
			datafile = srv_data
			if role == 1:
				datafile = cli_data
			command = f'cat {datafile} | {binary} -k {k} -1 {kbar} -2 {eps} -m {mu} -r {role} > log.out 2>&1'
			return command

		for times in range(run_times):
			result_folder_name = f'{results_folder_name}/eps_{eps}_k_{k}_kbar_{kbar}_mu_{mu}_times_{times}'
			mkdir(result_folder_name)
			mkdir(f'{result_folder_name}/server')
			cp(keyfile, f'{result_folder_name}/server')
			mkdir(f'{result_folder_name}/client')
			sp.Popen(runcommand(0), cwd=f'{result_folder_name}/server', shell=True, stdout=sp.PIPE)
			sp.run(runcommand(1), cwd=f'{result_folder_name}/client', shell=True, stdout=sp.PIPE)
