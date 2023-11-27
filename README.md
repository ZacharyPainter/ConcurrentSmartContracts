# Conccurent Smart Contracts

This repo contain the source code for "Lock-Free Concurrent Smart Contracts"

# Dependencies

Boost, Jemalloc

# To Compile

Make sure required dependencies are installed

Issue:
    $make

# To Run

Proper format: ./a.out <"Locking", "LockFree"> <#Transaction_Size> <#TransactionsPerBlock> <#NumKeys> <#NumThread> <includeGraphing>

"Locking": Runs the locking version based on transaction boosting
"LockFree": Runs the lockfree version based on LFTT

"Transaction_Size": Number of operations per transaction

"TransactionsPerBlock": Number of transactions to execute

"NumKeys": The number of unique users to define per smart contract, larger values reduces thread contention

"NumThread": Number of threads to spawn

"includeGraphing": If "true", will also include the time spent constructing the conflict graph in execution time
