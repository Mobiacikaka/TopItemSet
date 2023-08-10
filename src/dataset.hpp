#ifndef __DATASET_HPP__
#define __DATASET_HPP__

#include <vector>
#include <string>
#include <bloom.h>

typedef std::pair<std::string, size_t> KVpair;

class Itemset
{
	protected:
		std::vector<std::string> data;

	public:
		Itemset(std::string uncut_str, std::string seperator);
		Itemset(std::vector<std::string> cut_str);
		~Itemset() {}

		void PrintItemset() const;
		size_t GetItemsetSize() const;
		std::string operator[](size_t i) const;
};

class Dataset
{
	protected:
		std::vector<Itemset> data;

	public:
		Dataset();
		~Dataset() {}

		Itemset & operator[](size_t i);

		size_t GetDatasetSize() const;
		void PrintDataset() const;
		void PruneDataset(size_t );

		/* struct bloom * BloomPack(size_t k); */
		/* size_t BloomCheck(struct bloom * blm, size_t k); */
};

class KVDataset
{
	protected:
		std::vector<KVpair> kvdata;

	public:
		KVDataset() {}
		~KVDataset() {}

		KVpair &operator[](size_t);

		void GenerateKVDataset(Dataset &original_dataset);
		size_t GetKVDatasetSize() const;
		void PrintKVDataset() const;
		void SortKVDataset();
		void EraseFrom(size_t index);
};

#endif
