# PrimeCounter
Assignment 1 COP 4520
Created by: Mari Peele
Language: C++
Credit for original algorithm: The Sieve of Adkin was created by A. O. Atkin and Daniel J. Bernstein in 2023


Compile Instructions
g++ -O3 primecounter.cpp -o primecounter.exe
./primecounter

For this assignment I experimentally compared execution times for two different prime counting methods as both sequential and parallelized over eight threads. The first method was a primality test using a ticket system using an atomic counter and a mutex lock for writing to the variables tracking totals. The average execution time running eight threads concurrently was 13 seconds. The second method used implemented the Sieve of Atkin. To allow parallelization, a ticket system was used with two atomic counters along with two sections controlled by tickets. The average execution times for method two using eight threads ranged from 640ms to 780m.

The ticket system provides safety by ensuring each thread has its own unique number to test. It is implemented using std::atomic and while loops. An array of type unsigned is used in place of a bool array for the sieve to prevent issues with flipping bits. The Sieve of Atkin improves on the Sieve of Eratosthenes by preparing the list first and then marking off squares of primes rather than multiples of primes. 



