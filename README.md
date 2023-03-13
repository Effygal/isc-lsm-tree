# Cache simulation tools
## Including FIFO, LRU and CLOCK
### usage:
```
import clock
my_clock = clock.clock(1000)
my_clock.run(trace)
my_clock.hitrate()
my_clock.contents() #returns the cache contents at the end of the simulation
```

### synthetic trace generator:
```
import misc
trace1 = hc_trace(r,f,M,n)
trace2 = gen_from_iad(func,M,n)
```

### LSM tree simulator:
```
The Log-Structured Merge-Tree (LSM Tree) is a data structure that is commonly used in databases to efficiently manage large amounts of data.

To simulate an LSM Tree, follow these steps:

Define the data structure: An LSM Tree is composed of multiple levels, each of which contains a sorted key-value store. The first level (called the memtable) is an in-memory store that accumulates new data. Once the memtable reaches a certain size, it is flushed to disk as an immutable SSTable (sorted string table), which is then merged with existing SSTables in lower levels.

Implement insertions: When a new key-value pair is inserted, it is first added to the in-memory memtable. Once the memtable reaches its threshold size, it is flushed to disk as an SSTable. If there are existing SSTables in the next level, a merge is performed between the new SSTable and the existing ones to create a new SSTable with sorted and unique key-value pairs. The merge is performed in a way that ensures that the resulting SSTable is still sorted and free of duplicates.

Implement lookups: To look up a value for a given key, you start by checking the in-memory memtable. If the key is not found there, you move on to the first level of SSTables, which are also sorted. If the key is still not found, you proceed to the next level of SSTables and continue until you either find the value or exhaust all levels.

Implement deletions: When a key is deleted, you can mark it as deleted in the in-memory memtable. If an SSTable is flushed to disk, the deleted key is added to a separate deletion SSTable to record that the key has been deleted. During merges, deleted keys are ignored and not included in the resulting SSTable.

Tune parameters: There are several parameters that can be tuned in an LSM Tree, such as the memtable size, the SSTable size, and the number of levels. You can experiment with different values for these parameters to optimize performance for your particular use case.

Overall, simulating an LSM Tree can be a complex task, but by following these steps, you can start to get a sense of how it works and how to implement it in code.
```
