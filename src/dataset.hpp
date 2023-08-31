#ifndef __DATASET_HPP__
#define __DATASET_HPP__

#include <vector>
#include <string>
#include <bloom.h>

#define logshit std::clog << "shit" << std::endl;
typedef std::pair<std::string, size_t> KVpair;

class Itemset
{
	protected:
		std::vector<std::string> data;

	public:
		Itemset(std::string uncut_str, char);
		Itemset(std::vector<std::string> cut_str);
		~Itemset() {}

		void PrintItemset() const;
		size_t GetItemsetSize() const;
		std::string operator[](size_t i) const;

		void PruneItemset(std::vector<std::string>) ;
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
		void PruneDataset(size_t begin);
		void PruneDataset(std::vector<std::string>);
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
		void PrintKVDataset(std::ostream &out) const;
		void PrintKVDataset(std::string filename="") const;
		void SortKVDataset();
		void Erase(size_t index);
};

#endif
