
// Lock manager implementing deterministic two-phase locking as described in
// 'The Case for Determinism in Database Systems'.
#include <deque>
#include "txn/lock_manager.h"

using std::deque;

LockManager::~LockManager() {
  for (unordered_map<Key, deque<LockRequest>*>::iterator it = lock_table_.begin(); it != lock_table_.end(); it++) {
    delete it->second;
  }
}

LockManagerA::LockManagerA(deque<Txn*>* ready_txns) {
  ready_txns_ = ready_txns;
}

bool LockManagerA::WriteLock(Txn* txn, const Key& key) {
  //
  // Implement this method!
  LockRequest lock(EXCLUSIVE, txn);
  deque<LockRequest> *locks = lock_table_[key];
  bool succeed;
  if (!locks) {
    locks = new deque<LockRequest>();
    lock_table_[key] = locks;
    succeed = true;
  }
  else {
    if (locks->empty()) {
      succeed = true;
    }
    else {
      txn_waits_[txn]++;
      succeed = false;
    }
  }
  locks-> push_back(lock);
  return succeed;
}

bool LockManagerA::ReadLock(Txn* txn, const Key& key) {
  // Since Part 1A implements ONLY exclusive locks, calls to ReadLock can
  // simply use the same logic as 'WriteLock'.
  return WriteLock(txn, key);
}

void LockManagerA::Release(Txn* txn, const Key& key) {
  //
  // Implement this method!
  deque<LockRequest> *locks = lock_table_[key];
  if (!locks) {
    locks = new deque<LockRequest>();
    lock_table_[key] = locks;
  }
  bool remove_owner = true;
  if (locks) {
    for (deque<LockRequest>::iterator it = locks->begin(); it < locks->end(); it++) {
      if (it->txn_ == txn) {
        locks->erase(it);
        break;
      }
      remove_owner = false;
    }

    if ((!locks->empty()) && (remove_owner)) {
      LockRequest next_lock = locks->front();

      if (--txn_waits_[next_lock.txn_] == 0) {
        ready_txns_->push_back(next_lock.txn_);
        txn_waits_.erase(next_lock.txn_);
      }
    }
  }
}

LockMode LockManagerA::Status(const Key& key, vector<Txn*>* owners) {
  //
  // Implement this method!
  deque<LockRequest> *locks = lock_table_[key];
  if (!locks) {
    locks = new deque<LockRequest>();
    lock_table_[key] = locks;
  }

  if (locks->empty()) {
   return UNLOCKED;
  }
  else {
    vector<Txn*> owners_new;
    owners_new.push_back(locks->front().txn_);
    *owners = owners_new;
    return EXCLUSIVE;
  }
}
