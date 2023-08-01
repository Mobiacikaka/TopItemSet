#ifndef __DATASET_HPP__
#define __DATASET_HPP__

#include <vector>
#include <string>
#include <bloom.h>

class Itemset
{
private:
	std::vector<std::string> data;

protected:

public:
	Itemset(std::string uncut_str, std::string seperator);
	Itemset(std::vector<std::string> cut_str);

	void PrintItemset() const;
	
	~Itemset();
};

class Dataset
{
private:
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

#endif
