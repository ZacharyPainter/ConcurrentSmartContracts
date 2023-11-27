#include <thread>
#include <boost/random.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include "lftt.h"
#include "block.h"
#include "lockFreeSmartContract.cpp"
#include "lockingSmartContract.cpp"


//Default Values
int transactionSize = 3;
int transactionsPerBlock = 10;
int numKeys = 1000;
int numThreads = 4;
bool includeGraphing = false;
std::string mode;

LockFreeSmartContract *lockFreeSmartContract;
LockingSmartContract *lockingSmartContract;
Block *b;

Descriptor **descriptors;

//Array of thread data
ThreadData **t_data;


void generateGraphLockFree()
{
    for (int i = 0; i < lockFreeSmartContract->numKeys; i++)
    {
        NodeDescriptor *curr = lockFreeSmartContract->users[i].load();
        while (curr != nullptr)
        {
            NodeDescriptor *prev = curr->prev;
            if (prev != nullptr)
            {
                curr->desc->preds.push_back(prev->desc);
            }
            curr = prev;
        }
    }
}

void generateGraph()
{
    // Iterate over all descriptors
    for (int i = 0; i < transactionsPerBlock; i++)
    {
        Descriptor *desc = descriptors[i];
        Transaction *t = desc->t;

        for (int j = i + 1; j < transactionsPerBlock; j++)
        {
            Descriptor *desc2 = descriptors[j];
            
            // Iterate over the map of lock counters observed during execution
            for (auto const& x : desc->lockProfile)
            {
                int key = x.first;
                int value = x.second;

                // If the other descriptor has a lock on the same key, add an edge
                if (desc2->lockProfile.find(key) != desc2->lockProfile.end())
                {
                    //std::cout << "Adding edge from " << i << " to " << j << "\n";
                    if (x.second > desc2->lockProfile[key])
                    {
                        //std::cout << j << " happens-before " << i << "\n";
                        desc->preds.push_back(desc2);
                    }
                    else
                    {
                        //std::cout << i << " happens-before " << j << "\n";
                        desc2->preds.push_back(desc);
                    }
                }
            }
        }
    }
}

void workLocking(int id)
{
    //std::cout << "Thread " << id << " entering\n";

    int thread_range = transactionsPerBlock / numThreads;
    for (int i = thread_range * id; i < thread_range * (id + 1); i++)
    {
        //std::cout << id << " executing: " << i << "\n";
        //std::cout << "Value1: " << b->transactions[i]->value1 << "\nValue2: " << b->transactions[i]->value2 << "\nValue3: " << b->transactions[i]->value3 << "\n\n";
        Descriptor *desc = descriptors[i];
        desc->t = b->transactions[i];
        lockingSmartContract->giveTokens(desc);
    }
}

void workLockFree(int id)
{
    //std::cout << "Thread " << id << " entering\n";

    int thread_range = transactionsPerBlock / numThreads;
    for (int i = thread_range * id; i < thread_range * (id + 1); i++)
    {
        //std::cout << id << " executing: " << i << "\n";
        //std::cout << "Value1: " << b->transactions[i]->value1 << "\nValue2: " << b->transactions[i]->value2 << "\nValue3: " << b->transactions[i]->value3 << "\n\n";
        Descriptor *desc = descriptors[i];
        desc->t = b->transactions[i];
        desc->status = ACTIVE;
        lockFreeSmartContract->execute(desc, t_data[id]);
    }
}

int main(int argc, const char *argv[])
{
    if (argc < 7)
    {
        printf("Proper format: %s <\"Locking\", \"LockFree\"> <#Transaction_Size> <#TransactionsPerBlock> <#NumKeys> <#NumThread> <includeGraphing\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    mode = argv[1];
    transactionSize = atoi(argv[2]);
    transactionsPerBlock = atoi(argv[3]);
    numKeys = atoi(argv[4]);
    numThreads = atoi(argv[5]);
    includeGraphing = strcmp(argv[6], "true") ? false : true;

    descriptors = new Descriptor *[transactionsPerBlock];
    t_data = new ThreadData*[numThreads];
    
    //Fill the array with descriptors
    for (int i = 0; i < transactionsPerBlock; i++)
    {
        Descriptor *d = new Descriptor();
        descriptors[i] = d;
    }

    for (int i = 0; i < numThreads; i++)
    {
        t_data[i] = new ThreadData();
    }

    //Generate a block
    b = new Block(transactionsPerBlock, numKeys, transactionSize);
    //b->printBlock();

    //Init smart contract
    lockFreeSmartContract = new LockFreeSmartContract(numKeys);
    lockingSmartContract = new LockingSmartContract(numKeys);

    printf("Starting test...\n\n");

    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();

    //Start threads working
    for (int i = 0; i < numThreads; i++)
    {
        if (mode == "Locking")
            threads.push_back(std::thread(workLocking, i));
        else if (mode == "LockFree")
            threads.push_back(std::thread(workLockFree, i));
    }

    // Join main thread to each thread
    // This waits for all threads to complete before proceeding in main
    for (int i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }

    auto graph = std::chrono::high_resolution_clock::now();
    if (includeGraphing)
    {
        if (mode == "Locking")
            generateGraph();
        else if (mode == "LockFree")
            generateGraphLockFree();
    }
    auto graphend = std::chrono::high_resolution_clock::now();

    // Stop timing
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start).count();
    auto elapsedGraph = std::chrono::duration_cast<std::chrono::microseconds>(graphend - graph).count();

    std::cout << "Graph generation time: " << elapsedGraph << "ms\n";

    int sum;
    if (mode == "Locking")
        sum = lockingSmartContract->getValues();
    else if (mode == "LockFree")
        sum = lockFreeSmartContract->getValues();

    //Print elapsed time in milliseconds
    std::cout << "Elapsed time: " << elapsed << " microseconds\n";
    std::cout << "Throughput: " << (sum / ((double)elapsed / 1000)) << " transactions per millisecond\n";
    std::cout << "Percent transactions failed: " << 1 - ((float)sum / (transactionsPerBlock * transactionSize)) << "\n";

    if (sum % transactionSize != 0)
    {
        std::cout << "ERROR: sum % transactionSize != 0, got remainder: " << sum % transactionSize << "\n";
    }

    std::ofstream file;
    std::string filename;
    if (includeGraphing)
        filename = mode + "_" + std::to_string(transactionSize) + "_graph.dat";
    else
        filename = mode + "_" + std::to_string(transactionSize) + "_noGraph.dat";  
        
	file.open(filename, std::ios_base::app);

    if (includeGraphing)
        file << transactionsPerBlock << "\t" << sum / ((double)elapsed / 1000) << "\t" << 1 - ((float)sum / (transactionsPerBlock * transactionSize)) << "\n";
    else
        file << numThreads << "\t" << sum / ((double)elapsed / 1000) << "\t" << 1 - ((float)sum / (transactionsPerBlock * transactionSize)) << "\n";
    

    //printf("Ops/s %.0f\n", (g_commits*transaction_size)/elapsed);
    //printf("Total Commits %d, Total Aborts: %d \n", g_commits, g_aborts);
    //printf("Success Rate: %f%% \n", 100*((double)g_commits/(test_size*transaction_size)));
}