# Cache simulation tools
## Including FIFO, LRU and CLOCK
### usage:
```
import clock
my_clock = clock.clock(1000)
my_clock.run(trace)
my_clock.hitrate()
```

### synthetic trace generator:
```
import misc
trace1 = hc_trace(r,f,M,n)
trace2 = gen_from_iad(func,M,n)
```