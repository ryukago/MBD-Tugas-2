// Author: Kun Ren (kun.ren@yale.edu)
// Modified by Daniel Abadi

#include "txn/mvcc_storage.h"

// Init the storage
void MVCCStorage::InitStorage() {
  for (int i = 0; i < 1000000;i++) {
    Write(i, 0, 0);
    Mutex* key_mutex = new Mutex();
    mutexs_[i] = key_mutex;
  }
}

// Free memory.
MVCCStorage::~MVCCStorage() {
  for (unordered_map<Key, deque<Version*>*>::iterator it = mvcc_data_.begin();
       it != mvcc_data_.end(); ++it) {
    delete it->second;
  }

  mvcc_data_.clear();

  for (unordered_map<Key, Mutex*>::iterator it = mutexs_.begin();
       it != mutexs_.end(); ++it) {
    delete it->second;
  }

  mutexs_.clear();
}

// Lock the key to protect its version_list. Remember to lock the key when you read/update the version_list
void MVCCStorage::Lock(Key key) {
  mutexs_[key]->Lock();
}

// Unlock the key.
void MVCCStorage::Unlock(Key key) {
  mutexs_[key]->Unlock();
}

// MVCC Read
bool MVCCStorage::Read(Key key, Value* result, int txn_unique_id) {
  // CPSC 438/538:
  //
  // Implement this method!

  // Hint: Iterate the version_lists and return the verion whose write timestamp
  // (version_id) is the largest write timestamp less than or equal to txn_unique_id.

  int max_version = 0;

  if (mvcc_data_.count(key)) {
    for (deque<Version*>::iterator it = mvcc_data_[key]->begin(); it != mvcc_data_[key]->end(); ++it) {
      if ((max_version < (*it)->version_id_) && ((*it)->version_id_ == txn_unique_id)) {
        max_version = (*it)->version_id_;
      }
    }
  }
  else {
    return false;
  }

  *result = max_version;

  return true;
}


// Check whether apply or abort the write
bool MVCCStorage::CheckWrite(Key key, int txn_unique_id) {
  if (mvcc_data_.count(key)) {
    if (!mvcc_data_[key]->empty()) {
      
      deque<Version*>::iterator max_version = mvcc_data_[key]->begin();
      
      for (deque<Version*>::iterator it = mvcc_data_[key]->begin(); it != mvcc_data_[key]->end(); ++it) {
        if (((*max_version)->version_id_ < (*it)->version_id_) && ((*it)->version_id_ <= txn_unique_id)) {
          max_version = it;
        }
      }

      if ((*(*max_version)--).max_read_id_ <= txn_unique_id) {
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

// MVCC Write, call this method only if CheckWrite return true.
void MVCCStorage::Write(Key key, Value value, int txn_unique_id) {
  // CPSC 438/538:
  //
  // Implement this method!

  // Hint: Insert a new version (malloc a Version and specify its value/version_id/max_read_id)
  // into the version_lists. Note that InitStorage() also calls this method to init storage.
  // Note that you don't have to call Lock(key) in this method, just
  // call Lock(key) before you call this method and call Unlock(key) afterward.

  if (mvcc_data_.count(key)) {
    Version newVersion = {0};
    newVersion.value_ = value;
    newVersion.max_read_id_ = 0;
    newVersion.version_id_ = txn_unique_id;

    (*mvcc_data_[key]).push_back(&newVersion);
  }
}
