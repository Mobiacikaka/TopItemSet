#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "party.hpp"
#include "MurmurHash3.h"

#include <cassert>
#include <algorithm>
#include <queue>
#include <random>
#include <cmath>
#include <iostream>
#include <ctime>
#include <ENCRYPTO_utils/socket.h>
#include <ENCRYPTO_utils/connection.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/sharing/sharing.h>
#include <cryptopp/md5.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/integer.h>

using namespace std;

const vector<int> H = {1,2,3,4,5};


const size_t prune_times = 3;
#define MASK SIZE_MAX

void Party::set_param(
	e_role role,
	std::string address,
	uint16_t port,
	seclvl seclevel,
	uint32_t bitlen,
	uint32_t nthreads,
	e_mt_gen_alg mt_alg,
	size_t k,
	size_t kbar,
	double eps,
	double p1,
	double eps_em,
	double mu
)
{
	this->role = role;
	this->address = address;
	this->port = port;
	this->seclevel = seclevel;
	this->bitlen = bitlen;
	this->nthreads = nthreads;
	this->mt_alg = mt_alg;

	this->k = k;
	this->kbar = kbar >= k ? kbar : k;
	// WARNING
	// the input eps is a sum for item and itemset mining
	// two operations share a same amount of privacy consumption
	this->eps = eps / 2;
	this->p1 = p1;
	this->eps_em = eps_em;
	this->mu = mu;

	this->comparetimes = 0;
}

void Party::PrintShareDataset(std::ostream &out)
{
	for(size_t i = 0; i < shr_dataset.size(); i ++)
		out << shr_dataset[i].first << "\t" << shr_dataset[i].second << endl;
}

void Party::PrintShareDataset(std::string filename="")
{
	ofstream file;
	if(filename.empty()) {
		this->PrintShareDataset(std::cout);
	}
	else {
		file.open(filename);
		if(file.is_open() == false) {
			cerr << filename << " open failed." << endl;
			exit(-1);
		}
		this->PrintShareDataset(file);
	}
}

double Party::get_delta(size_t nr_users)
{
	unique_ptr<CSocket> tsocket;
	double delta;

	if(role == SERVER) {
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed!" << endl;
			exit(1);
		}

		size_t nr_users_cli;
		tsocket->Send((void *)&nr_users, sizeof(nr_users));
		tsocket->Receive((void *)&nr_users_cli, sizeof(nr_users));
		delta = 2.0 / (nr_users + nr_users_cli);
	}
	else {
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed!" << endl;
			exit(1);
		}

		size_t nr_users_srv;
		tsocket->Receive((void *)&nr_users_srv, sizeof(nr_users_srv));
		tsocket->Send((void *)&nr_users, sizeof(nr_users));
		delta = 2.0 / (nr_users + nr_users_srv);
	}

	tsocket->Close();
	return delta;
}

void Party::CalculateTopKItem(Dataset & original_dataset)
{
	clog << "Running CalculateTopKItem" << endl;

	delta = this->get_delta(original_dataset.GetDatasetSize());
	this->kvdataset.GenerateKVDataset(original_dataset);
	this->kvdataset.SortKVDataset();

	clog << "Prune Start" << endl;
	this->Prune();
	this->kvdataset.PrintKVDataset("item_0_prune.out");
	clog << "Prune Finished" << endl;

	clog << "Merge Start" << endl;
	this->Merge();
	this->PrintShareDataset("item_1_merge.out");
	clog << "Merge Finished" << endl;

	clog << "Sort Start" << endl;
	this->Sort();
	this->PrintShareDataset("item_2_sort.out");
	clog << "Sort Finished" << endl;

	clog << "Selection Start" << endl;
	this->Selection();
	this->PrintShareDataset("item_3_select.out");
	clog << "Selection Finished" << endl;
}

void Party::CalculateTopKItemSet(Dataset & original_dataset)
{
	clog << endl << "Running CalculateTopKItemSet" << endl;

	/* Top K Item Set Selection */
	/* this->PrintShareDataset(std::cout); */
	this->MakeTopKPublic();
	for(size_t i = 0; i < this->topkitem.size(); i++)
		cout << this->topkitem[i] << "\t"; cout << endl;
	assert(this->topkitem.size());
	original_dataset.PruneDataset(this->topkitem);
	/* original_dataset.PrintDataset(); */

	this->kvdataset.GenerateKVDataset(original_dataset, true);
	this->kvdataset.SortKVDataset();
	/* this->kvdataset.PrintKVDataset(std::cout); */
	delta = this->get_delta(original_dataset.GetDatasetSize());

	clog << "Prune Start" << endl;
	this->Prune();
	this->kvdataset.PrintKVDataset("itemset_0_prune.out");
	clog << "Prune Finished" << endl;

	clog << "Merge Start" << endl;
	this->Merge();
	this->PrintShareDataset("itemset_1_merge.out");
	clog << "Merge Finished" << endl;

	clog << "Sort Start" << endl;
	this->Sort();
	this->PrintShareDataset("itemset_2_sort.out");
	clog << "Sort Finished" << endl;

	clog << "Selection Start" << endl;
	this->Selection();
	this->PrintShareDataset("itemset_3_select.out");
	clog << "Selection Finished" << endl;
}

void Party::CalculateTopKItemSet_FrequencyEstimate(Dataset & original_dataset)
{
	clog << endl << "Running CalculateTopKItemSet" << endl;

	this->MakeTopKPublic();
	assert(this->topkitem.size());
	/* original_dataset.PruneDataset(this->topkitem); */
	this->ConstructCandidateItemSet();

	// Generate KVDataset by frequency estimation
	this->kvdataset.GenerateKVDataset(original_dataset, this->IS);
	this->kvdataset.SortKVDataset();
	delta = this->get_delta(original_dataset.GetDatasetSize());

	clog << "Merge Start" << endl;
	this->Merge();
	this->PrintShareDataset("itemset_0_merge.out");
	clog << "Merge Finished" << endl;

	clog << "Sort Start" << endl;
	this->Sort();
	this->PrintShareDataset("itemset_1_sort.out");
	clog << "Sort Finished" << endl;

	clog << "Selection Start" << endl;
	this->Selection();
	this->PrintShareDataset("itemset_2_select.out");
	clog << "Selection Finished" << endl;
}

void Party::Run()
{
	Dataset original_dataset;
	/* original_dataset.PrintDataset(); */

	this->CalculateTopKItem(original_dataset);
	/* this->CalculateTopKItemSet(original_dataset); */
	this->CalculateTopKItemSet_FrequencyEstimate(original_dataset);
}


struct Key Party::read_key() {
	ifstream f("key.txt");
	if(!f.is_open())
	{
		clog << "key.txt not exist" << endl;
		exit(0);
	}
	struct Key key;
	f >> key.pub.n;
	f >> key.pub.y;
	f >> key.priv.p;
	f >> key.priv.q;
	f.close();
	return key;
}


uint64_t integer_mulmod(uint64_t x, uint64_t y, uint64_t p) {
	CryptoPP::Integer _x(x), _y(y), _p(p), res;
	res = (_x * _y) % _p;
	return res.ConvertToLong();
}


uint64_t Party::power(uint64_t x, uint64_t y, uint64_t p) {
	uint64_t res = 1;
	x %= p;

	if(x == 0) return 0;

	while(y > 0) {
		if((y & 1) == 1)
			res = integer_mulmod(res, x, p);
		y = y >> 1;
		x = integer_mulmod(x, x, p);
	}

	return res;
}


uint64_t Party::encrypt_bit(uint64_t bit, struct Key &key) {
	uint64_t n(key.pub.n), y(key.pub.y);
	uint64_t x = rand() % n;
	if(bit) {
		return (y * this->power(x, 2, n)) % n;
		return integer_mulmod(y, this->power(x, 2, n), n);
	}
	return power(x, 2, n);
}


int64_t Party::jacobi(uint64_t a, uint64_t n) {
	if(a == 0) return 0;
	if(a == 1) return 1;

	uint64_t e = 0;
	uint64_t a1 = a;
	while(a1 % 2 == 0)
	{
		e += 1;
		a1 /= 2;
	}
	assert(pow(2, e) * a1 == a);

	int64_t s = 0;
	if(e % 2 == 0) s = 1;
	else if(n % 8 == 1 || n % 8 == 7) s = 1;
	else if(n % 8 == 3 || n % 8 == 5) s = -1;

	if(n % 4 == 3 && a1 % 4 == 3) s *= -1;

	uint64_t n1 = n % a1;
	if(a1 == 1) return s;
	return s * jacobi(n1, a1);
}


uint64_t Party::decrypt_bit(uint64_t bitc, struct Key &key) {
	uint64_t p(key.priv.p), q(key.priv.q);
	assert(p && q);
	int64_t e = jacobi(bitc, p);
	if(e == 1) return 0;
	return 1;
}


uint64_t Party::get_sizeof_interset_server(std::unique_ptr<CSocket> &tsocket, struct Key &key_A, size_t prune_size) {
	prune_size = prune_size > kvdataset.GetKVDatasetSize() ? kvdataset.GetKVDatasetSize() : prune_size;

	// step 1
	vector<uint32_t> blm = vector<uint32_t>(this->M, 1);
	for(int i = 0; i < prune_size && i < kvdataset.GetKVDatasetSize(); i ++) {
		KVpair item(kvdataset[i]);
		for(int j = 0; j < H.size(); j ++)
		{
			uint32_t hv;
			MurmurHash3_x86_32(item.first.c_str(), item.first.size(), H[j], &hv);
			hv %= M;
			blm[hv] = 0;
		}
	}

	for(int i = 0; i < M; i ++) {
		uint64_t enc = this->encrypt_bit(blm[i], key_A);
		tsocket->Send((void *)&enc, sizeof(enc));
	}

	// step 2
	vector<vector<uint64_t>> EB_list;
	for(int i = 0; i < prune_size && i < kvdataset.GetKVDatasetSize(); i ++)
	{
		vector<uint64_t> EB;
		for(int j = 0; j < H.size(); j ++)
		{
			uint64_t value;
			tsocket->Receive((void *)&value, sizeof(value));
			EB.push_back(value);
		}
		EB_list.push_back(EB);
	}

	// step 3
	int c(0);
	for(int i = 0; i < EB_list.size(); i ++)
	{
		bool flag(true);
		for(int j = 0; j < EB_list[i].size(); j ++)
		{
			uint64_t d = this->decrypt_bit(EB_list[i][j], key_A);
			if(d != 0) flag = false;
		}
		if(flag) c += 1;
	}
	tsocket->Send((void *)&c, sizeof(c));
	return c;
}


uint64_t Party::get_sizeof_interset_client(std::unique_ptr<CSocket> &tsocket, struct Key &key_A, size_t prune_size) {
	prune_size = prune_size > kvdataset.GetKVDatasetSize() ? kvdataset.GetKVDatasetSize() : prune_size;

	// step 1
	vector<uint64_t> cblm;
	for(int i = 0; i < M; i ++)
	{
		uint64_t cipher;
		tsocket->Receive((void *)&cipher, sizeof(cipher));
		cblm.push_back(cipher);
	}

	// step 2
	for(int i = 0; i < prune_size && i < kvdataset.GetKVDatasetSize(); i ++)
	{
		KVpair item(kvdataset[i]);
		for(int j = 0; j < H.size(); j ++)
		{
			uint32_t hv(0);
			MurmurHash3_x86_32(item.first.c_str(), item.first.size(), H[j], &hv);
			hv %= M;
			uint64_t bh = cblm[hv];
			uint64_t qrm = this->encrypt_bit(0, key_A);
			uint64_t value = integer_mulmod(bh, qrm, key_A.pub.n);
			tsocket->Send((void *)&value, sizeof(value));
		}
	}

	// step 3
	int c(0);
	tsocket->Receive((void *)&c, sizeof(c));
	return c;
}


void Party::Prune()
{
	size_t i;
	struct bloom * blm;
	unique_ptr<CSocket> tsocket;
	size_t nr_interset;

	assert(this->role == SERVER || this->role == CLIENT);

	if(role == SERVER)
	{
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed!" << endl;
			exit(1);
		}

		struct Key key_A = this->read_key();
		tsocket->Send((void *)&(key_A.pub), sizeof(key_A.pub));

		for(i = 0; i < prune_times; i ++)
		{
			size_t prune_size = kbar * pow(2, i);
			nr_interset = this->get_sizeof_interset_server(tsocket, key_A, prune_size);
			if(nr_interset * 1.0 / kbar >= this->mu) break;
		}

		tsocket->Close();
	}
	else
	{
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed!" << endl;
			exit(1);
		}

		struct Key key_A;
		tsocket->Receive((void *)&(key_A.pub), sizeof(key_A.pub));

		for(i = 0; i < prune_times; i ++)
		{
			size_t prune_size = kbar * pow(2, i);
			nr_interset = this->get_sizeof_interset_client(tsocket, key_A, prune_size);
			/* clog << nr_interset << endl; */
			if(nr_interset * 1.0 / kbar >= this->mu) break;
		}

		tsocket->Close();
	}

	if(i >= prune_times)
		prune_size = kbar * pow(2, i-1) + 1;
	else
		prune_size = kbar * pow(2, i) + 1;
	if(prune_size > this->kvdataset.GetKVDatasetSize())
		prune_size = this->kvdataset.GetKVDatasetSize();
}

void Party::makeMD5set()
{
	this->md5set.clear();
	this->md5map.clear();
	assert(this->md5set.size() == 0 && this->md5map.size() == 0);
	using namespace CryptoPP;
	for(size_t i = 0; i < kvdataset.GetKVDatasetSize(); i ++) {
		string ID = kvdataset[i].first;
		string digest;
		Weak1::MD5 hash;

		hash.Update((const CryptoPP::byte*)&ID[0], ID.size());
		digest.resize(hash.DigestSize());
		hash.Final((CryptoPP::byte*)&digest[0]);

		md5set.push_back(digest);
		md5map[digest] = kvdataset[i];
	}
}

int Party::MakeShareSrv(size_t & index, CSocket * tsocket)
{
	int shr_rnd;
	string str = md5set[index];

	tsocket->Send((void *)str.c_str(), str.size());
	tsocket->Receive((void *)&shr_rnd, sizeof(shr_rnd));

	return role == SERVER ? shr_rnd + kvdataset[index].second : shr_rnd - kvdataset[index].second;
}

int Party::MakeShareCli(CSocket * tsocket)
{
	// Part 1: decode the message send from server
	string encoded;
	encoded.resize(16);
	tsocket->Receive((void *)&encoded[0], encoded.size());

	// Part 2: check the local kvdataset and find the same one
	auto i = find(md5set.begin(), md5set.end(), encoded);
	size_t index = i - md5set.begin();

	int shr_rnd = rand() & MASK;
	tsocket->Send((void *)&shr_rnd, sizeof(shr_rnd));
	int rtn = role == SERVER ? shr_rnd + kvdataset[index].second : shr_rnd - kvdataset[index].second;

	if(index < kvdataset.GetKVDatasetSize())
	{
		if(index < prune_size) prune_size --;
		this->kvdataset.Erase(index);
		md5set.erase(md5set.begin() + index);
	}

	return rtn;
}

void Party::Merge()
{
	unique_ptr<CSocket> tsocket;
	makeMD5set();
	this->shr_dataset.clear();

	size_t len;

	if(role == SERVER)
	{
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed!" << endl;
			exit(1);
		}

		len = prune_size < this->kvdataset.GetKVDatasetSize() ? prune_size : this->kvdataset.GetKVDatasetSize();
		tsocket->Send((void *)&len, sizeof(len));
		for(size_t i = 0; i < len; i ++)
		{
			KVpair tmp_kv(kvdataset[i].first, MakeShareSrv(i, tsocket.get()));
			shr_dataset.push_back(tmp_kv);
		}

		tsocket->Receive((void *)&len, sizeof(len));
		for(size_t i = 0; i < len; i ++) {
			KVpair tmp_kv("", MakeShareCli(tsocket.get()));
			shr_dataset.push_back(tmp_kv);
		}
	}
	else {
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed!" << endl;
			exit(1);
		}

		tsocket->Receive((void *)&len, sizeof(len));
		for(size_t i = 0; i < len; i ++)
		{
			KVpair tmp_kv("", MakeShareCli(tsocket.get()));
			shr_dataset.push_back(tmp_kv);
		}

		len = prune_size;
		tsocket->Send((void *)&len, sizeof(len));
		for(size_t i = 0; i < len; i++)
		{
			KVpair tmp_kv(kvdataset[i].first, MakeShareSrv(i, tsocket.get()));
			shr_dataset.push_back(tmp_kv);
		}
	}

	tsocket->Close();
}

// non-Secure compare
bool Party::compare(KVpair & kv1, KVpair & kv2)
{
	this->comparetimes ++;

	unique_ptr<CSocket> tsocket;
	bool flag(false);
	if(role == SERVER) {
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed!" << endl;
			exit(1);
		}

		int count1, count2;
		tsocket->Receive((void *)&count1, sizeof(count1));
		tsocket->Receive((void *)&count2, sizeof(count2));

		flag = kv1.second - count1 > kv2.second - count2;
		tsocket->Send((void *)&flag, sizeof(flag));
	}
	else {
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed!" << endl;
			exit(1);
		}

		int rnd = rand() & MASK;
		int count1, count2;
		count1 = kv1.second - rnd;
		count2 = kv2.second - rnd;
		tsocket->Send((void *)&count1, sizeof(count1));
		tsocket->Send((void *)&count2, sizeof(count2));

		tsocket->Receive((void *)&flag, sizeof(flag));
	}
	tsocket->Close();

	return flag;
}

// Secure compare
bool Party::compare(KVpair & kv1, KVpair & kv2, int)
{
	this->comparetimes ++;

	ABYParty * party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg);
	vector<Sharing*> sharings = party->GetSharings();
	BooleanCircuit * circ = (BooleanCircuit*) sharings[S_BOOL]->GetCircuitBuildRoutine();

	share *srv1, *srv2, *cli1, *cli2;
	if(role == SERVER) {
		srv1 = circ->PutINGate(static_cast<uint32_t>(kv1.second), bitlen, role);
		srv2 = circ->PutINGate(static_cast<uint32_t>(kv2.second), bitlen, role);
		cli1 = circ->PutDummyINGate(bitlen);
		cli2 = circ->PutDummyINGate(bitlen);
	}
	else {
		srv1 = circ->PutDummyINGate(bitlen);
		srv2 = circ->PutDummyINGate(bitlen);
		cli1 = circ->PutINGate(static_cast<uint32_t>(kv1.second), bitlen, role);
		cli2 = circ->PutINGate(static_cast<uint32_t>(kv2.second), bitlen, role);
	}

	share *cmb1, *cmb2, *shr_cmp, *shr_out;
	cmb1 = circ->PutSUBGate(srv1, cli1);
	cmb2 = circ->PutSUBGate(srv2, cli2);
	shr_cmp = circ->PutGTGate(cmb1, cmb2);
	shr_out = circ->PutOUTGate(shr_cmp, ALL);

	party->ExecCircuit();

	uint32_t output = shr_out->get_clear_value<uint32_t>();
	assert(output == 0 || output == 1);

	delete party;
	delete srv1, srv2, cli1, cli2;
	delete cmb1, cmb2, shr_cmp, shr_out;

	return output;
}

// True if a > b
#define compare(a, b) \
	compare((a), (b))

#define EXCHANGE(a, b) \
	{ \
		KVpair tmp = (a); \
		(a) = (b); \
		(b) = tmp; \
	}

size_t Party::__select_pivot(size_t low, size_t high)
{
	size_t c1 = low, c2 = high, c3 = static_cast<size_t>((low+high)/2);
	size_t med(0);
	if(compare(shr_dataset[c1], shr_dataset[c2])) {
		if(compare(shr_dataset[c3], shr_dataset[c1])) med = c1;
		else if(compare(shr_dataset[c2], shr_dataset[c3])) med = c2;
		else med = c3;
	}
	else {
		if(compare(shr_dataset[c3], shr_dataset[c2])) med = c2;
		else if (compare(shr_dataset[c1], shr_dataset[c3])) med = c1;
		else med = c3;
	}
	return med;
}

size_t Party::__partition(size_t low, size_t high)
{
	size_t pivot = this->__select_pivot(low, high);
	while(low < high) {
		while(low < high && !compare(shr_dataset[high], shr_dataset[pivot])) high --;
		while(low < high && !compare(shr_dataset[pivot], shr_dataset[low])) low ++;
		EXCHANGE(shr_dataset[low], shr_dataset[high])
	}
	return low;
}

void Party::SecurePartition()
{
	const size_t len = shr_dataset.size();
	size_t low(0), high(len-1);
	while(true) {
		low = this->__partition(low, high);
		if(low >= this-> kbar+1) break;
	}
	shr_dataset.erase(shr_dataset.begin() + low, shr_dataset.end());
}

void Party::Sort()
{
	const size_t len = shr_dataset.size();
	const size_t kbar = this->kbar + 1;

	// Construct Heap
	for(size_t i = len-1; i > 0; i --) {
		size_t top = (i - 1) / 2;
		if(compare(shr_dataset[i], shr_dataset[top])) {
			EXCHANGE(shr_dataset[i], shr_dataset[top]);
			top = i;
			while(top < len/2) {
				size_t left = 2 * top + 1;
				size_t right = 2 * top + 2;
				size_t exc;
				if(right >= len) exc = left;
				else if(compare(shr_dataset[left], shr_dataset[right])) exc = left;
				else exc = right;

				if(compare(shr_dataset[exc], shr_dataset[top]))
					EXCHANGE(shr_dataset[top], shr_dataset[exc]);

				top = exc;
			}
		}
	}

	// Pop Heap
	for(size_t i = 0; i < kbar; i ++) {
		EXCHANGE(shr_dataset[0], shr_dataset[len-i-1]);

		size_t top = 0;
		while(top < (len-i-1)/2)
		{
			if(log2(top+1) >= kbar-i) break;
			size_t left = 2 * top + 1;
			size_t right = 2 * top + 2;
			size_t exc;
			if(right >= len - i - 1) exc = left;
			else if(compare(shr_dataset[left], shr_dataset[right])) exc = left;
			else exc = right;

			if(compare(shr_dataset[exc], shr_dataset[top]))
				EXCHANGE(shr_dataset[top], shr_dataset[exc]);

			top = exc;
		}
	}

	reverse(shr_dataset.begin(), shr_dataset.end());
	shr_dataset.erase(shr_dataset.begin() + kbar, shr_dataset.end());
}

double Party::gen_laplace(double location, double scale)
{
	uniform_real_distribution<> values {-0.5, 0.5};
	random_device rd;
	default_random_engine rng {rd()};
	double rnd = values(rng);
	double res;

	res = scale * ((rnd > 0) - (rnd < 0)) * log(1 - 2*abs(rnd));
	return res;
}

double Party::get_delta_q(double delta, size_t kbar, double c)
{
	double delta_q = delta;
	double delta_max;

	delta_max = kbar * ( ( 2*pow(delta_q, c) + delta_q - c*(pow(delta_q, c) + 2*delta_q) )/(4 - 4*c) );
	while(delta_max > delta)
	{
		delta_q = delta_q * 0.99;
		delta_max = kbar * ( ( 2*pow(delta_q, c) + delta_q - c*(pow(delta_q, c) + 2*delta_q) )/(4 - 4*c) );
	}

	return delta_q;
}

double Party::get_T(double delta_q, double eps1, double eps2)
{
	unique_ptr<CSocket> tsocket;

	double T;
	if(role == SERVER) {
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed" << endl;
			exit(1);
		}

		T = log( 1/delta_q ) / (eps2 / 2) + gen_laplace(0, 1/eps1);
		tsocket->Send((void *)&T, sizeof(T));
		tsocket->Close();
	}
	else {
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed" << endl;
			exit(1);
		}

		tsocket->Receive((void *)&T, sizeof(T));
		tsocket->Close();
	}

	return T;
}

double Party::get_qi(size_t i, double eps2)
{
	ABYParty * party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg, 4000000);
	vector<Sharing*> sharings = party->GetSharings();
	BooleanCircuit * bcirc = (BooleanCircuit *) sharings[S_BOOL]->GetCircuitBuildRoutine();

	share *srv_i, *cli_i, *srv_j, *cli_j;
	if(role == SERVER)
	{
		srv_i = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[i].second), bitlen, role);
		srv_j = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[i+1].second), bitlen, role);
		cli_i = bcirc->PutDummyINGate(bitlen);
		cli_j = bcirc->PutDummyINGate(bitlen);
	}
	else
	{
		srv_i = bcirc->PutDummyINGate(bitlen);
		srv_j = bcirc->PutDummyINGate(bitlen);
		cli_i = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[i].second), bitlen, role);
		cli_j = bcirc->PutINGate(static_cast<uint32_t>(shr_dataset[i+1].second), bitlen, role);
	}

	share *cmb_i, *cmb_j, *cmb_dif, *shr_out;
	cmb_i = bcirc->PutSUBGate(srv_i, cli_i);
	cmb_j = bcirc->PutSUBGate(srv_j, cli_j);
	cmb_dif = bcirc->PutSUBGate(cmb_i, cmb_j);
	shr_out = bcirc->PutOUTGate(cmb_dif, ALL);

	party->ExecCircuit();

	int output = static_cast<int32_t>(shr_out->get_clear_value<uint32_t>());

	delete party;
	delete srv_i, srv_j, cli_i, cli_j;
	delete cmb_i, cmb_j, cmb_dif, shr_out;

	double qi = max(output - 1, 0);
	double qi_n;
	if(role == SERVER)
	{
		unique_ptr<CSocket> tsocket;
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed!" << endl;
			exit(1);
		}

		qi_n = qi + gen_laplace(0, 1/eps2);
		tsocket->Send((void*)&qi_n, sizeof(qi_n));
		tsocket->Close();
	}
	else
	{
		unique_ptr<CSocket> tsocket;
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed" << endl;
			exit(1);
		}

		tsocket->Receive((void*)&qi_n, sizeof(qi_n));
		tsocket->Close();
	}

	return qi_n;
}

double Party::generate_R(double mass)
{
	double mass2;

	unique_ptr<CSocket> tsocket;
	if(role == SERVER) {
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed" << endl;
			exit(1);
		}

		tsocket->Send((void*)&mass, sizeof(mass));
		tsocket->Receive((void *)&mass2, sizeof(mass2));
	}
	else {
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed" << endl;
			exit(1);
		}

		tsocket->Receive((void *)&mass2, sizeof(mass2));
		tsocket->Send((void *)&mass, sizeof(mass));
	}

	tsocket->Close();
	return mass + mass2;
}

uint64_t Party::RandomDraw(double mass)
{
	double R = generate_R(mass);
	uint64_t M = (uint64_t)R;
	uint64_t mask = 0;

	for(size_t i = 63; i >=0; i --) {
		if(M >> i) {
			mask = (1 << ++i) - 1;
			break;
		}
	}

	unique_ptr<CSocket> tsocket;
	uint64_t xrnd;
	if(role == SERVER) {
		tsocket = Listen(address, role);
		if(!tsocket) {
			cerr << "Listen Failed" << endl;
			exit(1);
		}

		while(true) {
			uint64_t rnd1 = rand();
			uint64_t rnd2 = rand();
			xrnd = rnd1 ^ rnd2;
			xrnd &= mask;

			if(xrnd < M) break;
		}

		tsocket->Send((void *)&xrnd, sizeof(xrnd));
	}
	else {
		tsocket = Connect(address, role);
		if(!tsocket) {
			cerr << "Connect Failed" << endl;
			exit(1);
		}

		tsocket->Receive((void *)&xrnd, sizeof(xrnd));
	}
	tsocket->Close();

	return xrnd;
}

template<class T>
void Party::erase(vector<T> v, size_t i)
{
	size_t len = v.size();
	v.erase(v.begin() + i);
	if(i >= len/2)
		v.erase(v.end() - i);
	else
		v.erase(v.end() - i - 1);
}

vector<size_t> Party::random_draw_output(double eps_em)
{
	// Selection Probability Calculate
	vector<int> shr_dataset;
	vector<int> gap;
	vector<double> mass;
	vector<size_t> nr;

	shr_dataset.resize(this->shr_dataset.size() * 2);
	size_t length(shr_dataset.size());
	for(size_t i = 0; i < this->shr_dataset.size(); i ++)
		shr_dataset[i] = this->shr_dataset[i].second;
	for(size_t i = length; i < length * 2; i ++)
		shr_dataset[i] = shr_dataset[2*length - i - 1];

	gap.resize(length);
	mass.resize(length);

	for(size_t i = 0; i < length / 2; i ++)
		nr[i] = nr[length - i - 1] = i;

	size_t middle(length / 2);
	for(size_t i = 0; i < length; i ++)
	{
		if(i == 0)
			gap[i] = static_cast<int>(shr_dataset[i] - 0);
		else if(i < middle)
			gap[i] = static_cast<int>(shr_dataset[i] - shr_dataset[i-1]);
		else if(i < length - 1)
			gap[i] = static_cast<int>(shr_dataset[i] - shr_dataset[i+1]);
		else
			gap[i] = static_cast<int>(shr_dataset[i]);

		int utility = i < middle ? i - middle + 1 : middle - i;
		double weight = exp(eps_em * utility);
		double shift = i > 0 ? mass[i - 1] : 0;
		mass[i] = shift + weight * gap[i];
	}

	// Top K Selection
	vector<size_t> output;
	uint64_t r = RandomDraw(mass[length - 1]);
	for(size_t i = 0; i < k; i ++)
	{
		int j = -1;
		for(size_t i = 0; i < shr_dataset.size(); i ++)
		{
			ABYParty * party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg, 4000000);
			vector<Sharing*> & sharings = party->GetSharings();
			BooleanCircuit * bcirc = (BooleanCircuit*)sharings[S_BOOL]->GetCircuitBuildRoutine();

			share *di_srv, *di_cli, *shr_e;
			if(role == SERVER) {
				di_srv = bcirc->PutINGate((uint32_t)shr_dataset[i], bitlen, role);
				di_cli = bcirc->PutDummyINGate(bitlen);
			}
			else {
				di_srv = bcirc->PutDummyINGate(bitlen);
				di_cli = bcirc->PutINGate((uint32_t)shr_dataset[i], bitlen, role);
			}
			shr_e = bcirc->PutSUBGate(di_srv, di_cli);

			share *gapi_srv, *gapi_cli, *shr_gap;
			if(role == SERVER) {
				gapi_srv = bcirc->PutINGate((uint32_t)gap[i], bitlen, role);
				gapi_cli = bcirc->PutDummyINGate(bitlen);
			}
			else {
				gapi_srv = bcirc->PutDummyINGate(bitlen);
				gapi_cli = bcirc->PutINGate((uint32_t)gap[i], bitlen, role);
			}
			shr_gap = bcirc->PutSUBGate(gapi_srv, gapi_cli);

			share *massi_srv, *massi_cli, *shr_mass;
			if(role == SERVER) {
				massi_srv = bcirc->PutINGate((uint64_t)mass[i], 64, role);
				massi_cli = bcirc->PutDummyINGate(64);
			}
			else {
				massi_srv = bcirc->PutDummyINGate(64);
				massi_cli = bcirc->PutINGate((uint64_t)mass[i], 64, role);
			}
			shr_mass = bcirc->PutSUBGate(massi_srv, massi_cli);

			share * shr_r;
			if(role == SERVER)
				shr_r = bcirc->PutINGate(r, 64, role);
			else
				shr_r = bcirc->PutDummyINGate(64);

			share * shr_cmp = bcirc->PutGTGate(shr_r, shr_mass);

			share *out_cmp = bcirc->PutOUTGate(shr_cmp, ALL);

			party->ExecCircuit();

			uint32_t cmp = out_cmp->get_clear_value<uint32_t>();

			delete party;
			delete di_srv, di_cli, shr_e;
			delete gapi_srv, gapi_cli, shr_gap;
			delete massi_srv, massi_cli, shr_mass;
			delete shr_r, shr_cmp, out_cmp;

			if(cmp == true) {
				output.push_back(nr[i]);
				erase(nr, i);
				erase(shr_dataset, i);
				erase(gap, i);
				erase(mass, i);
			}
		}
	}

	return output;
}

void Party::RandomSelection()
{
	unique_ptr<CSocket> tsocket;

	if(role == SERVER) {
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed" << endl;
			exit(1);
		}

		size_t len(shr_dataset.size());
		for(size_t i = 0; i < len; i ++) {
			uint32_t rnd1 = rand();
			uint32_t rnd2;
			tsocket->Send((void *)&rnd1, sizeof(rnd1));
			tsocket->Receive((void *)&rnd2, sizeof(rnd2));

			size_t sel = (rnd1 + rnd2) % shr_dataset.size();
			shr_dataset.erase(shr_dataset.begin() + sel);
		}
	}
	else {
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Connect Failed" << endl;
			exit(1);
		}

		size_t len(shr_dataset.size());
		for(size_t i = 0; i < len; i ++) {
			uint32_t rnd1;
			uint32_t rnd2 = rand();
			tsocket->Receive((void *)&rnd1, sizeof(rnd1));
			tsocket->Send((void *)&rnd2, sizeof(rnd2));

			size_t sel = (rnd1 + rnd2) % shr_dataset.size();
			shr_dataset.erase(shr_dataset.begin() + sel);
		}
	}

	tsocket->Close();
}


void Party::Selection()
{
	double eps1, eps2;
	double c;
	double delta_q;
	double T; // threshold

	eps1 = p1 * eps;
	eps2 = eps - eps1;
	c = 2 * eps1 / eps2 ;
	delta_q = get_delta_q(delta, kbar, c);
	T = get_T(delta_q, eps1, eps2);

	clog << "k   \t" << k << endl;
	clog << "kbar\t" << kbar << endl;
	clog << "eps \t" << eps << endl;
	clog << "p1  \t" << p1 << endl;
	clog << "epsem\t" << eps_em << endl;
	clog << "delta\t" << delta << endl;
	clog << "delta_q\t" << delta_q << endl;
	clog << "thresh\t" << T << endl;
	clog << "compare times\t" << this->comparetimes << endl;

	double qi_n;
	for(int i = kbar - 1; i >= 0; i --)
	{
		qi_n = get_qi(i, eps2); // noisy qi

		if(qi_n > T)
		{
			shr_dataset.erase(shr_dataset.begin()+i+1, shr_dataset.end());
			/* RandomSelection(); */
			return;
		}
	}

	if(role == SERVER) {
		// clog << "nr_interset: " << nr_interset << endl;
		clog << "delta: " << delta << endl;
		clog << "qi_n: " << qi_n << endl;
		clog << "T: " << T << endl;
	}

	clog << "There is no output!" << endl;
	// assert(0);
	return;
}


void Party::MakeTopKPublic()
{
	unique_ptr<CSocket> tsocket;
	/* string md5str; */
	string itemid;
	size_t idlength, itemca, itemcb;

	if(this->role == SERVER) {
		tsocket = Listen(address, port);
		if(!tsocket) {
			cerr << "Listen Failed" << endl;
			exit(1);
		}

		for(size_t i = 0; i < this->shr_dataset.size(); i++) {
			itemid = this->shr_dataset[i].first;
			itemca = this->shr_dataset[i].second; // Item Count a
			if(itemid.empty() == true) {
				// Receive ID
				tsocket->Receive((void *)&idlength, sizeof(idlength));
				itemid.resize(idlength);
				tsocket->Receive((void *)&itemid[0], idlength);
			}
			else {
				// Send ID
				idlength = itemid.size();
				tsocket->Send((void *)&idlength, sizeof(idlength));
				tsocket->Send((void *)itemid.c_str(), itemid.size());
			}
			// Send Count
			tsocket->Send((void *)&itemca, sizeof(itemca));
			// Receive Count
			tsocket->Receive((void *)&itemcb, sizeof(itemcb));

			this->topkitem.push_back(itemid);
			this->topk_item_freq.push_back(make_pair(itemid, itemca - itemcb));
		}
	}
	else {
		tsocket = Connect(address, port);
		if(!tsocket) {
			cerr << "Listen Failed" << endl;
			exit(1);
		}

		for(size_t i = 0; i < this->shr_dataset.size(); i++) {
			itemid = shr_dataset[i].first;
			itemcb = this->shr_dataset[i].second;
			if(itemid.empty() == false) {
				// Send ID
				idlength = itemid.size();
				tsocket->Send((void *)&idlength, sizeof(idlength));
				tsocket->Send((void *)itemid.c_str(), itemid.size());
			}
			else {
				// Receive ID
				tsocket->Receive((void *)&idlength, sizeof(idlength));
				itemid.resize(idlength);
				tsocket->Receive((void *)&itemid[0], idlength);
			}
			// Receive Count
			tsocket->Receive((void *)&itemca, sizeof(itemca));
			// Send Count
			tsocket->Send((void *)&itemcb, sizeof(itemcb));

			this->topkitem.push_back(itemid);
			this->topk_item_freq.push_back(make_pair(itemid, itemca - itemcb));
		}
	}

	tsocket->Close();
}


void Party::ConstructCandidateItemSet()
{
	size_t boundsize = static_cast<size_t>(log2(this->topk_item_freq.size()));

	size_t maxfreq = 0;
	for(size_t i = 0; i < this->topk_item_freq.size(); i++)
		if(this->topk_item_freq[i].second > maxfreq)
			maxfreq = this->topk_item_freq[i].second;

	// sort by the dictionary order
	sort(
		this->topk_item_freq.begin(),
		this->topk_item_freq.end(),
		[](const KVpair &a, const KVpair &b) {
			return a.first < b.first;
		}
	);

	size_t topklistlength(this->topk_item_freq.size());
	size_t queuesize = pow(2, ceil(log2(2 * this->k)));
	/* size_t queuesize = this->k^2; */
	priority_queue<Set_Freq_pair, vector<Set_Freq_pair>, Comparator> IS_invert;

	for(size_t setsize = 1; setsize <= boundsize; setsize ++)
	{
		bool levelflag(false);

		string bitmask(setsize, 1);
		bitmask.resize(topklistlength);
		do {
			vector<string> comb;
			double freq(-1.0);
			for(size_t j = 0; j < topklistlength; j ++)
			{
				if(bitmask[j])
				{
					comb.push_back(this->topk_item_freq[j].first);
					freq *= (0.9 * this->topk_item_freq[j].second) / maxfreq;
				}
			}
			if(IS_invert.size() <= queuesize) {
				IS_invert.push(make_pair(comb, freq));
				levelflag = true;
			}
			else {
				if(IS_invert.top().second >= freq) {
					if(IS_invert.top().second != freq) IS_invert.pop();
					IS_invert.push(make_pair(comb, freq));
					levelflag = true;
				}
			}
		} while (prev_permutation(bitmask.begin(), bitmask.end()));

		if(!levelflag) break;
	}

	// select the most frequent top 2*k
	while(!IS_invert.empty()) {
		auto &top(IS_invert.top());
		this->IS.push_back(make_pair(top.first, top.second * -1));
		IS_invert.pop();
	}

	reverse(this->IS.begin(), this->IS.end());
	if(this->IS.size() > 2 * this->k)
		this->IS.erase(this->IS.begin() + 2 * this->k, this->IS.end());
}
