#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <map>

enum OpStatus
{
    ACTIVE = 0,
    COMMITTED,
    ABORTED,
};

enum ReturnCode
{
    OK = 0,
    SKIP,
    FAIL,
    RETRY
};

enum OpType
{
    READ = 0,
    WRITE
};

class Transaction
{
    public: 
    Transaction(){};

    std::string method;
    std::vector<int> values;
};

class alignas(32) Descriptor
{
    public: 
    // Status of the transaction: values in [0, size] means live txn, values -1 means aborted, value -2 means committed.
    std::atomic <int> status;
    std::atomic <int> currOp;
    Transaction * t;
    std::map<int, int> lockProfile;
    std::vector<Descriptor *> preds;

};

class NodeDescriptor
{
    public: 
    NodeDescriptor()
    {
        desc = nullptr;
    }

    Descriptor* desc;
    int value;
    NodeDescriptor *prev;
};

class alignas(32) ThreadData
{
    public: 
    NodeDescriptor **descriptors;
    int descCount = 0;
    std::vector<Descriptor *> helpStack;
};