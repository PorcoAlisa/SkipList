#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <sys/syscall.h>
#include <unistd.h>
#include "SkipList.h"
using namespace std;

#define NUM_THREADS 1
#define TEST_COUNT 100000
SkipList<int, std::string> skipList(18);
 
void InsertElementTest()
{
    printf("create thread(%ld).\n",syscall(SYS_gettid));
    for (int i = 0; i < (TEST_COUNT / NUM_THREADS); i++) {
        skipList.InsertElement(rand() % TEST_COUNT, "a");
    }
}
 
void SearchElementTest()
{
    printf("create thread(%ld).\n",syscall(SYS_gettid));
    for (int i = 0; i < (TEST_COUNT / NUM_THREADS); i++) {
        skipList.SearchElement(rand() % TEST_COUNT);
    }
}

int main(int argc, char *argv[])
{
    {    
        vector<thread> threads;
        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(InsertElementTest);
        }
        for (thread &th : threads) {
            th.join();
        }
        auto finish = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finish - start;
        cout << "insert elapsed:" << elapsed.count() << endl;
    }
 
    {
        vector<thread> threads;
        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(SearchElementTest);
        }
        for (thread &th : threads) {
            th.join();
        }
        auto finish = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finish - start;
        cout << "search elapsed:" << elapsed.count() << endl;
    }
}