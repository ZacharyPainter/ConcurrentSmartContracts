#include <atomic>
#include <iostream>
#include <algorithm>
#include "lftt.h"
#include <jemalloc/jemalloc.h>
#include <vector>

class LockFreeSmartContract
{
    public:
    std::atomic<NodeDescriptor *> *users;
    int numKeys;

    LockFreeSmartContract(int _numKeys)
    {
        users = new std::atomic<NodeDescriptor *>[_numKeys];
        numKeys = _numKeys;
    }

    int getCommittedValue(NodeDescriptor *curr)
    {
        while (curr != nullptr && curr->desc->status == ABORTED)
        {
            curr = curr->prev;
        }

        if (curr)
            return curr->value;
        else
            return 0;
    }

    bool giveToken(int userId, Descriptor *desc, ThreadData *tData)
    {
        NodeDescriptor *newDesc = new NodeDescriptor();
        NodeDescriptor *conflictTx = nullptr;
        
        while (true)
        {
            newDesc->desc = desc;
            newDesc->prev = users[userId].load();
            int status = newDesc->prev == nullptr ? COMMITTED : newDesc->prev->desc->status.load();

            // If we are helping an operation that is already complete, break from the loop
            if (newDesc->prev != nullptr && newDesc->prev->desc == desc)
                return true;

            // If our transaction is no longer active, break from the loop
            if (desc->status != ACTIVE)
                return false;

            // If this state variable is not free, help it complete and then retry the loop
            if (newDesc->prev != nullptr && status == ACTIVE)
            {
                giveTokens(newDesc->prev->desc, tData);
                continue;
            }

            newDesc->value = getCommittedValue(newDesc->prev) + 1;
            
            if (users[userId].compare_exchange_weak(newDesc->prev, newDesc))
            {
                desc->currOp++;
                return true;
            }
        }
    }

    void giveTokens(Descriptor *desc, ThreadData *tData)
    {
        bool ret;
        int currOp = desc->currOp;

        // Check if helpStack contains desc
        if (std::find(tData->helpStack.begin(), tData->helpStack.end(), desc) != tData->helpStack.end())
        {
            int status = desc->status.load();
            if (status == ACTIVE)
                desc->status.compare_exchange_weak(status, ABORTED);
            return;
        }

        tData->helpStack.push_back(desc);
        for (int i = currOp; i < desc->t->values.size(); i++)
        {
            //std::cout << "Giving token to " << desc->t->values[i] << "\n";
            ret = giveToken(desc->t->values[i], desc, tData);
        }
        tData->helpStack.pop_back();

        int status = desc->status.load();
        if (status == ACTIVE)
            desc->status.compare_exchange_weak(status, COMMITTED);
    }

    void execute(Descriptor *desc, ThreadData *tData)
    {
        tData->helpStack.clear();
        desc->currOp = 0;
        giveTokens(desc, tData);
    }

    int getValues()
    {
        int sum = 0;
        for (int i = 0; i < numKeys; i++)
        {
            NodeDescriptor *curr = users[i];

            if (curr == nullptr)
            {
                //std::cout << "User " << i << ": 0 tokens\n"; 
            }
            else
            {
                //std::cout << "User " << i << ": " << curr->value << " tokens\n";
                sum += getCommittedValue(curr);
            }
        }
        std::cout << "Total: " << sum << " tokens\n";
        return sum;
    }
};