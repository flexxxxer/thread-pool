# Thread Pool | Fixed-size array of threads

This thread-pool implementation allows you to:
- perform tasks without the cost of creating and destroying threads
- allows you to perform tasks in different modes: asynchronous and deferred (parallel)
- performing functions with any number of parameters

Features:
- pooling on first initialization (singleton)
- **(in future)** auto resize when there are not enough threads to complete tasks
