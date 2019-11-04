
// Lock manager implementing deterministic two-phase locking as described in
// 'The Case for Determinism in Database Systems'.

#include <algorithm>
#include <set>
#include "txn/lock_manager.h"

using std::cout;
using std::endl;
using std::find;

LockManagerA::LockManagerA(deque<Txn*>* ready_txns) {
  ready_txns_ = ready_txns;
}

bool LockManagerA::WriteLock(Txn* txn, const Key& key) {
    LockRequest* req = new LockRequest(EXCLUSIVE, txn);

    // Search if key has been added to table
    if (lock_table_.find(key) == lock_table_.end()) { // not found
        deque<LockRequest> *d = new deque<LockRequest>;

        lock_table_.insert(std::pair<int, deque<LockRequest>*>(key, d));
    } else {
        if (lock_table_[key]->empty()) {
            lock_table_[key]->push_back(*req);
            return true;
        } else {
            txn_waits_[txn]++;
            lock_table_[key]->push_back(*req);
            return false;
        }
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
    if (find(ready_txns_->begin(), ready_txns_->end(), txn) != ready_txns_->end()) {
        for (auto it = lock_table_[key]->begin(); it < lock_table_[key]->end(); ) {
            if (it->txn_ == txn) {
                it = lock_table_[key]->erase(it);
            } else {
                it++;
            }
        }
        ready_txns_->push_back(lock_table_[key]->front().txn_);
    } else {
        for (auto it = lock_table_[key]->begin(); it < lock_table_[key]->end(); ) {
            if (it->txn_ == txn) {
                it = lock_table_[key]->erase(it);
            } else {
                it++;
            }
        }
    }
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
        LockRequest* req = new LockRequest(EXCLUSIVE, txn);

    // Search if key has been added to table
    if (lock_table_.find(key) == lock_table_.end()) { // not found
        deque<LockRequest> *d = new deque<LockRequest>;

        lock_table_.insert(std::pair<int, deque<LockRequest>*>(key, d));
    } else {
        if (lock_table_[key]->empty()) {
            lock_table_[key]->push_back(*req);
            return true;
        } else {
            txn_waits_[txn]++;
            lock_table_[key]->push_back(*req);
            return false;
        }
    }

    lock_table_[key]->push_back(*req);
    return true;
}

bool LockManagerB::ReadLock(Txn* txn, const Key& key) {
    LockRequest* req = new LockRequest(SHARED, txn);

    // Search if key has been added to table
    if (lock_table_.find(key) == lock_table_.end()) { // not found
        deque<LockRequest> *d = new deque<LockRequest>;

        lock_table_.insert(std::pair<int, deque<LockRequest>*>(key, d));
    } else {
        if (lock_table_[key]->empty()) {
            lock_table_[key]->push_back(*req);
            return true;
        } else {
            txn_waits_[txn]++;
            lock_table_[key]->push_back(*req);
            return false;
        }
    }

    lock_table_[key]->push_back(*req);
    return true;
}

void LockManagerB::Release(Txn* txn, const Key& key) {
    for (auto it = lock_table_[key]->begin(); it < lock_table_[key]->end(); ) {
        if (it->txn_ == txn) {
            it = lock_table_[key]->erase(it);
        } else {
            it++;
        }
    }

    std::set<Txn *> sets;
    for (unsigned int i = 0; i < ready_txns_->size(); i++) {
        sets.insert(ready_txns_->at(i));
    }

    bool shared = false;
    for (unsigned int i = 0; i < lock_table_[key]->size(); i++) {
        auto a = sets.find(lock_table_[key]->at(i).txn_);
        if (*a != lock_table_[key]->at(i).txn_) {
            if (lock_table_[key]->at(i).mode_ == EXCLUSIVE && !shared) {
                ready_txns_->push_back(lock_table_[key]->at(i).txn_);
                break;
            } else {
                ready_txns_->push_back(lock_table_[key]->at(i).txn_);
                shared = true;
            }
        }
    }
}

LockMode LockManagerB::Status(const Key& key, vector<Txn*>* owners) {
    LockMode lm = UNLOCKED;

    owners->clear();
    for (unsigned int i = 0; i < lock_table_[key]->size(); i++) {
        if (lock_table_[key]->at(i).mode_ == EXCLUSIVE) {
            if (lm != SHARED) {
                owners->push_back(lock_table_[key]->at(i).txn_);
                return EXCLUSIVE;
            } else {
                return lm;
            }
        } else {
            owners->push_back(lock_table_[key]->at(i).txn_);
            lm = SHARED;
        }
    }

    return lm;
}

