#include "dataset.hpp"

using namespace std;

void Itemset::_init_itemset_with_uncut_string(std::string uncut_str, char del)
{
	// https://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
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

void KVDataset::GenerateKVDataset(Dataset &original_dataset, std::vector<Set_Freq_pair> &IS, std::vector<KVpair> &topk_item_freq)
{
	this->kvdata.clear();

	std::map< std::string, std::vector<size_t> > map_item_to_users;

	for(size_t i = 0; i < original_dataset.GetDatasetSize(); i ++) {
		Itemset itset(original_dataset[i]);
		for(size_t j = 0; j < topk_item_freq.size(); j ++) {
			if(itset.include(topk_item_freq[j].first))
				map_item_to_users[topk_item_freq[j].first].push_back(i);
		}
	}

	for(size_t i = 0; i < IS.size(); i ++) {
		Itemset candset(IS[i].first);
		string minimal_search_item;
		size_t minimal_search_times(UINT32_MAX);
		for(size_t j = 0; j < candset.GetItemsetSize(); j ++) {
			if(map_item_to_users[candset[j]].size() < minimal_search_times) {
				minimal_search_item = candset[j];
				minimal_search_times = map_item_to_users[minimal_search_item].size();
			}
		}
		assert(minimal_search_item.size()); // there must be something
		vector<size_t> &users_list(map_item_to_users[minimal_search_item]);
		size_t count(0);
		for(size_t j = 0; j < users_list.size(); j ++)
			count += static_cast<size_t>(original_dataset[users_list[j]].include(IS[i].first));
		this->kvdata.push_back(make_pair(candset.ConcatWithOrder(), count));
	}
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
