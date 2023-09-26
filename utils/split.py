#!/bin/python3

import numpy

if __name__ == '__main__':
	original_data = open("./kosarak.dat")
	srv_file = open("./server.txt", "w")
	cli_file = open("./client.txt", "w")
	srv_ratio = numpy.random.uniform(0.3, 0.7)
	for line in original_data.readlines():
		sel = numpy.random.choice([0, 1], p=[srv_ratio, 1-srv_ratio])
		if sel:
			srv_file.write(line)
		else:
			cli_file.write(line)
