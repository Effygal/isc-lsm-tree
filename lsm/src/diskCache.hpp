//
//  diskCache.hpp
//  lsm-tree
//
//    sLSM: Skiplist-Based LSM Tree
//    Copyright © 2017 Aron Szanto. All rights reserved.
//
#pragma once

#ifndef diskCache_h
#define diskCache_h
#include <vector>
#include <cstdint>
#include <string>
#include <cstring>
#include "run.hpp"
#include "diskRun.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cassert>
#include <algorithm>

using namespace std;


template <class K, class V>
class DiskCache {
    
public: // TODO make some of these private
    typedef KVPair<K,V> KVPair_t;
    typedef pair<KVPair<K, V>, int> KVIntPair_t;
    KVPair_t KVPAIRMAX;
    KVIntPair_t KVINTPAIRMAX;
    V V_TOMBSTONE = (V) TOMBSTONE;

    struct OrderedDict{
        int size ;
        vector<KVIntPair_t> arr;
        KVIntPair_t max;
        //replace everything with LRU cache:
        //consider ordered dict
        StaticHeap(unsigned sz, KVIntPair_t mx) {
            size = 0;
            arr = vector<KVIntPair_t>(sz, mx);
            max = mx;
        }
        
        void insert(KVIntPair_t blob) {
            
        }
        void heapify(int i) {
            
        }
        
        KVIntPair_t pop() {
            
        }
    };

    
    int _level;
    unsigned _pageSize; // number of elements per fence pointer
    unsigned long _runSize; // number of elts in a run
    unsigned _numRuns; // number of runs in a level
    unsigned _activeRun; // index of active run
    unsigned _mergeSize; // # of runs to merge downwards
    double _bf_fp; // bloom filter false positive
    vector<DiskRun<K,V> *> runs;

    
    
    DiskLevel<K,V>(unsigned int pageSize, int level, unsigned long runSize, unsigned numRuns, unsigned mergeSize, double bf_fp):_numRuns(numRuns), _runSize(runSize),_level(level), _pageSize(pageSize), _mergeSize(mergeSize), _activeRun(0), _bf_fp(bf_fp){
        KVPAIRMAX = (KVPair_t) {INT_MAX, 0};
        KVINTPAIRMAX = KVIntPair_t(KVPAIRMAX, -1);
        
        for (int i = 0; i < _numRuns; i++){
            DiskRun<K,V> * run = new DiskRun<K, V>(_runSize, pageSize, level, i, _bf_fp);
            runs.push_back(run);
        }

        

        
        
    }
    
    ~DiskCache<K,V>(){
        for (int i = 0; i< runs.size(); ++i){
            delete runs[i];
        }
    }
    
    void addRuns(vector<DiskRun<K, V> *> &runList, const unsigned long runLen, bool lastLevel) {
        
        //TODO: change with LRU DS
        StaticHeap h = StaticHeap((int) runList.size(), KVINTPAIRMAX);
        vector<int> heads(runList.size(), 0);
        for (int i = 0; i < runList.size(); i++){
            KVPair_t kvp = runList[i]->map[0];
            h.push(KVIntPair_t(kvp, i));
        }
        
        int j = -1;
        K lastKey = INT_MAX;
        unsigned lastk = INT_MIN;
        while (h.size != 0){
            auto val_run_pair = h.pop();
            assert(val_run_pair != KVINTPAIRMAX); // TODO delete asserts
            if (lastKey == val_run_pair.first.key){
                if( lastk < val_run_pair.second){
                    runs[_activeRun]->map[j] = val_run_pair.first;
                }
            }
            else {
                ++j;
                if ( j != -1 && lastLevel && runs[_activeRun]->map[j].value == V_TOMBSTONE){
                    --j;
                }
                runs[_activeRun]->map[j] = val_run_pair.first;
            }
            
            lastKey = val_run_pair.first.key;
            lastk = val_run_pair.second;
            
            unsigned k = val_run_pair.second;
            if (++heads[k] < runList[k]->getCapacity()){
                KVPair_t kvp = runList[k]->map[heads[k]];
                h.push(KVIntPair_t(kvp, k));
            }
                
        }
        
        if (lastLevel && runs[_activeRun]->map[j].value == V_TOMBSTONE){
            --j;
        }
        runs[_activeRun]->setCapacity(j + 1);
        runs[_activeRun]->constructIndex();
        if(j + 1 > 0){
            ++_activeRun;
        }
        
    }
    
    
    bool cacheFull(){
        return (_activeRun == _numRuns);
    }
    bool cacheEmpty(){
        return (_activeRun == 0);
    }
    
    V lookup (const K &key, bool &found) {
        //TODO: change with cache DS
        int maxRunToSearch = levelFull() ? _numRuns - 1 : _activeRun - 1;
        for (int i = maxRunToSearch; i >= 0; --i){
            if (runs[i]->maxKey == INT_MIN || key < runs[i]->minKey || key > runs[i]->maxKey || !runs[i]->bf.mayContain(&key, sizeof(K))){
                continue;
            }
            V lookupRes = runs[i]->lookup(key, found);
            if (found) {
                return lookupRes;
            }
            
        }
        
        return (V) NULL;
        
    }
    unsigned long num_elements(){
        unsigned long total = 0;
        for (int i = 0; i < _activeRun; ++i)
            total += runs[i]->getCapacity();
        return total;
    }
};
#endif /* diskLevel_h */
