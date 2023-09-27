#!/bin/python3

from io import TextIOWrapper
from matplotlib import pyplot
import numpy

def draw():
	comparison_list = [
		'svsm',
		'mine',
	]

	markers = [
		'o',
		'v',
	]

	comparison_metric = input('Metric, ji or ncr: ') or 'ji'
	assert(comparison_metric == 'ji' or comparison_metric == 'ncr')
	comparison_var = input('Variable, eps or k: ') or 'eps'
	assert(comparison_var == 'eps' or comparison_var == 'k')
	dataset = input('Dataset, POS or IBM or kosarak: ') or 'IBM'
	assert(dataset == 'POS' or dataset == 'IBM' or dataset == 'kosarak')

	x: list = []
	if comparison_var == 'eps':
		x = [5 * (i + 1) / 10 for i in range(8)]
	else:
		x = [8, 16, 32, 48, 64, 80, 96, 112, 128]

	file_list: list[TextIOWrapper] = []
	for method in comparison_list:
		file_list.append(open(f'./datasets/{dataset}/comparison/{method}_{comparison_var}_{comparison_metric}.txt'))

	y_list: list[list[float]] = []
	for file in file_list:
		ji_list = file.readline().split(' ')
		ji_list = [float(ji) for ji in ji_list]
		assert(len(ji_list) == len(x))
		y_list.append(ji_list)

	for i in range(len(comparison_list)):
		y = y_list[i]
		pyplot.plot(x, y, label=comparison_list[i], marker=markers[i])

	pyplot.xlabel(comparison_var)
	pyplot.ylabel(comparison_metric)
	pyplot.xticks(x)
	pyplot.yticks(numpy.arange(0, 1.1, 0.1))
	pyplot.legend(fontsize='medium')
	pyplot.show()

if __name__ == '__main__':
	draw()
