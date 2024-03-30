# sled

- [ ] add log module
- [ ] add http module(cpp-httplib)
- [ ] add ORM module


## Benchmark
 Name (* = baseline)      |  ns/op  | Baseline |  Ops/second
--------------------------|--------:|---------:|-----------:
 Random bool *            |       9 |        - | 109123586.4
 Random int8_t            |       7 |    0.778 | 134375880.8
 Random int32_t           |       8 |    0.889 | 120300189.7
 Random uint32_t          |       7 |    0.778 | 131234452.1
 Random uint32_t range    |       8 |    0.889 | 123079276.7
 Gaussian(0, 1)           |      17 |    1.889 |  58742994.8
 Exponential(1)           |      12 |    1.333 |  81447219.4
 Random float             |       7 |    0.778 | 136571495.3
 Random double            |       7 |    0.778 | 131796121.5
 Base64Encode             |  106299 |11811.000 |      9407.4
 Base64Decode             |  104897 |11655.222 |      9533.1
 ThreadBlockingCallByDefaultSocketServer |    6624 |  736.000 |    150950.1
 ThreadBlockingCallByNullSocketServer |    5309 |  589.889 |    188358.8
 ThreadPoolBench          |    3096 |  344.000 |    322941.4
 SystemTimeNanos          |      24 |    2.667 |  40659163.6


