# Task description
Write C++ program which creates 3 threads
1. Data generator
        Generates random size vectors of ints for random time (max 10s) and passes to processer.
        Use some little sleep between each yield.
2. Data processer
        Calculates average for each of vectors. Passes results to aggregator.
3. Data aggregator
        Calculates average of averages. Prints out result after finishing work.

Program should gracefully quit at the end, using notification, that there will be no more work (time limit reached).

For data passing between threads, create "universal (templated) queue" which you can use in all (current two) cases.
Threads are not allowed to block each other executing its task, except in a moment when pushing / popping data in exchange queue.
