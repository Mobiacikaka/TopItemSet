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

		std::vector<std::string> PruneItemset(std::vector<std::string>) const;
		std::string ConcatWithOrder() const;
};

class Dataset
{
	protected:
		std::vector<Itemset> data;

	public:
		Dataset();
		Dataset(std::vector<Itemset> &data);
		~Dataset() {}

		Itemset & operator[](size_t i);

		size_t GetDatasetSize() const;
		void PrintDataset() const;
		void PruneDataset(size_t );
		std::vector<Itemset> PruneItemset(std::vector<std::string>) const;
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
		void GenerateKVDataset(Dataset &original_dataset, bool);
		size_t GetKVDatasetSize() const;
		void PrintKVDataset() const;
		void SortKVDataset();
		void EraseFrom(size_t index);
};

#endif
