#include "dataset.hpp"

#include <algorithm>
#include <fstream>
#include <map>
#include <iostream>
#include <cassert>
#include <bits/stdc++.h>
using namespace std;

Itemset::Itemset(std::string uncut_str, char del=' ')
{
	// https://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
	/* stringstream ss(uncut_str); */
	/* string word; */
	/* while (!ss.eof()) { */
	/* 	getline(ss, word, del); */
	/* 	this->data.push_back(word); */
	/* } */
	size_t begin(0), end(0);
	while(begin < uncut_str.size())
	{
		end = uncut_str.find(del, begin);
		if(end != std::string::npos)
			this->data.push_back(uncut_str.substr(begin, end-begin));
		else {
			this->data.push_back(uncut_str.substr(begin));
			break;
		}
		begin = end + 1;
	}

	// data always sorted
	sort(this->data.begin(), this->data.end());
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
	// assert this->data is ordered
	string concatstr;
	for(size_t i = 0; i < this->data.size(); i++)
		concatstr = concatstr + this->data[i] + ",";
	return concatstr;
}

bool Itemset::include(std::vector<std::string> &smallset) const
{
	// data and smallset must be sorted
	return includes(
			this->data.begin(),
			this->data.end(),
			smallset.begin(),
			smallset.end()
		);
}

Dataset::Dataset()
{
	string uncut_itemset_str;
	while(getline(cin, uncut_itemset_str, '\n'))
	{
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

void Dataset::PruneDataset(size_t begin)
{
	this->data.erase(this->data.begin() + begin);
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

size_t Dataset::CountSubset(std::vector<std::string> &smallset) const
{
	size_t count(0);
	for(size_t i = 0; i < this->data.size(); i++)
	{
		if(this->data[i].include(smallset))
			count ++;
	}
	return count;
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

void KVDataset::GenerateKVDataset(Dataset &original_dataset, std::vector<Set_Freq_pair> &IS)
{
	this->kvdata.clear();
	for(size_t i = 0; i < IS.size(); i ++)
	{
		Itemset candidate_itemset(IS[i].first);
		string candidate_concat_str = candidate_itemset.ConcatWithOrder();
		size_t count = original_dataset.CountSubset(IS[i].first);
		this->kvdata.push_back(make_pair(candidate_concat_str, count));
	}
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

void KVDataset::Erase(size_t index)
{
	this->kvdata.erase(this->kvdata.begin() + index);
}

KVpair & KVDataset::operator[](size_t i)
{
	return this->kvdata[i];
}
