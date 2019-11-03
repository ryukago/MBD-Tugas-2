
// Lock manager implementing deterministic two-phase locking as described in
// 'The Case for Determinism in Database Systems'.

#include "txn/lock_manager.h"

using std::cout;
using std::endl;

LockManagerA::LockManagerA(deque<Txn*>* ready_txns) {
  ready_txns_ = ready_txns;
}

bool LockManagerA::WriteLock(Txn* txn, const Key& key) {
    // Search if key has been added to table
    if (lock_table_.find(key) == lock_table_.end()) { // not found
        deque<LockRequest> *d = new deque<LockRequest>;

        lock_table_.insert(std::pair<int, deque<LockRequest>*>(key, d));
    }

    LockRequest* req = new LockRequest(EXCLUSIVE, txn);

    // If deque empty, push lock req
    lock_table_[key]->push_back(*req);

    // if key already in the table, return false
    if (lock_table_.find(key) != lock_table_.end()) {
        // Track all txns still waiting on acquiring at least one lock
        txn_waits_.insert(std::pair<Txn*, int>(txn, key));
        return false;
    }

    lock_table_[key]->push_back(*req);
    return true;
}

bool LockManagerA::ReadLock(Txn* txn, const Key& key) {
  // Since Part 1A implements ONLY exclusive locks, calls to ReadLock can
  // simply use the same logic as 'WriteLock'.
  return WriteLock(txn, key);
}

void LockManagerA::Release(Txn* txn, const Key& key) {
  // Pop the first element of lock_table and push the next one to ready_txns
  lock_table_[key]->pop_front();
  ready_txns_->push_back(lock_table_[key]->front().txn_);
}

LockMode LockManagerA::Status(const Key& key, vector<Txn*>* owners) {
    // Clear all elements in owners
    owners->clear();
    owners->push_back(ready_txns_->back());

    if (owners->size() > 0) return EXCLUSIVE;
    return UNLOCKED;
}

LockManagerB::LockManagerB(deque<Txn*>* ready_txns) {
  ready_txns_ = ready_txns;
}

bool LockManagerB::WriteLock(Txn* txn, const Key& key) {
  //
  // Implement this method!
  return true;
}

bool LockManagerB::ReadLock(Txn* txn, const Key& key) {
  //
  // Implement this method!
  return true;
}

void LockManagerB::Release(Txn* txn, const Key& key) {
  //
  // Implement this method!
}

LockMode LockManagerB::Status(const Key& key, vector<Txn*>* owners) {
  //
  // Implement this method!
  return UNLOCKED;
}

