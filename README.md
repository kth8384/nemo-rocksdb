# Introduction

nemo-rocksdb is compatible with rocksdb, and I added TTL feature on it, supporting to put a record with any specified TTL. The performance is close to rocksdb, so you can use it without worring about performance penalty. It uses rocksdb as a submodule, so it's easy to upgrade rocksdb to the latest version if needed, now it's using rocksdb(v5.0.1). Besides, it is going to be used in [Nemo](https://github.com/Qihoo360/nemo) as a submodule.

# Notice!!!
Please **DO NOT!!!** use **master** branch directly in your project，cause we add some **non-universal** features on that required by [Nemo](https://github.com/Qihoo360/nemo) and you may not need, checkout and use **Tags or [public-release](https://github.com/KernelMaker/nemo-rocksdb/tree/public-release)** branch instead, that is PURE and COMPATIBLE!

# Features
## TTL
  DBNemo is derived from rocksdb::StackableDB, so it is compatible with rocksdb, you can use all the methods of rocksdb as usual, such as
  

|   Method    |
| --- |
|DBNemo::Put()         |
|DBNemo::Write()       |
|DBNemo::Get()         |
|DBNemo::Delete()      |
|DBNemo::Merge()       |
|DBNemo::NewIterator() |
|DBNemo::CompactRange()|

**How to expire key?**

just use **Put** or **Write** as usual, and add one more parameter(**int32_t ttl**) at last

```cpp
db->Put(rocksdb::WriteOptions(), "persistent_key", "value"); // never to be expired
db->Put(rocksdb::WriteOptions(), "expire_key", "value", 6); // will be expired after 6s

rocksdb::WriteBatch batch;
batch.Put("expire_key", "value"); // never to be expired
db->Write(rocksdb::WriteOptions(), &batch, 6); // will be expired after 6s
```

you can create iterator by **NewIterator()** to iterate db, it will ignore the expired records automaticly.

you can also use **Merge** method on record which has been set TTL, and when the record expired later, the existing_value would disappear, then, you may get different result from geting before the record expired. read [Test Merge example](https://github.com/KernelMaker/nemo-rocksdb/blob/master/example/example.cc) to learn more.

DBNemo uses specified inner CompactionFilter to drop the expired records in compaction process by default, you can add your own CompactionFilterFactory or CompactionFilter in rocksdb::Options, that's ok!

# Usage
```cpp
#include "db_nemo_impl.h"
#include <iostream>

int main() {
  rocksdb::DBNemo *db;
  rocksdb::Options options;
  options.create_if_missing = true;
  rocksdb::Status s = rocksdb::DBNemo::Open(options, "./db", &db);
  
  /*
   * 1. Insert a persistent record
   */
  s = db->Put(rocksdb::WriteOptions(), "persistent_key", "value");
  if (!s.ok()) {
    std::cout << "Put Error: " << s.ToString() << std::endl;
  }
  
  /*
   * 2. Insert a record with 6s TTL
   */
  s = db->Put(rocksdb::WriteOptions(), "expire_key", "value", 6);
  if (!s.ok()) {
    std::cout << "Put Error: " << s.ToString() << std::endl;
  }
  
  /*
   * 3. Insert three records in one batch with 8s TTL
   */
  rocksdb::WriteBatch batch;
  batch.Put("expire_key_1", "value1");
  batch.Put("expire_key_2", "value2");
  batch.Put("expire_key_3", "value3");
  s = db->Write(rocksdb::WriteOptions(), &batch, 8);
  if (!s.ok()) {
    std::cout << "Put Error: " << s.ToString() << std::endl;
  }
  
  /*
   * 4. Iterate db, the expired keys would be ignored automaticly
   */
  rocksdb::Iterator* iter = db->NewIterator(rocksdb::ReadOptions());
  while (true) {
    iter->SeekToFirst();
    std::cout << "---------------------------" << std::endl;
    while (iter->Valid()) {
      std::cout << iter->key().ToString() << " "
        << iter->value().ToString() << " "
        << static_cast<rocksdb::NemoIterator*>(iter)->timestamp()
        << std::endl;
      
      iter->Next();
    }
    std::cout << "---------------------------" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  delete db;
  return 0;
}

```
see [More](https://github.com/KernelMaker/nemo-rocksdb/blob/master/example/example.cc)

# Performance

It almost **costs NOTHING** to add TTL feature in NemoDB, so its performance is close to rocksdb.

In my environment, it used 390s to finish the benchmark(put 100 millions records), the benchmark code is [here](https://github.com/KernelMaker/nemo-rocksdb/blob/master/example/benchmark.cc)
