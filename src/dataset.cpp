#include "dataset.hpp"
#include "MurmurHash3.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <cassert>
#include <cryptopp/md5.h>
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
	/* sort(data.begin(), data.end()); */
}

Itemset::Itemset(std::vector<std::string> cut_str)
{
	this->data = cut_str;
	/* sort(data.begin(), data.end()); */
}

Itemset::~Itemset()
{
}

string Itemset::operator[](size_t i) const
{
	return this->data[i];
}

size_t Itemset::GetItemsetSize() const
{
	return this->data.size();
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

string Itemset::ConcatenateWithOrder() const
{
	string constr;
	vector<string> sorteddata(data);
	sort(sorteddata.begin(), sorteddata.end());
	for(size_t i = 0; i < sorteddata.size(); i++)
	{
		constr += sorteddata[i];
	}
	return constr;
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

Dataset::~Dataset()
{
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

string HashDataset::HashItemsetStupid(Itemset &itemset)
{
	using namespace CryptoPP;

	std::string constr = itemset.ConcatenateWithOrder();
	std::string digest;
	Weak1::MD5 _hash;

	_hash.Update((const byte *)&constr[0], constr.size());
	digest.resize(_hash.DigestSize());
	_hash.Final((byte*)&digest[0]);

	return digest;
}

string HashDataset::HashItemsetIntoString(Itemset &itemset)
{
	return this->HashItemsetStupid(itemset);
}

HashDataset::HashDataset()
{
}

HashDataset::HashDataset(Dataset &original_dataset)
{
	map<string, size_t> hashdata_map;
	for(size_t i = 0; i < original_dataset.GetDatasetSize(); i++)
	{
		size_t _N(original_dataset[i].GetItemsetSize());

		for(size_t _K = 1; _K < _N; _K++)
		{
			// Combination(N, K)
			// https://stackoverflow.com/questions/12991758/creating-all-possible-k-combinations-of-n-items-in-c
			string bitmask(_K, 1);
			bitmask.resize(_N, 0);

			do {
				// list combination
				vector<string> combination;
				for (size_t j = 0; j < _N; j++)
				{
					if (bitmask[j]) combination.push_back(original_dataset[i][j]);
				}
				// hash combination into string and add it to this->hashdata
				Itemset combination_itemset(combination);
				string hashstring = this->HashItemsetIntoString(combination_itemset);
				// initialize as 0 if hashstring does not exist in hashdata_map
				hashdata_map[hashstring] += 1;
			} while (prev_permutation(bitmask.begin(), bitmask.end()));
		}
	}

	// convert map to vector
	// https://www.techiedelight.com/convert-map-vector-key-value-pairs-cpp/
	std::transform(
		hashdata_map.begin(),
		hashdata_map.end(),
		std::back_inserter(this->hashdata),
		[](const std::pair<string, size_t> &p) {
			return p;
		}
	);
}

HashDataset::~HashDataset()
{
}
