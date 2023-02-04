# misc functions for simulating hot/cold traffic, resampling, etc.
# trace generator.
#


import random

def hc(r,f,M):
    if random.random() < r:
        return random.randint(0,int(f*M)-1)
    else:
        return random.randint(int(f*M),M-1)

def hc_trace(r,f,M,n):
    trc = np.zeros(n,dtype=np.int32)
    for i in range(n):
        if random.random() < r:
            trc[i] = random.randint(0,int(f*M)-1)
        else:
            trc[i] = random.randint(int(f*M),M-1)
    return trc

def t_hc(r,f,M):
    if random.random() < r:
        return random.expovariate(r/(f*M))
    else:
        return random.expovariate((1-r)/((1-f)*M))

import heapq
import numpy as np

def gen_from_iad(f,M,n):
    h = []
    for i in range(M):
        t = f()
        heapq.heappush(h, [t,i])        
    a = []
    for i in range(n):
        t0,addr = h[0]
        a.append(addr)
        t = f()
        heapq.heapreplace(h, [t0+t,addr])
    return np.array(a, dtype=np.int32)

def gen_from_iad2(f,M,n):
    h = []
    a0 = 0
    while len(h) < M:
        t = f()
        if t != -1:
            heapq.heappush(h, [t,a0])
            a0 += 1
    a = []
    for i in range(n):
        t = f()
        if t == -1:
            a.append(a0)
            a0 += 1
        else:
            t0,addr = h[0]
            a.append(addr)
            heapq.heapreplace(h, [t0+t,addr])
    return np.array(a, dtype=np.int32)

import fifo
def sim_fifo(C, trace, raw=False):
    f = fifo.fifo(C)
    f.run(trace)
    if raw:
        a,m,c = f.data()
        return 1 - m/a
    else:
        return f.hitrate()

import clock
def sim_clock(C, trace, raw=False):
    c = clock.clock(C)
    c.run(trace)
    if raw:
        a,m,cf,recycle,examined,sumabit = c.data() #variable name c used for two different thing?
        return 1 - m/a
    else:
        return c.hitrate()

import lru
def sim_lru(C, trace, raw=False):
    l = lru.lru(C)
    l.run(trace)
    if raw:
        a,m,c = l.data()
        return 1 - m/a
    else:
        return l.hitrate()

# "compact" the address space of a trace.
def squash(t):
    a = np.unique(t)
    n = np.zeros(np.max(t)+1,dtype=np.int32)
    x = np.arange(len(a),dtype=np.int32)
    n[a] = x
    return n[t]

import pickle
def from_pickle(f):
    fp = open(f,'rb')
    val = pickle.load(fp)
    fp.close()
    return val

def to_pickle(var, f):
    fp = open(f,'wb')
    pickle.dump(var, fp)
    fp.close()
