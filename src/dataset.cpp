#include "dataset.hpp"

#include <algorithm>
#include <fstream>
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
