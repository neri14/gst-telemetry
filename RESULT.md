# Results

////before moving compositioning to GPU
[INF] layout: Total drawing time over 2969 frames: 90559 ms
[INF] layout: Average drawing time: 30.502 ms

real    2m37.812s
user    2m40.805s
sys     0m4.411s

////after moving compositioning to GPU
[INF] layout: Total drawing time over 2969 frames: 80011 ms
[INF] layout: Average drawing time: 26.949 ms

real    3m14.365s
user    3m17.138s
sys     0m4.211s

/// MUCH SLOWER!!!
/// man separate GstBuffers being compositioned in GPU add way too much overhead
/// original approach of single cairo surface drawn into single GstBuffer seems better

/// simpler version of BufferPoolManager to be ported to master as it did show initially roughly 10s improvement in above case
/// tracing required to check where gains can be made
