#ifndef __DATASET_HPP__
#define __DATASET_HPP__

#include <vector>
#include <string>
#include <algorithm>
#include <algorithm>
#include <fstream>
#include <map>
#include <iostream>
#include <cassert>
#include <bits/stdc++.h>
#include <bloom.h>

typedef std::pair<std::string, size_t> KVpair;
typedef std::pair<std::vector<std::string>, double> Set_Freq_pair;

class Comparator {
	public:
		bool operator()(const KVpair &a, const KVpair &b) { return a.second < b.second; }
		bool operator()(const Set_Freq_pair &a, const Set_Freq_pair &b) { return a.second < b.second; }
};

class Itemset
{
	private:
		void _init_itemset_with_uncut_string(std::string uncut_str, char del);

	protected:
		std::vector<std::string> data;

	public:
		Itemset(std::string uncut_str, char del=' ') { this->_init_itemset_with_uncut_string(uncut_str, del); }

		Itemset(std::vector<std::string> cut_str) { this->data = cut_str; }

		~Itemset() {}

		std::string operator[](size_t i) const { return this->data[i]; }

		std::string ConcatWithOrder() const {
			std::string ccstr;
			for(size_t i = 0; i < this->data.size(); i ++)
				ccstr = ccstr + this->data[i] + ",";
			return ccstr;
		}

		size_t GetItemsetSize() const { return this->data.size(); }

		void PruneItemset(std::vector<std::string>) ;

		bool include(std::vector<std::string> &smallset) const {
			return std::includes(
				this->data.begin(),
				this->data.end(),
				smallset.begin(),
				smallset.end()
			);
		}

		bool include(std::string id) const {
			return std::find(
				this->data.begin(),
				this->data.end(),
				id
			) != this->data.end();
		}

		void PrintItemset() const {
			for(size_t i = 0; i < this->data.size(); i ++)
				std::cout << this->data[i] << ",";
			std::cout << std::endl;
		}
};

class Dataset
{
	protected:
		std::vector<Itemset> data;

	public:
		Dataset() {
			std::string uncut_itemset_str;
			while(getline(std::cin, uncut_itemset_str, '\n'))
				this->data.push_back(uncut_itemset_str);
		}

		Dataset(std::vector<Itemset> &data) { this->data = data; }

		~Dataset() {}

		Itemset & operator[](size_t i) { return this->data[i]; }

		void PrintDataset() const {
			for(size_t i = 0; i < this->data.size(); i ++)
				this->data[i].PrintItemset();
		}

		void PruneDataset(size_t begin) { this->data.erase(this->data.begin() + begin); }

		void PruneDataset(std::vector<std::string>);

		size_t GetDatasetSize() const { return this->data.size(); }

		size_t CountSubset(std::vector<std::string> &smallset) const {
			size_t count(0);
			for(size_t i = 0; i < this->data.size(); i ++)
				count += static_cast<size_t>(this->data[i].include(smallset));
			return count;
		}

		size_t CountItem(std::string id) const {
			size_t count(0);
			for(size_t i = 0; i < this->data.size(); i ++)
				count += static_cast<size_t>(this->data[i].include(id));
			return count;
		}
};

class KVDataset
{
	protected:
		std::vector<KVpair> kvdata;

	public:
		KVDataset() {}
		~KVDataset() {}

		KVpair &operator[](size_t i) { return this->kvdata[i]; }
		size_t GetKVDatasetSize() const { return this->kvdata.size(); }

		void GenerateKVDataset(Dataset &original_dataset);
		void GenerateKVDataset(Dataset &original_dataset, bool);
		void GenerateKVDataset(Dataset &original_dataset, std::vector<Set_Freq_pair> &IS, std::vector<KVpair> &topk_item_freq);
		void PrintKVDataset(std::ostream &out) const {
			for(auto it = this->kvdata.begin(); it < this->kvdata.end(); it ++)
				out << it->first << "\t" << it->second << std::endl;
		}
		void PrintKVDataset(std::string filename="") const;
		void SortKVDataset() {
			sort(
				this->kvdata.begin(),
				this->kvdata.end(),
				[](const KVpair &a, const KVpair &b) { return a.second > b.second; }
			);
		}
		void Erase(size_t index) { this->kvdata.erase(this->kvdata.begin() + index); }
};

#endif
