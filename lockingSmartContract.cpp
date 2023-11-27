#include <mutex>
#include <iostream>
#include "lftt.h"
#include <jemalloc/jemalloc.h>

class LockingSmartContract
{
    public:
    int *users;
    int *lockCounts;
    std::mutex *locks;
    int numKeys;

    LockingSmartContract(int _numKeys)
    {
        users = new int[_numKeys];
        locks = new std::mutex[_numKeys];
        lockCounts = new int[_numKeys];
        numKeys = _numKeys;
    }

    void undoGiveToken(int userId)
    {
        users[userId]--;
    }

    bool giveToken(int userId, Descriptor *desc)
    {
        if (locks[userId].try_lock())
        {
            users[userId]++;
            desc->lockProfile[userId] = lockCounts[userId]++;
            return true;
        }  
        else
        {
            return false;
        }
    }

    bool giveTokens(Descriptor *desc)
    {
        int i;
        for (i = 0; i < desc->t->values.size(); i++)
        {
            if (!giveToken(desc->t->values[i], desc))
            {
                break;
            }
        }

        // Perform rollback if the loop does not run to completion
        if (i != desc->t->values.size())
        {
            for (int j = i - 1; j >= 0; j--)
            {
                undoGiveToken(desc->t->values[j]);
                locks[desc->t->values[j]].unlock();
            }
            return false;
        }
        else
        {
            for (int j = i - 1; j >= 0; j--)
            {
                locks[desc->t->values[j]].unlock();
            }
            return true;
        }
    }

    int getValues()
    {
        int sum = 0;
        for (int i = 0; i < numKeys; i++)
        {
            //std::cout << "User " << i << ": " << users[i] << " tokens\n";
            sum += users[i];
        }
        std::cout << "Total: " << sum << " tokens\n";
        return sum;
    }
};