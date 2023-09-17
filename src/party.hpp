#ifndef __PARTY_HPP__
#define __PARTY_HPP__

#include "dataset.hpp"
#include <abycore/aby/abyparty.h>
#include <string>
#include <vector>
#include <map>
#include <cryptopp/integer.h>

struct Key {
	struct Pub {
		uint64_t n, y;
	} pub;
	struct Priv {
		uint64_t p, q;
	} priv;
};

class Party
{
private:
	KVDataset kvdataset;
	std::vector<KVpair> shr_dataset;
	void PrintShareDataset(std::ostream &out);
	void PrintShareDataset(std::string filename);

	struct /* ABYParty Parameters */
	{
		e_role role;
		uint16_t port;
		seclvl seclevel;
		uint32_t bitlen;
		uint32_t nthreads;
		e_mt_gen_alg mt_alg;
	};
	std::string address;

	size_t k;
	size_t kbar;
	double eps;
	double p1;
	double eps_em;
	double delta;

	size_t prune_size;
	std::vector<std::string> md5set;
	std::map<std::string, KVpair> md5map;
	void makeMD5set();
	int MakeShareSrv(size_t & index, CSocket * tsocket);
	int MakeShareCli(CSocket * tsocket);

	size_t comparetimes;
	bool compare(KVpair & kv1, KVpair & kv2);
	bool compare(KVpair & kv1, KVpair & kv2, int);

	double get_delta(size_t nr_users);
	double get_delta_q(double delta, size_t kbar, double c);
	double get_T(double delta_q, double eps1, double eps2);
	double get_qi(size_t i, double eps2);
	double gen_laplace(double location, double scale);

	template<class T>
	void erase(std::vector<T> v, size_t i);
	double generate_R(double mass);
	uint64_t RandomDraw(double mass);
	std::vector<size_t> random_draw_output(double eps_em);
	void RandomSelection(std::ostream &out);

	/* std::vector<size_t> topkindex; */
	std::vector<std::string> topkitem;
	std::vector<KVpair> topk_item_freq;
	void MakeTopKPublic();

	const int M = 1000;
	double mu = 0.9;
	struct Key read_key();
	uint64_t encrypt_bit(uint64_t bit, struct Key &key);
	uint64_t decrypt_bit(uint64_t bit, struct Key &key);
	int64_t jacobi(uint64_t bitc, uint64_t p);
	uint64_t power(uint64_t x, uint64_t y, uint64_t p);
	uint64_t get_sizeof_interset_server(std::unique_ptr<CSocket> &, struct Key &, size_t);
	uint64_t get_sizeof_interset_client(std::unique_ptr<CSocket> &, struct Key &, size_t);

	size_t __partition(size_t low, size_t high);
	size_t __select_pivot(size_t low, size_t high);
	void SecurePartition();

	void CalculateTopKItem(Dataset & original_dataset);
	void CalculateTopKItemSet(Dataset & original_dataset);
	void CalculateTopKItemSet_FrequencyEstimate(Dataset & original_dataset);

	std::vector<Set_Freq_pair> IS; // candidate set
	void ConstructCandidateItemSet();

protected:
	void Prune();
	void Merge();
	void Sort();
	// std::vector<size_t> Selection( const size_t k, const size_t kbar, const double epsilon, const double p1, const double eps_em, const double delta);
	void Selection(std::string filename);

public:
	Party() {}
	~Party() {}

	void set_param(e_role role, std::string address,
		uint16_t port, seclvl seclevel, uint32_t bitlen,
		uint32_t nthreads, e_mt_gen_alg mt_alg,
		size_t k, size_t kbar, double eps, double p1,
		double eps_em, double mu);
	void Run();

};

#endif
