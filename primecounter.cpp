/**
 *  Created by: Mari Peele
 *  Course: COP 4520
 *	Semester: Spring 2023
 *  Project: Programming Assignment 1 (Prime Counter)
 */



#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>


 /**
  * PrimeCounter implements an Atkin Sieve to find primes within a given range.
  * The counter tabulates the number of primes found, the sum of the primes, and the top ten primes in ascending order.
  * It also has the capability to print the results found and an execution time to a file.
  */
class PrimeCounter {

	
	private:
		long long mRange;
		long long mSum;
		long long mTotal;
		std::atomic<long long> mCounter;
		std::atomic<long long> mCounter2;
		std::atomic<long long> mCounter3;
		std::atomic<unsigned> *mSieve;
		std::vector<long long> mTopTen;

	public:
		
		PrimeCounter(long long range) :
			mRange(range),
			mSieve(nullptr)
		{
			initialize();
		}

		~PrimeCounter()
		{
			delete[] mSieve;
		}
		
		void initialize()
		{
			
			mCounter = 0;

			mCounter2 = 5;

			mSum = 0;

			mTotal = 0;

			if (mRange >= 2)
			{
				mTopTen = std::vector<long long>(10, 0);

				mSieve = new std::atomic<unsigned>[mRange + 1];

				for (long long i = 0; i <= mRange; i++)
				{
					mSieve[i].store(0);
				}

				if (mRange > 2)
				{
					mSieve[2].store(true);
				}

				if (mRange > 3)
				{
					mSieve[3].store(true);
				}
			}
		}

		/**
		 * Prints execution time, number of primes found, sum of primes, and top ten primes in ascending order.
		 * 
		 * @param fileStream The file stream to print to
		 * @param milliseconds The execution time in milliseconds
		 */

		void printResults(std::ofstream &fileStream, std::chrono::milliseconds milliseconds)
		{
			if (milliseconds.count() < 1000)
			{
				fileStream << milliseconds.count() << "ms";
			}
			else
			{
				fileStream << (milliseconds.count()/1000) << "s" << ":" << (milliseconds.count() % 1000) << "ms";
			}


			fileStream << " " << mTotal << " " << mSum << std::endl;
			
			std::sort(mTopTen.begin(), mTopTen.end());

			for (auto &element : mTopTen)
			{
				if (element != 0)
				{
					fileStream << element << " ";
				}
			}
		}

		/**
		 * Uses an atomic ticket system to calculate primes using the Sieve of Atkin.
		 * Each thread is given a ticket which consists of a unique number to test in the sieve.
		 */
		void sieveOfAtkin()
		{
			long long ticket = mCounter.fetch_add(1);

			while(ticket * ticket <= mRange)
			{
				for (long long j = 1; j * j <= mRange; j++)
				{
					long long n = (4 * ticket * ticket) + (j * j);

					if (n <= mRange && (n % 12 == 1 || n % 12 == 5))
					{
						mSieve[n] ^= true;
					}

					n = (3 * ticket * ticket) + (j * j);
					if (n <= mRange && (n % 12 == 7))
					{
						mSieve[n] ^= true;
					}

					n = (3 * ticket * ticket) - (j * j);
					if (ticket > j && n <= mRange && (n % 12 == 11))
					{
						mSieve[n] ^= true;
					}
				}

				ticket = mCounter.fetch_add(1);
			}

			long long ticket2 = mCounter2.fetch_add(1);

			while (ticket2 * ticket2 <= mRange)
			{
				if (mSieve[ticket2])
				{
					for (long long q = ticket2 * ticket2; q <= mRange; q += ticket2 * ticket2)
					{
						mSieve[q].store(false);
					}
				}
				ticket2 = mCounter2.fetch_add(1);
			}
		}
	
		/**
		 * Uses the array created by the sieve to tablulate results.
		 */
		void tabulateResults()
		{
			for (long long i = 2; i <= mRange; i++)
			{
				if (mSieve[i])
				{
					mSum += i;
					mTotal++;
					updateTopTen(i);
				}
			}
		}

		/**
		 * Updates the top ten list by swapping out smaller numbers for larger numbers.
		 * Each round propigates smaller numbers out of the list.
		 */
		void updateTopTen(long long prime)
		{
			long long value = prime;

			for (int i = 0; i < 10; i++)
			{
				if (value >= mTopTen.at(i))
				{
					std::swap(value, mTopTen.at(i));
				}
			}
		}
};

/**
 * ThreadManager creates, tasks, and joins threads.
 */
class ThreadManager
{
	int mThreadCount;
	std::vector<std::thread> mThreads;

public:

	ThreadManager(int threadCount)
	{
		mThreadCount = threadCount;
	}

	~ThreadManager() = default;

	void createThreads()
	{
		mThreads = std::vector<std::thread>(8);
	}

	void joinThreads()
	{
		for (std::thread &t : mThreads)
		{
			if (t.joinable())
			{
				t.join();
			}
		}
	}

	void taskThreads(PrimeCounter *myCounter)
	{
		for (std::thread &t : mThreads)
		{
			t = std::thread(&PrimeCounter::sieveOfAtkin, myCounter);
		}
	}
};

/**
 * Timer starts a timer, stops the timer, and calculates time tracked.
 */
class Timer
{
	std::chrono::steady_clock::time_point mStartTime;
	std::chrono::steady_clock::time_point mEndTime;
	
	public:

		Timer()
		{
			// Do nothing
		}

		~Timer() = default;

		void startTimer()
		{
			mStartTime = std::chrono::steady_clock::now();
		}

		void stopTimer()
		{
			mEndTime = std::chrono::steady_clock::now();
		}

		std::chrono::milliseconds getExecutionTime()
		{
			auto timeElapsed = mEndTime - mStartTime;

			return std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed);
		}
};

int main() {

	long long range = 100000000;
	int threadCount = 8;

	Timer timer;
	PrimeCounter myCounter(range);
	ThreadManager manager(threadCount);

	timer.startTimer();

	manager.createThreads();
	
	manager.taskThreads(&myCounter);
	
	manager.joinThreads();

	myCounter.tabulateResults();

	timer.stopTimer();

	std::ofstream fileStream("primes.txt");

	myCounter.printResults(fileStream, timer.getExecutionTime());


	return 0;
}