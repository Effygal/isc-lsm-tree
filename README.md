Baseline uses [sLSM](https://github.com/aronszanto/sLSM-Tree.git)
`lsm2` dispatches multiple threads to emulate in-storage search.
latency caused by thread launching should be deducted from total.

Report under `lsm/simulation_output.txt`