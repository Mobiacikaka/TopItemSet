#include "dataset.hpp"

#include <algorithm>
#include <fstream>
#include <map>
#include <iostream>
using namespace std;

Itemset::Itemset(std::string uncut_str, std::string seperator=",")
{
	size_t begin(0), end(0);
	while(end < uncut_str.size())
	{
		end = uncut_str.find(seperator);
		if(end != std::string::npos)
		{
			data.push_back(uncut_str.substr(begin, end-begin));
		}
		else
		{
			data.push_back(uncut_str.substr(begin));
		}
		begin = end;
		end += 1;
	}
}

Itemset::Itemset(std::vector<std::string> cut_str)
{
	this->data = cut_str;
}

void Itemset::PrintItemset() const
{
	size_t length = this->data.size();
	for (size_t i = 0; i < length - 1; i ++)
	{
		cout << this->data[i] << ",";
	}
	if(length > 0)
	{
		cout << this->data[length - 1] << endl;
	}
}

size_t Itemset::GetItemsetSize() const
{
	return this->data.size();
}

string Itemset::operator[](size_t i) const
{
	return this->data[i];
}

Dataset::Dataset()
{
	size_t num_users;
	cin >> num_users;
	for(size_t i = 0; i < num_users; i ++)
	{
		string uncut_itemset_str;
		cin >> uncut_itemset_str;
		Itemset itemset(uncut_itemset_str);
		this->data.push_back(itemset);
	}
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

void Dataset::PruneDataset(size_t retainsize)
{
	this->data.erase(this->data.begin() - retainsize, this->data.end());
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
		for (size_t j = 0; j < itemset.GetItemsetSize(); i++)
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

size_t KVDataset::GetKVDatasetSize() const
{
	return this->kvdata.size();
}

void KVDataset::PrintKVDataset() const
{
	for(auto it = this->kvdata.begin(); it < this->kvdata.end(); it++)
		cout << it->first << "\t" << it->second << endl;
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
