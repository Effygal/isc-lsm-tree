#include <map>
#include <unordered_map>
#include <assert.h>
#include <vector>

/*
  +-+-+-+-+-+-+-+-+-+-+-+-+-+
  | | |x| |x| |x| | | | | | |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+
   0   ^         ^
       |         head ->
       tail ->
  head points to first unoccupied entry
  tail may point to empty entry
 */

class LRU {
    int M = 0;
    int C = 0;
    std::vector<int> map;
    int tail = 0, head = 0;
    std::vector<int> cache;

    std::vector<int> ref;	// time of most recent ref
    std::vector<int> enter;	// time entered cache

    int n_evict = 0;
    double s_evict_ref = 0;
    double s2_evict_ref = 0;
    
    int len = 0;
    
    int n_cachefill = 0;
    int n_access = 0;
    int n_miss = 0;

public:
    LRU(int _C) {
	C = _C;
	map.resize(100000, -1);
	enter.resize(100000, -1);
	ref.resize(100000, -1);
	cache.resize(2*_C, -1);
    }
    ~LRU() {}

    // invalid entries in cache, map are flagged with -1
    // need specific flag if we want to use uint...
    
    void verify(void) {
	for (int i = tail; i < head; i++)
	    assert(cache[i] == -1 || map[cache[i]] == i);
	for (int i = 0; i < map.size(); i++) {
	    assert(map[i] == -1 || cache[map[i]] == i);
	    assert(map[i] < head);
	}
    }
		
    int contents(int *out) {
	int i, n;
	for (i = head-1, n = 0; i >= 0; i--)
	    if (cache[i] != -1)
		out[n++] = cache[i];
	return n;
    }
    
    void pull(int addr) {
	assert(map[addr] != -1);
	cache[map[addr]] = -1;
	map[addr] = -1;
	len--;
    }
    
    int pop(void) {
	while (cache[tail] == -1)
	    tail++;
	int addr = cache[tail];
	pull(addr);
	return addr;
    }

    void push(int addr) {
	if (head >= 2*C) {
	    int i, j;
	    for (i=0, j=0; i < head; i++)
		if (cache[i] != -1) {
		    map[cache[i]] = j;
		    cache[j++] = cache[i];
		}
	    tail = 0;
	    head = j;
	}
	map[addr] = head;
	cache[head++] = addr;
	len++;
    }
    
    void access(int addr) {
	n_access++;
	if (len < C)
	    n_cachefill = n_access;
	if (addr >= map.size()) 
	    map.resize(addr * 3 / 2, -1);
	if (map[addr] == -1) 
	    n_miss++;
	else
	    pull(addr);
	push(addr);
	if (len > C)
	    pop();
    }

    void access_verbose(int addr, int *miss, int *evictee,
			int *last_ref, int *entered) {
	n_access++;
	if (len < C)
	    n_cachefill = n_access;
	if (addr >= map.size()) {
	    int n = addr * 3 / 2;
	    map.resize(n, -1);
	    ref.resize(n, 0);
	    enter.resize(n, 0);
	}
	if (map[addr] == -1) {
	    n_miss++;
	    enter[addr] = n_access;
	    ref[addr] = n_access;
	} else {
	    ref[addr] = n_access;
	    pull(addr);
	}
	push(addr);
	if (len > C) {
	    int e = pop();
	    *miss = 1;
	    *evictee = e;
	    *last_ref = n_access - ref[e];
	    *entered = n_access - enter[e];

	    n_evict++;
	    s_evict_ref += *last_ref;
	    s2_evict_ref += 1.0 * (*last_ref) * (*last_ref);
	}
    }

    void queue_stats(int *n, double *s, double *s2) {
	*n = n_evict;
	*s = s_evict_ref;
	*s2 = s2_evict_ref;
    }
    
    void multi_access(int n, int *addrs) {
	for (int i = 0; i < n; i++) {
	    access(addrs[i]);
	}
    }
	    
    void multi_access_age(int n, int *addrs, int *misses,
			  int *evicted, int *age1, int *age2) {
	for (int i = 0; i < n; i++)
	    access_verbose(addrs[i], &misses[i], &evicted[i], &age1[i], &age2[i]);
    }

    double hit_rate(void) {
	double miss_rate = (n_miss - C) * 1.0 / (n_access - n_cachefill);
	return 1 - miss_rate;
    }

    void data(int &_access, int &_miss, int &_cachefill) {
	_access = n_access;
	_miss = n_miss;
	_cachefill = n_cachefill;
    }
};

extern "C" void *lru_create(int C)
{
    LRU *l = new LRU(C);
    return (void*)l;
}

extern "C" void lru_delete(void *_l)
{
    LRU *l = (LRU*) _l;
    delete l;
}

extern "C" void lru_run(void *_l, int n, int *a)
{
    LRU *l = (LRU*) _l;
    l->multi_access(n, a);
}

extern "C" int lru_contents(void *_l, int *out)
{
    LRU *l = (LRU*) _l;
    return l->contents(out);
}

extern "C" void lru_run_age(void *_l, int n, int *a, int *b, int *c, int *d, int *e)
{
    LRU *l = (LRU*) _l;
    l->multi_access_age(n, a, b, c, d, e);
}

extern "C" double lru_hitrate(void *_l)
{
    LRU *l = (LRU*) _l;
    return l->hit_rate();
}

extern "C" void lru_queue_stats(void *_l, int *n, double *s, double *s2)
{
    LRU *l = (LRU*) _l;
    l->queue_stats(n, s, s2);
}

extern "C" void lru_data(void *_l, int *_access, int *_miss, int *_cachefill)
{
    LRU *l = (LRU*) _l;
    l->data(*_access, *_miss, *_cachefill);
}
