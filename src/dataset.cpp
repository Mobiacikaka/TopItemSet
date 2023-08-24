#include "dataset.hpp"

#include <algorithm>
#include <fstream>
#include <map>
#include <iostream>
#include <cassert>
#include <bits/stdc++.h>
using namespace std;

Itemset::Itemset(std::string uncut_str, char del='\t')
{
	// https://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
	stringstream ss(uncut_str);
	string word;
	while (!ss.eof()) {
		getline(ss, word, del);
		this->data.push_back(word);
	}
}

Itemset::Itemset(std::vector<std::string> cut_str)
{
	this->data = cut_str;
}

void Itemset::PrintItemset() const
{
	size_t length = this->data.size();
	for (size_t i = 0; i < length; i ++)
	{
		cout << this->data[i] << ",";
	}
	cout << endl;
}

size_t Itemset::GetItemsetSize() const
{
	return this->data.size();
}

string Itemset::operator[](size_t i) const
{
	return this->data[i];
}

void Itemset::PruneItemset(vector<string> item_remove_exception) 
{
	vector<string> newdata;
	for(size_t i = 0; i < this->data.size(); i ++) {
		auto it = find(
			item_remove_exception.begin(),
			item_remove_exception.end(),
			this->data[i]
		);
		if(it != item_remove_exception.end()) newdata.push_back(this->data[i]);
	}
	this->data = newdata;
	return ;
}

string Itemset::ConcatWithOrder() const
{
	string concatstr;
	vector<string> sorted(this->data);
	sort(sorted.begin(), sorted.end());
	for(size_t i = 0; i < sorted.size(); i++)
		concatstr = concatstr + sorted[i] + ",";
	return concatstr;
}

Dataset::Dataset()
{
	size_t num_users;
	cin >> num_users;
	for(size_t i = 0; i < num_users; i ++)
	{
		string uncut_itemset_str;
		getline(cin, uncut_itemset_str, '\n');
		Itemset itemset(uncut_itemset_str);
		this->data.push_back(itemset);
	}
}

Dataset::Dataset(vector<Itemset> &data)
{
	this->data = data;
}

Itemset & Dataset::operator[](size_t i)
{
	return this->data[i];
}

size_t Dataset::GetDatasetSize() const
{
	return this->data.size();
}

void Dataset::PrintDataset() const
{
	for(size_t i = 0; i < this->data.size(); i++)
	{
		this->data[i].PrintItemset();
	}
}

void Dataset::PruneDataset(vector<string> item_remove_exception) 
{
	assert(item_remove_exception.size() > 0);
	vector<Itemset> newdata;
	for(size_t i = 0; i < this->data.size(); i ++) {
		this->data[i].PruneItemset(item_remove_exception);
		if(this->data[i].GetItemsetSize()) newdata.push_back(this->data[i]);
	}
	this->data = newdata;
	return;
}

// BloomFilter
const size_t BLOOMFILTER_NUM_ENTRIES = 1000000;
const float BLOOMFILTER_ERRORS_RATE = 0.01;

void KVDataset::GenerateKVDataset(Dataset &original_dataset)
{
	map<string, size_t> kvdata_map;

	this->kvdata.clear();

	for(size_t i = 0; i < original_dataset.GetDatasetSize(); i++)
	{
		Itemset itemset(original_dataset[i]);
		for (size_t j = 0; j < itemset.GetItemsetSize(); j++)
			kvdata_map[itemset[j]] ++;
	}

	std::transform(
			kvdata_map.begin(),
			kvdata_map.end(),
			std::back_inserter(this->kvdata),
			[](const KVpair &p) {
				return p;
			}
			);
}

void KVDataset::GenerateKVDataset(Dataset &original_dataset, bool)
{
	map<string, size_t> hashmap;
	this->kvdata.clear();
	for(size_t i = 0; i < original_dataset.GetDatasetSize(); i++) {
		assert(original_dataset[i].GetItemsetSize() > 0);
		size_t itemsetlength(original_dataset[i].GetItemsetSize());
		for(size_t selectionsize = 1; selectionsize <= itemsetlength; selectionsize ++) {
			string bitmask(selectionsize, 1);
			bitmask.resize(itemsetlength, 0);
			do {
				vector<string> combination;
				for(size_t j = 0; j < itemsetlength; j ++)
					if(bitmask[j]) combination.push_back(original_dataset[i][j]);
				Itemset combination_itemset(combination);
				string concatstr = combination_itemset.ConcatWithOrder();
				hashmap[concatstr] += 1;
			} while (prev_permutation(bitmask.begin(), bitmask.end()));
		}
	}

	std::transform(
		hashmap.begin(),
		hashmap.end(),
		std::back_inserter(this->kvdata),
		[](const KVpair &p) {
			return p;
		}
	);
}

size_t KVDataset::GetKVDatasetSize() const
{
	return this->kvdata.size();
}

void KVDataset::PrintKVDataset(ostream &out=std::cout) const
{
	for(auto it = this->kvdata.begin(); it < this->kvdata.end(); it++)
		out << it->first << "\t" << it->second << endl;
}

void KVDataset::PrintKVDataset(std::string filename) const
{
	ofstream file(filename);
	if(file.is_open() == false) {
		cerr << filename << " open failed" << endl;
		exit(-1);
	}
	this->PrintKVDataset(file);
}

void KVDataset::SortKVDataset()
{
	sort(this->kvdata.begin(),
		this->kvdata.end(),
		[](const KVpair &a, const KVpair &b) {return a.second > b.second;}
	);
}

void KVDataset::EraseFrom(size_t index)
{
	this->kvdata.erase(this->kvdata.begin() + index);
}

KVpair & KVDataset::operator[](size_t i)
{
	return this->kvdata[i];
}
