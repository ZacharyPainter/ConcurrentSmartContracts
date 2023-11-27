#pragma once
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "lftt.h"

class Block
{
    public: 
    Block(int _numTransactions, int _numKeys, int _transactionsPerBlock)
    {
        numTransactions = _numTransactions;
        transactions = new Transaction*[numTransactions];
        
        // Seed random generation
        std::srand(std::time(nullptr));

        // Generate n transactions randomly without replacement
        for (int i = 0; i < numTransactions; i++)
        {
            Transaction *t = new Transaction();
            t->values = generateValuesWithoutReplacement(_transactionsPerBlock, _numKeys);
            t->method = "giveToken";
            this->transactions[i] = t;
        }
    }
    
    Transaction **transactions;
    int numTransactions;

    void printBlock()
    {
        for (int i = 0; i < numTransactions; i++)
        {
            std::cout << "Method: " << transactions[i]->method << "\n";
            
            // Print all values
            for (int j = 0; j < transactions[i]->values.size(); j++)
            {
                std::cout << transactions[i]->values[j] << " ";
            }
            std::cout << "\n";
        }
    }

    // Generate n random values without replacement
    std::vector<int> generateValuesWithoutReplacement(int n, int numKeys)
    {
        std::vector<int> values;
        for (int i = 0; i < n; i++)
        {
            do
            {
                // Generate random number
                int rand = std::rand() % numKeys;

                // Check if it already exists
                if (std::find(values.begin(), values.end(), rand) == values.end())
                {
                    values.push_back(rand);
                    break;
                }
            } while (true);
        }
        return values;
    }

};