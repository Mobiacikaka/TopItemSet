#pragma once

#include <vector>
#include <string>
#include <map>
#include <bloom.h>

// key value pair
typedef std::pair<std::string, size_t> KVpair;

class Itemset
{
	protected:
		std::vector<std::string> data;

	public:
		Itemset(std::string uncut_str, std::string seperator);
		Itemset(std::vector<std::string> cut_str);
		~Itemset();

		std::string operator[](size_t i) const;

		size_t GetItemsetSize() const;
		void PrintItemset() const;
		std::string ConcatenateWithOrder() const;
};

class Dataset
{
	protected:
		std::vector<Itemset> data;

	public:
		Dataset();
		~Dataset();

		Itemset & operator[](size_t i);

		size_t GetDatasetSize() const;
		void PrintDataset() const;
		void PruneDataset(size_t );
};

class HashDataset
{
	private:
		std::string HashItemsetStupid(Itemset &itemset);

	protected:
		std::vector<KVpair> hashdata;
		std::vector<std::pair<std::string, std::vector<Itemset>>> hashstring_combination_data;

		std::string HashItemsetIntoString(Itemset &itemset);

	public:
		HashDataset();
		HashDataset(Dataset &original_dataset);
		~HashDataset();

		KVpair operator[](size_t);

		void ReadOriginalDataset(Dataset &original_dataset);
		size_t GetDatasetSize() const;
		void PrintDataset() const;
		void SortDataset();
		void EraseFromIndexToEnd(size_t i);

		/* struct bloom * BloomPack(size_t k); */
		/* size_t BloomCheck(struct bloom * blm, size_t k); */
};
