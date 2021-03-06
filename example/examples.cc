#include "include/db_nemo.h"
#include "include/db_nemo_checkpoint.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {

  class StringMergeOperator : public rocksdb::MergeOperator {
    public:
      virtual bool FullMergeV2(const MergeOperationInput& merge_in,
          MergeOperationOutput* merge_out) const override {
        if (merge_in.existing_value) {
          merge_out->new_value.append(merge_in.existing_value->data(), merge_in.existing_value->size());
        }
        for (const auto& operand : merge_in.operand_list) {
          merge_out->new_value.append(operand.data(), operand.size());
        }
        return true;
      }

      virtual bool PartialMergeMulti(const rocksdb::Slice& key,
          const std::deque<rocksdb::Slice>& operand_list,
          std::string* new_value,
          rocksdb::Logger* logger) const override {

        for (const auto& operand : operand_list) {
          new_value->append(operand.data(), operand.size());
          new_value->append(operand.data(), operand.size());
        }
        return true;
      }

      virtual const char* Name() const override {
        return "StringMergeOperator";
      }
  };

  rocksdb::DBNemo *db;
  rocksdb::Options options;
  options.create_if_missing = true;
	options.merge_operator.reset(new StringMergeOperator());
  rocksdb::Status s = rocksdb::DBNemo::Open(options, "./db", &db, 'H');

/*
 * 1. Test Put
 *
  {
  s = db->Put(rocksdb::WriteOptions(), "key_1", "value_1", 3);
  s = db->Put(rocksdb::WriteOptions(), "key_2", "value_2");
  if (!s.ok()) {
    std::cout << "Put Error: " << s.ToString() << std::endl;
  }

  std::string value;
  int times = 5;
  int32_t ttl;
  while (times--) {
    s = db->Get(rocksdb::ReadOptions(), "key_1", &value);
    if (s.ok()) {
      std::cout << "Get value: " << value << std::endl;
    } else if (s.IsNotFound()) {
      std::cout << "Get Nothing" << std::endl;
    } else {
      std::cout << "Get Error: " << s.ToString() << std::endl;
    }
    s = db->GetKeyTTL(rocksdb::ReadOptions(), "key_1", &ttl);
    std::cout << "GetKeyTTL return: " << s.ToString() << " ttl: " << ttl << std::endl;

    s = db->Get(rocksdb::ReadOptions(), "key_2", &value);
    if (s.ok()) {
      std::cout << "Get value: " << value << std::endl;
    } else if (s.IsNotFound()) {
      std::cout << "Get Nothing" << std::endl;
    } else {
      std::cout << "Get Error: " << s.ToString() << std::endl;
    }
    s = db->GetKeyTTL(rocksdb::ReadOptions(), "key_2", &ttl);
    std::cout << "GetKeyTTL return: " << s.ToString() << " ttl: " << ttl << std::endl;
    s = db->GetKeyTTL(rocksdb::ReadOptions(), "key_3", &ttl);
    std::cout << "GetKeyTTL return: " << s.ToString() << " ttl: " << ttl << std::endl;
//    std::this_thread::sleep_for(std::chrono::duration<int, std::ratio<1, 1> >(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  }
 *
 */

/*
 * 2. Test Iterator
 *
  {
  rocksdb::WriteBatch batch;
  batch.Put("key1", "value1");
  batch.Put("key2", "value2");
  batch.Put("key3", "value3");
  s = db->Write(rocksdb::WriteOptions(), &batch, 3);
  if (!s.ok()) {
    std::cout << "Write Error: " << s.ToString() << std::endl;
  }
  s = db->Put(rocksdb::WriteOptions(), "key4", "value4", 4);
  if (!s.ok()) {
    std::cout << "Put Error: " << s.ToString() << std::endl;
  }
  s = db->Put(rocksdb::WriteOptions(), "key5", "value5", 5);
  if (!s.ok()) {
    std::cout << "Put Error: " << s.ToString() << std::endl;
  }
  s = db->Put(rocksdb::WriteOptions(), "key6", "value6");
  if (!s.ok()) {
    std::cout << "Put Error: " << s.ToString() << std::endl;
  }

  rocksdb::Iterator* iter = db->NewIterator(rocksdb::ReadOptions());
  int times = 8;
  while (times--) {
    iter->SeekToFirst();
    std::cout << "---------------------------" << std::endl;
    while (iter->Valid()) {
      std::cout << iter->key().ToString() << " " << iter->value().ToString() << " " << static_cast<rocksdb::NemoIterator*>(iter)->timestamp() << std::endl;
      iter->Next();
    }
    std::cout << "---------------------------" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  }
 *
 */

/*
 * 3. Test CompactFilter
 *
  {
  rocksdb::Options op = db->GetOptions(); 
  std::cout << op.compaction_filter_factory << std::endl;
  s = db->Put(rocksdb::WriteOptions(), "persistent_key", "KernelMaker");
  if (!s.ok()) {
    std::cout << "Put Error: " << s.ToString() << std::endl;
  }

  for (int i = 0; i < 10; i++) {
    s = db->Put(rocksdb::WriteOptions(), std::to_string(i)+"_key", "value", 3);
    if (!s.ok()) {
      std::cout << "Put Error: " << s.ToString() << std::endl;
    }
  }
  for (int i = 0; i < 10; i++) {
    s = db->Put(rocksdb::WriteOptions(), std::to_string(i+10)+"_key", "value", 10);
    if (!s.ok()) {
      std::cout << "Put Error: " << s.ToString() << std::endl;
    }
  }
  std::string value;
  for (int i = 0; i < 20; i++) {
    s = db->Get(rocksdb::ReadOptions(), std::to_string(i)+"_key", &value);
    if (s.ok()) {
      std::cout << "Get key: " << std::to_string(i)+"_key" << " value: "<< value << std::endl;
    } else if (s.IsNotFound()) {
      std::cout << "Get Nothing" << std::endl;
    } else {
      std::cout << "Get Error: " << s.ToString() << std::endl;
    }
  }
  std::this_thread::sleep_for(std::chrono::seconds(5));
  s = db->CompactRange(rocksdb::CompactRangeOptions(), NULL, NULL);
  std::cout << "CompactRange return: " << s.ToString() << std::endl;
  for (int i = 0; i < 20; i++) {
    s = db->Get(rocksdb::ReadOptions(), std::to_string(i)+"_key", &value);
    if (s.ok()) {
      std::cout << "Get key: " << std::to_string(i)+"_key" << " value: "<< value << std::endl;
    } else if (s.IsNotFound()) {
      std::cout << "Get Nothing, key: " << std::to_string(i)+"_key" << std::endl;
    } else {
      std::cout << "Get Error: " << s.ToString() << std::endl;
    }
  }
  std::this_thread::sleep_for(std::chrono::seconds(6));
  s = db->CompactRange(rocksdb::CompactRangeOptions(), NULL, NULL);
  std::cout << "CompactRange return: " << s.ToString() << std::endl;
  for (int i = 0; i < 20; i++) {
    s = db->Get(rocksdb::ReadOptions(), std::to_string(i)+"_key", &value);
    if (s.ok()) {
      std::cout << "Get key: " << std::to_string(i)+"_key" << " value: "<< value << std::endl;
    } else if (s.IsNotFound()) {
      std::cout << "Get Nothing, key: " << std::to_string(i)+"_key" << std::endl;
    } else {
      std::cout << "Get Error: " << s.ToString() << std::endl;
    }
  }
  }
 *
 */

/*
 * 4. Test Merge
 *
  {
  s = db->Put(rocksdb::WriteOptions(), "merge", "a", 3);
  if (!s.ok()) {
    std::cout << "Put Error: " << s.ToString() << std::endl;
  }
  s = db->Merge(rocksdb::WriteOptions(), "merge", "b");
  if (!s.ok()) {
    std::cout << "Merge Error: " << s.ToString() << std::endl;
  }

  int times = 5;
  std::string value;
  while (times--) {
    value = std::string(nullptr, 0);
    s = db->Get(rocksdb::ReadOptions(), "merge", &value);
    if (s.ok()) {
      std::cout << "Get key: " << "merge" << " value: "<< value << std::endl;
    } else if (s.IsNotFound()) {
      std::cout << "Get Nothing, key: " << "merge" << std::endl;
    } else {
      std::cout << "Get Error: " << s.ToString() << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  }
 *
 */

/*
 * 5. Test MultiStructure Compaction
 *
  {
  std::string meta_key = "H";
  meta_key.append("abc");
  s = db->Put(rocksdb::WriteOptions(), meta_key, "meta_value_1");
  s = db->Put(rocksdb::WriteOptions(), meta_key, "meta_value_2");

  std::string data_key = "h";
  uint8_t len = 3;
  data_key.append((char*)&len, 1);
  data_key.append("abc_1");
  s = db->Put(rocksdb::WriteOptions(), data_key, "data_value_1");

  data_key = "h";
  len = 3;
  data_key.append((char*)&len, 1);
  data_key.append("abc_2");
  s = db->Put(rocksdb::WriteOptions(), data_key, "data_value_2");

  s = db->CompactRange(rocksdb::CompactRangeOptions(), NULL, NULL);

  }
 *
 */

/*
 * 6. Test MultiStructure Iterator
 *
  {
  s = db->Put(rocksdb::WriteOptions(), "Habc", "meta_value_1");
  s = db->Put(rocksdb::WriteOptions(), "Habc", "meta_value_2", 3);


  std::string data_key = "h";
  uint8_t len = 3;
  data_key.append((char*)&len, 1);
  data_key.append("abc_1");
  s = db->Put(rocksdb::WriteOptions(), data_key, "data_value_1");

  data_key = "h";
  len = 3;
  data_key.append((char*)&len, 1);
  data_key.append("abc_2");
  s = db->Put(rocksdb::WriteOptions(), data_key, "data_value_2");

  data_key = "h";
  len = 3;
  data_key.append((char*)&len, 1);
  data_key.append("abc_3");
  s = db->Put(rocksdb::WriteOptions(), data_key, "data_value_3");

  data_key = "h";
  len = 3;
  data_key.append((char*)&len, 1);
  data_key.append("abc_4");
  s = db->Put(rocksdb::WriteOptions(), data_key, "data_value_4");

  data_key = "h";
  len = 3;
  data_key.append((char*)&len, 1);
  data_key.append("abc_5");
  s = db->Put(rocksdb::WriteOptions(), data_key, "data_value_5");

  data_key = "h";
  len = 3;
  data_key.append((char*)&len, 1);
  data_key.append("abc_6");
  s = db->Put(rocksdb::WriteOptions(), data_key, "data_value_6");

  rocksdb::Iterator* iter = db->NewIterator(rocksdb::ReadOptions());
  int times = 6;
  while (times--) {
    std::cout << "---------------------------" << std::endl;
    iter->SeekToFirst();
    while (iter->Valid()) {
      std::cout << iter->key().ToString() << " " << iter->value().ToString() << " " << static_cast<rocksdb::NemoIterator*>(iter)->timestamp() << std::endl;
      iter->Next();
    }
    std::cout << "---------------------------" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  delete iter;

  s = db->CompactRange(rocksdb::CompactRangeOptions(), NULL, NULL);

  }
 *
 */

/*
 * 7. Test PutWithKeyVersion;
 *
  {
  s = db->Put(rocksdb::WriteOptions(), "Habc", "meta_value_1");

  std::string data_key = "h";
  uint8_t len = 3;
  data_key.append((char*)&len, 1);
  data_key.append("abc_1");
  s = db->Put(rocksdb::WriteOptions(), data_key, "data_value_1");

  rocksdb::Iterator* iter = db->NewIterator(rocksdb::ReadOptions());
  std::cout << "---------------------------" << std::endl;
  iter->SeekToFirst();
  while (iter->Valid()) {
    std::cout << iter->key().ToString() << " " << iter->value().ToString() << " " << static_cast<rocksdb::NemoIterator*>(iter)->timestamp() << std::endl;
    iter->Next();
  }
  std::cout << "---------------------------" << std::endl;
  delete iter;

  s = db->PutWithKeyVersion(rocksdb::WriteOptions(), "Habc", "meta_value2");
  iter = db->NewIterator(rocksdb::ReadOptions());
  std::cout << "---------------------------" << std::endl;
  iter->SeekToFirst();
  while (iter->Valid()) {
    std::cout << iter->key().ToString() << " " << iter->value().ToString() << " " << static_cast<rocksdb::NemoIterator*>(iter)->timestamp() << std::endl;
    iter->Next();
  }
  std::cout << "---------------------------" << std::endl;
  delete iter;
  }
 *
 */

/*
 * 8. Test WriteWithOldKeyTTL
 *
  {
  rocksdb::WriteBatch batch;
  batch.Put("Habc", "meta_value_1");
  s = db->Write(rocksdb::WriteOptions(), &batch, 3);
  s = db->Put(rocksdb::WriteOptions(), "Habc", "meta_value_1", 2);

  std::string data_key = "h";
  uint8_t len = 3;
  data_key.append((char*)&len, 1);
  data_key.append("abc_1");
  s = db->Put(rocksdb::WriteOptions(), data_key, "data_value_1");

  rocksdb::Iterator* iter;
  int times = 5;
  while (times--) {
    iter = db->NewIterator(rocksdb::ReadOptions());
    std::cout << "---------------------------" << std::endl;
    iter->SeekToFirst();
    while (iter->Valid()) {
      std::cout << iter->key().ToString() << " " << iter->value().ToString() << " " << static_cast<rocksdb::NemoIterator*>(iter)->timestamp() << std::endl;
      iter->Next();
    }
    std::cout << "---------------------------" << std::endl;
    if (times == 2) {
      s = db->WriteWithOldKeyTTL(rocksdb::WriteOptions(), &batch);
    }
    delete iter;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  }
 *
 */

/*
 * 9. Test Checkpoint
 */
  {
  s = db->Put(rocksdb::WriteOptions(), "Habc", "meta_value_1");
  s = db->Put(rocksdb::WriteOptions(), "Hefg", "meta_value_2", 3);
  
  rocksdb::DBNemoCheckpoint* cp;
  std::vector<std::string> live_files;
  rocksdb::VectorLogPtr live_wal_files;
  uint64_t manifest_file_size;
  uint64_t sequence_number;

  rocksdb::DBNemoCheckpoint::Create(db, &cp);
  s = cp->GetCheckpointFiles(live_files, live_wal_files,
      manifest_file_size, sequence_number);
  s = cp->CreateCheckpointWithFiles("./tmp", live_files, live_wal_files,
      manifest_file_size, sequence_number);

//  s = cp->CreateCheckpoint("./tmp");
  delete cp;
  std::cout << "CreateCheckpoint return: " << s.ToString() << std::endl;

  rocksdb::DBNemo *tdb;
  rocksdb::Options options;
  options.create_if_missing = true;
	options.merge_operator.reset(new StringMergeOperator());
  rocksdb::Status s = rocksdb::DBNemo::Open(options, "./tmp", &tdb, 'H');
  std::cout << "open Checkpoint db, return: " << s.ToString() << std::endl;
  std::cout << "Switch to Checkpoint db" << std::endl;

  int times = 5;
  int32_t ttl = 0;
  std::string value;
  while (times--) {
    s = tdb->Get(rocksdb::ReadOptions(), "Habc", &value);
    if (s.ok()) {
      std::cout << "Get value: " << value << std::endl;
    } else if (s.IsNotFound()) {
      std::cout << "Get Nothing" << std::endl;
    } else {
      std::cout << "Get Error: " << s.ToString() << std::endl;
    }
    s = tdb->GetKeyTTL(rocksdb::ReadOptions(), "Habc", &ttl);
    std::cout << "GetKeyTTL return: " << s.ToString() << " ttl: " << ttl << std::endl;

    s = tdb->Get(rocksdb::ReadOptions(), "Hefg", &value);
    if (s.ok()) {
      std::cout << "Get value: " << value << std::endl;
    } else if (s.IsNotFound()) {
      std::cout << "Get Nothing" << std::endl;
    } else {
      std::cout << "Get Error: " << s.ToString() << std::endl;
    }
    s = tdb->GetKeyTTL(rocksdb::ReadOptions(), "Hefg", &ttl);
    std::cout << "GetKeyTTL return: " << s.ToString() << " ttl: " << ttl << std::endl;
//    std::this_thread::sleep_for(std::chrono::duration<int, std::ratio<1, 1> >(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  delete tdb;
  
  }
	delete db;
}
