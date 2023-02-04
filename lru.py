from ctypes import *
import numpy as np
liblru = CDLL('./liblru.so')

liblru.lru_hitrate.restype = c_double
liblru.lru_create.restype = c_void_p

class lru:
    def __init__(self, C):
        self.l = liblru.lru_create(c_int(C))
        self.C = C
    def run(self, trace):
        if type(trace[0]) != np.int32:
            trace = np.array(trace, dtype=np.int32)
        liblru.lru_run(c_void_p(self.l), c_int(len(trace)),
                             trace.ctypes.data_as(c_void_p))

    def run_age(self, trace):
        if type(trace[0]) != np.int32:
            trace = np.array(trace, dtype=np.int32)
        misses = np.zeros(len(trace), dtype=np.int32)
        evicted = np.zeros(len(trace), dtype=np.int32)
        age1 = np.zeros(len(trace), dtype=np.int32)
        age2 = np.zeros(len(trace), dtype=np.int32)
        liblru.lru_run_age(c_void_p(self.l), c_int(len(trace)),
                               trace.ctypes.data_as(c_void_p),
                               misses.ctypes.data_as(c_void_p),
                               evicted.ctypes.data_as(c_void_p),
                               age1.ctypes.data_as(c_void_p),
                               age2.ctypes.data_as(c_void_p))
        return [age1,age2,misses]

    def run_parts(self, trace, n):
        a0,m0,vals = 0,0,[]
        for i in range(0, len(trace), n):
            t = np.array(trace[i:i+n],dtype=np.int32)
            self.run(t)
            a,m,c = self.data()
            vals.append(1 - (m-m0)/(a-a0))
            a0,m0 = a,m
        return np.array(vals)

    def contents(self):
        val = np.zeros(self.C,dtype=np.int32)
        n = liblru.lru_contents(c_void_p(self.l), val.ctypes.data_as(c_void_p))
        return val[:n]

    def hitrate(self):
        a,m,c = self.data()
        return 1 - (m - self.C) / (a - c)
    
    def data(self):
        n_access = c_int()
        n_miss = c_int()
        n_cachefill = c_int()
        liblru.lru_data(c_void_p(self.l), byref(n_access), byref(n_miss),
                              byref(n_cachefill))
        return [n_access.value, n_miss.value, n_cachefill.value]

    def queue_raw_stats(self):
        n,s,s2 = c_int(),c_double(),c_double()
        liblru.lru_queue_stats(c_void_p(self.l), byref(n), byref(s), byref(s2))
        return [n.value,s.value,s2.value]

    # returns (mean, std)
    def queue_stats(self):
        n,s,s2 = self.queue_raw_stats()
        return (s/n, np.sqrt((s2 - s*s/n)/(n-1)))

    def __del__(self):
        liblru.lru_delete(c_void_p(self.l))
