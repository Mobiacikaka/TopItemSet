#!/bin/python3
import os

if __name__ == '__main__':
	eps_list = [1.0]
	k_list = [20]
	mu_list = [0.5]
	kbar_list = list(range(20, 31))

	args = [
		(eps, k, kbar, mu)
		for eps in eps_list
		for k in k_list
		for kbar in kbar_list
		for mu in mu_list
	]

	for arg in args:
		eps, k, kbar, mu = arg
		run_times = 10
		time_list = []

	for eps in eps_list:
		for k in k_list:
			for mu in mu_list:
				run_times = 5
				time_list = []
				print(f'eps: {eps}, k: {k}, mu: {mu}')
				for times in range(run_times):
					folder_name = f'eps_{eps}_k_{k}_mu_{mu}_times_{times}'
					with open(f'{folder_name}/server/Runtime.out') as f:
						lines = f.readlines()
					time = []
					for line in lines:
						time.append(float(line.strip('\n')))
					time_list.append(time)
				# print avg time
				total_time_list = [time[4] for time in time_list]
				print(f'average time: {sum(total_time_list) / len(total_time_list)}')
