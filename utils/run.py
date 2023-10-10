#!/bin/python3
import os, subprocess, multiprocessing, socket, time, random
from gm import generate_key

root_dir = os.getcwd()

def build_topitemset():
	os.system('mkdir -p build')
	a = os.system('cd build && cmake .. && make')
	if a != 0:
		print("[run.py] Build Error!")
		exit()

def set_global(dataset: str):
	global binary, srv_data, cli_data, port_list
	binary = f'{root_dir}/build/topitemset'
	srv_data = f'{root_dir}/datasets/{dataset}/server.txt'
	cli_data = f'{root_dir}/datasets/{dataset}/client.txt'
	port_list = []

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

def onerun(eps, k, kbar, mu, times, port, results_folder_name):
	print(f'Running args - eps: {eps}, k: {k}, kbar: {kbar}, mu: {mu}, times: {times}')
	result_folder_name = f'{results_folder_name}/eps_{eps}_k_{k}_kbar_{kbar}_mu_{mu}'
	times_folder_name = f'{result_folder_name}/{times}'
	mkdir(times_folder_name)
	mkdir(f'{times_folder_name}/server')
	mkdir(f'{times_folder_name}/client')
	cp(keyfile, f'{times_folder_name}/server')

	def runcommand(role: int=0):
		assert(role == 0 or role == 1)
		if role == 0:
			datafile = srv_data
		else:
			datafile = cli_data
		command = f'cat {datafile} | {binary} -p {port} -k {k} -1 {kbar} -2 {eps} -m {mu} -r {role} > log.out 2>&1'
		return command

	def checkportfree(port: int):
		try:
			s.connect(('localhost', port))
			s.shutdown(2)
			return False
		except:
			return True

	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	while (not checkportfree(port)) or (port in port_list):
		assert(0)
		port = random.randint(port+2**10, port+2**11)
		if port > 2**15 + 2**14:
			port = 2**15 + 2**10
		time.sleep(1)

	port_list.append(port)
	subprocess.Popen(runcommand(0),	cwd=f'{times_folder_name}/server', shell=True, stdout=subprocess.PIPE)
	subprocess.run(runcommand(1),	cwd=f'{times_folder_name}/client', shell=True, stdout=subprocess.PIPE)
	port_list.remove(port)

def main():
	dataset = 'kosarak'
	eps_list = [2.0]
	k_list = [40]
	mu_list = [0.9]
	kbar_list = [40]
	run_times = 5

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
		kbar = k
		print(f'Running args - eps: {eps}, k: {k}, kbar: {kbar}, mu: {mu}, times: {run_times}')
		result_folder_name = f'{results_folder_name}/eps_{eps}_k_{k}_kbar_{kbar}_mu_{mu}'
		mkdir(result_folder_name)

		def runcommand(role=0):
			datafile = srv_data
			if role == 1:
				datafile = cli_data
			command = f'cat {datafile} | {binary} -k {k} -1 {kbar} -2 {eps} -m {mu} -r {role} > log.out 2>&1'
			return command

		for times in range(run_times):
			times_folder = f'{result_folder_name}/{times}'
			mkdir(times_folder)
			mkdir(f'{times_folder}/server')
			mkdir(f'{times_folder}/client')
			cp(keyfile, f'{times_folder}/server')
			subprocess.Popen(runcommand(0),	cwd=f'{times_folder}/server', shell=True, stdout=subprocess.PIPE)
			subprocess.run(runcommand(1),	cwd=f'{times_folder}/client', shell=True, stdout=subprocess.PIPE)

def main_multi():
	dataset		= 'IBM'
	eps_list	= [4.0]
	# eps_list	= [5 * i / 10 for i in range(1, 9)]
	# k_list		= [32]
	k_list		= [8, 16, 32, 48, 64, 80, 96, 112, 128]
	mu_list		= [0.9]
	kbar_list	= k_list
	run_times	= 10

	results_folder_name = input('Type the folder name the result located in: ')
	results_folder_name = f'{root_dir}/datasets/{dataset}/{results_folder_name}'
	if os.path.isdir(results_folder_name):
		flag = input('Folder exists, override? y or n: ') or 'n'
		assert(flag == 'y' or flag == 'n')
		if flag == 'y':
			os.system(f'rm -rf {results_folder_name}')
	mkdir(results_folder_name)

	base_port = 2**15
	addi_port = 0
	bound_port = 2**14

	args = []
	for eps in eps_list:
		for k in k_list:
			for mu in mu_list:
				for times in range(run_times):
					port = base_port + addi_port % bound_port
					args.append(
						(eps, k, k, mu, times, port, results_folder_name)
					)
					addi_port += 1

	build_topitemset()
	genkey()
	set_global(dataset)

	for arg in args:
		eps, k, kbar, mu, _, _, _ = arg
		result_folder_name = f'{results_folder_name}/eps_{eps}_k_{k}_kbar_{kbar}_mu_{mu}'
		if not os.path.isdir(result_folder_name):
			mkdir(result_folder_name)

	server_count = multiprocessing.cpu_count() // 2 - 2
	pool = multiprocessing.Pool(server_count)
	pool.starmap(onerun, args)
	pool.close()
	pool.join()

if __name__ == '__main__':
	main_multi()
