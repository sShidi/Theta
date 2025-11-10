#include "../include/dbm_async_worker.hpp"
#include "../include/utils/processor_jsfunc_wrapper.hpp"
#include "../include/utils/tsfn_types.hpp"
#include <fstream>
#include <regex>

void dbmAsyncWorker::Execute()
{
    auto get_view = [](const std::string& s) -> std::string_view {
        if (s == std::string(tkrzw::DBM::ANY_DATA)) {
            return tkrzw::DBM::ANY_DATA;
        }
        return s;
    };

    // ---------------- DBM operations ----------------
    if (operation == DBM_SET) {
        tkrzw::Status s = dbmReference->Set(
            std::any_cast<std::string>(params[0]),
            std::any_cast<std::string>(params[1]));
        if (s != tkrzw::Status::SUCCESS) SetError("DBM Set failed");
    }
    else if (operation == DBM_APPEND) {
        tkrzw::Status s = dbmReference->Append(
            std::any_cast<std::string>(params[0]),
            std::any_cast<std::string>(params[1]),
            std::any_cast<std::string>(params[2]));
        if (s != tkrzw::Status::SUCCESS) SetError("DBM Append failed");
    }
    else if (operation == DBM_GET_SIMPLE) {
        any_result = dbmReference->GetSimple(
            std::any_cast<std::string>(params[0]),
            std::any_cast<std::string>(params[1]));
//        // If result equals default, treat as not found
//        if (std::any_cast<std::string>(any_result) ==
//            std::any_cast<std::string>(params[1])) {
//            SetError("Key not found");
//        }
    }
    else if (operation == DBM_REMOVE) {
        tkrzw::Status s = dbmReference->Remove(
            std::any_cast<std::string>(params[0]));
        if (s != tkrzw::Status::SUCCESS) SetError("DBM Remove failed");
    }
    else if (operation == DBM_COMPARE_EXCHANGE) {
        std::string exp = std::any_cast<std::string>(params[1]);
        std::string des = std::any_cast<std::string>(params[2]);
        std::string_view exp_view = get_view(exp);
        std::string_view des_view = get_view(des);
        tkrzw::Status s = dbmReference->CompareExchange(
            std::any_cast<std::string>(params[0]), exp_view, des_view);
        if (s != tkrzw::Status::SUCCESS) SetError("DBM CompareExchange failed");
    }
    else if (operation == DBM_INCREMENT) {
        int64_t current = 0;
        tkrzw::Status s = dbmReference->Increment(
            std::any_cast<std::string>(params[0]),
            std::any_cast<int64_t>(params[1]),
            &current,
            std::any_cast<int64_t>(params[2]));
        if (s != tkrzw::Status::SUCCESS) SetError("DBM Increment failed");
        any_result = current;
    }
    else if (operation == DBM_COMPARE_EXCHANGE_MULTI) {
        auto expected = std::any_cast<std::vector<std::pair<std::string, std::string>>>(params[0]);
        auto desired = std::any_cast<std::vector<std::pair<std::string, std::string>>>(params[1]);
        std::vector<std::pair<std::string_view, std::string_view>> exp_pairs, des_pairs;
        for (const auto& p : expected) {
            exp_pairs.emplace_back(p.first, get_view(p.second));
        }
        for (const auto& p : desired) {
            des_pairs.emplace_back(p.first, get_view(p.second));
        }
        tkrzw::Status s = dbmReference->CompareExchangeMulti(exp_pairs, des_pairs);
        if (s != tkrzw::Status::SUCCESS) SetError("DBM CompareExchangeMulti failed");
    }
    else if (operation == DBM_REKEY) {
        tkrzw::Status s = dbmReference->Rekey(
            std::any_cast<std::string>(params[0]),
            std::any_cast<std::string>(params[1]),
            std::any_cast<bool>(params[2]),
            std::any_cast<bool>(params[3]));
        if (s != tkrzw::Status::SUCCESS) SetError("DBM Rekey failed");
    }
    else if (operation == DBM_PROCESS_MULTI) {
        auto keys = std::any_cast<std::vector<std::string>>(params[0]);
        TSFN tsfn = std::any_cast<TSFN>(params[1]);
        bool writable = std::any_cast<bool>(params[2]);
        processor_jsfunc_wrapper processor(tsfn);
        std::vector<std::pair<std::string_view, tkrzw::DBM::RecordProcessor*>> key_proc_pairs;
        for (const auto& key : keys) {
            key_proc_pairs.emplace_back(key, &processor);
        }
        tkrzw::Status s = dbmReference->ProcessMulti(key_proc_pairs, writable);
        tsfn.Release();
        if (s != tkrzw::Status::SUCCESS) SetError("DBM ProcessMulti failed");
    }
    else if (operation == DBM_PROCESS_FIRST) {
        TSFN tsfn = std::any_cast<TSFN>(params[0]);
        bool writable = std::any_cast<bool>(params[1]);
        processor_jsfunc_wrapper processor(tsfn);
        tkrzw::Status s = dbmReference->ProcessFirst(&processor, writable);
        tsfn.Release();
        if (s != tkrzw::Status::SUCCESS) SetError("DBM ProcessFirst failed");
    }
    else if (operation == DBM_PROCESS_EACH) {
        TSFN tsfn = std::any_cast<TSFN>(params[0]);
        bool writable = std::any_cast<bool>(params[1]);
        processor_jsfunc_wrapper processor(tsfn);
        tkrzw::Status s = dbmReference->ProcessEach(&processor, writable);
        tsfn.Release();
        if (s != tkrzw::Status::SUCCESS) SetError("DBM ProcessEach failed");
    }
    else if (operation == DBM_COUNT) {
        int64_t count = 0;
        tkrzw::Status s = dbmReference->Count(&count);
        if (s != tkrzw::Status::SUCCESS) SetError("DBM Count failed");
        any_result = count;
    }
    else if (operation == DBM_GET_FILE_SIZE) {
        int64_t size = 0;
        tkrzw::Status s = dbmReference->GetFileSize(&size);
        if (s != tkrzw::Status::SUCCESS) SetError("DBM GetFileSize failed");
        any_result = size;
    }
    else if (operation == DBM_GET_FILE_PATH) {
        std::string path;
        tkrzw::Status s = dbmReference->GetFilePath(&path);
        if (s != tkrzw::Status::SUCCESS) SetError("DBM GetFilePath failed");
        any_result = path;
    }
    else if (operation == DBM_GET_TIMESTAMP) {
        double timestamp = 0.0;
        tkrzw::Status s = dbmReference->GetTimestamp(&timestamp);
        if (s != tkrzw::Status::SUCCESS) SetError("DBM GetTimestamp failed");
        any_result = timestamp;
    }
    else if (operation == DBM_CLEAR) {
        tkrzw::Status s = dbmReference->Clear();
        if (s != tkrzw::Status::SUCCESS) SetError("DBM Clear failed");
    }
    else if (operation == DBM_INSPECT) {
        any_result = dbmReference->Inspect();
    }
    else if (operation == DBM_SHOULD_BE_REBUILT) {
        bool result = false;
        tkrzw::Status s = dbmReference->ShouldBeRebuilt(&result);
        if (s != tkrzw::Status::SUCCESS || !result) {
            SetError("ShouldBeRebuilt check failed or not needed");
        }
    }
    else if (operation == DBM_REBUILD) {
        tkrzw::Status s = dbmReference->RebuildAdvanced(
            std::any_cast<std::map<std::string,std::string>>(params[0]));
        if (s != tkrzw::Status::SUCCESS) {
            SetError("DBM Rebuild failed");
        }
    }
    else if (operation == DBM_SYNC) {
        tkrzw::Status s = dbmReference->Synchronize(
            std::any_cast<bool>(params[0]));
        if (s != tkrzw::Status::SUCCESS) {
            SetError("DBM Sync failed");
        }
    }
    else if (operation == DBM_SEARCH) {
        std::string mode = std::any_cast<std::string>(params[0]);
        std::string pattern = std::any_cast<std::string>(params[1]);
        size_t max = std::any_cast<std::size_t>(params[2]);
        std::vector<std::string> keys;
        auto iter = dbmReference->MakeIterator();
        bool is_ordered = dbmReference->IsOrdered();
        tkrzw::Status s;

        if (mode == "begin") {
            if (is_ordered) {
                s = iter->Jump(pattern);
                if (s == tkrzw::Status::SUCCESS) {
                    while (keys.size() < max) {
                        std::string key;
                        s = iter->Get(&key, nullptr);
                        if (s != tkrzw::Status::SUCCESS) break;
                        if (key.rfind(pattern, 0) != 0) break;
                        keys.push_back(key);
                        s = iter->Next();
                    }
                }
            } else {
                s = iter->First();
                while (keys.size() < max) {
                    std::string key;
                    s = iter->Get(&key, nullptr);
                    if (s != tkrzw::Status::SUCCESS) break;
                    if (key.rfind(pattern, 0) == 0) {
                        keys.push_back(key);
                    }
                    s = iter->Next();
                }
            }
        }else if (mode == "contain") {
			s = iter->First();
			while (keys.size() < max) {
				 std::string key;
				 s = iter->Get(&key, nullptr);
				 if (s != tkrzw::Status::SUCCESS) break;
				 if (key.find(pattern) != std::string::npos) {
					 keys.push_back(key);
				 }
				 s = iter->Next();
			}
		}else if (mode == "end") {
			s = iter->First();
			while (keys.size() < max) {
				 std::string key;
				 s = iter->Get(&key, nullptr);
				 if (s != tkrzw::Status::SUCCESS) break;
				 if (key.length() >= pattern.length() &&
					 key.compare(key.length() - pattern.length(), pattern.length(), pattern) == 0) {
					 keys.push_back(key);
				 }
				 s = iter->Next();
			}
		}else if (mode == "regex") {
            std::regex re(pattern);
            s = iter->First();
            while (keys.size() < max) {
                std::string key;
                s = iter->Get(&key, nullptr);
                if (s != tkrzw::Status::SUCCESS) break;
                if (std::regex_match(key, re)) {
                    keys.push_back(key);
                }
                s = iter->Next();
            }
        } // add other modes if needed
        any_result = keys;
    }
    else if (operation == DBM_EXPORT_KEYS_AS_LINES) {
        std::string dest_path = std::any_cast<std::string>(params[0]);
        std::ofstream file(dest_path);
        if (!file) {
            SetError("Failed to open file for exportKeysAsLines");
            return;
        }
        auto iter = dbmReference->MakeIterator();
        tkrzw::Status s = iter->First();
        if (s != tkrzw::Status::SUCCESS) {
            SetError("Iterator First failed");
            return;
        }
        while (true) {
            std::string key;
            s = iter->Get(&key, nullptr);
            if (s != tkrzw::Status::SUCCESS) break;
            file << key << '\n';
            if (file.fail()) {
                SetError("Write failed in exportKeysAsLines");
                return;
            }
            iter->Next();
        }
        file.close();
        if (file.fail()) SetError("DBM ExportKeysAsLines failed");
    }
    else if (operation == DBM_RESTORE_DATABASE) {
        tkrzw::Status s = tkrzw::PolyDBM::RestoreDatabase(
            std::any_cast<std::string>(params[0]),
            std::any_cast<std::string>(params[1]),
            std::any_cast<std::string>(params[2]),
            std::any_cast<int64_t>(params[3]));
        if (s != tkrzw::Status::SUCCESS) SetError("DBM RestoreDatabase failed");
    }
    else if (operation == DBM_PROCESS) {
        std::string key = std::any_cast<std::string>(params[0]);
        bool writable = std::any_cast<bool>(params[1]);
        TSFN tsfn = std::any_cast<TSFN>(params[2]);
        processor_jsfunc_wrapper processor(tsfn);
        tkrzw::Status s = dbmReference->Process(key, &processor, writable);
        tsfn.Release();
        if (s != tkrzw::Status::SUCCESS) SetError("DBM Process failed");
    }

    // ---------------- Iterator operations ----------------
    if (operation == ITERATOR_FIRST) {
        tkrzw::Status s = (*iteratorReference)->First();
        if (s != tkrzw::Status::SUCCESS) SetError("Iterator First failed");
    }
    else if (operation == ITERATOR_LAST) {
        tkrzw::Status s = (*iteratorReference)->Last();
        if (s != tkrzw::Status::SUCCESS) SetError("Iterator Last failed");
    }
    else if (operation == ITERATOR_JUMP) {
        std::string key = std::any_cast<std::string>(params[0]);
        tkrzw::Status s = (*iteratorReference)->Jump(key);
        if (s != tkrzw::Status::SUCCESS) SetError("Iterator Jump failed");
    }
    else if (operation == ITERATOR_JUMP_LOWER) {
        std::string key = std::any_cast<std::string>(params[0]);
        tkrzw::Status s = (*iteratorReference)->JumpLower(key, false);
        if (s != tkrzw::Status::SUCCESS) SetError("Iterator JumpLower failed");
    }
    else if (operation == ITERATOR_JUMP_UPPER) {
        std::string key = std::any_cast<std::string>(params[0]);
        tkrzw::Status s = (*iteratorReference)->JumpUpper(key, false);
        if (s != tkrzw::Status::SUCCESS) SetError("Iterator JumpUpper failed");
    }
    else if (operation == ITERATOR_NEXT) {
        tkrzw::Status s = (*iteratorReference)->Next();
        if (s != tkrzw::Status::SUCCESS) SetError("Iterator Next failed");
    }
    else if (operation == ITERATOR_PREVIOUS) {
        tkrzw::Status s = (*iteratorReference)->Previous();
        if (s != tkrzw::Status::SUCCESS) SetError("Iterator Previous failed");
    }
    else if (operation == ITERATOR_GET) {
        std::string key, value;
        tkrzw::Status s = (*iteratorReference)->Get(&key, &value);
        if (s != tkrzw::Status::SUCCESS) {
            SetError("Iterator Get failed");
        } else {
            any_result = std::make_pair(key, value);
        }
    }
    else if (operation == ITERATOR_SET) {
        std::string value = std::any_cast<std::string>(params[0]);
        tkrzw::Status s = (*iteratorReference)->Set(value);
        if (s != tkrzw::Status::SUCCESS) SetError("Iterator Set failed");
    }
    else if (operation == ITERATOR_REMOVE) {
        tkrzw::Status s = (*iteratorReference)->Remove();
        if (s != tkrzw::Status::SUCCESS) SetError("Iterator Remove failed");
    }

    // ---------------- Index operations ----------------
    if (operation == INDEX_ADD) {
        tkrzw::Status s = indexReference->Add(
            std::any_cast<std::string>(params[0]),
            std::any_cast<std::string>(params[1]));
        if (s != tkrzw::Status::SUCCESS) SetError("Index Add failed");
    }
    else if (operation == INDEX_GET_VALUES) {
        any_result = indexReference->GetValues(
            std::any_cast<std::string>(params[0]),
            std::any_cast<std::size_t>(params[1]));
    }
    else if (operation == INDEX_CHECK) {
        bool ok = indexReference->Check(
            std::any_cast<std::string>(params[0]),
            std::any_cast<std::string>(params[1]));
        if (!ok) SetError("Index Check failed");
    }
    else if (operation == INDEX_REMOVE) {
        tkrzw::Status s = indexReference->Remove(
            std::any_cast<std::string>(params[0]),
            std::any_cast<std::string>(params[1]));
        if (s != tkrzw::Status::SUCCESS) SetError("Index Remove failed");
    }
    else if (operation == INDEX_SHOULD_BE_REBUILT) {
        bool result = false;
        tkrzw::Status s = indexReference->GetInternalDBM()->ShouldBeRebuilt(&result);
        if (s != tkrzw::Status::SUCCESS || !result) {
            SetError("Index ShouldBeRebuilt failed or not needed");
        }
    }
    else if (operation == INDEX_REBUILD) {
        tkrzw::Status s = indexReference->Rebuild();
        if (s != tkrzw::Status::SUCCESS) SetError("Index Rebuild failed");
    }
    else if (operation == INDEX_SYNC) {
        tkrzw::Status s = indexReference->Synchronize(std::any_cast<bool>(params[0]));
        if (s != tkrzw::Status::SUCCESS) SetError("Index Sync failed");
    }
    else if (operation == INDEX_MAKE_JUMP_ITERATOR) {
        std::string partialKey = std::any_cast<std::string>(params[0]);
        tkrzw::PolyIndex::Iterator* jump_iter =
            std::any_cast<tkrzw::PolyIndex::Iterator*>(params[1]);
        jump_iter->Jump(partialKey);
    }
    else if (operation == INDEX_GET_ITERATOR_VALUE) {
        tkrzw::PolyIndex::Iterator* jump_iter =
            std::any_cast<tkrzw::PolyIndex::Iterator*>(params[0]);
        std::pair<std::string, std::string> kv;
        if (jump_iter->Get(&kv.first, &kv.second)) {
            any_result = kv;
        } else {
            SetError("Index iterator Get failed");
        }
    }
    else if (operation == INDEX_CONTINUE_ITERATION) {
        tkrzw::PolyIndex::Iterator* jump_iter =
            std::any_cast<tkrzw::PolyIndex::Iterator*>(params[0]);
        jump_iter->Next();
    }
}

void dbmAsyncWorker::OnOK()
{
    if (operation == DBM_GET_SIMPLE || operation == DBM_GET_FILE_PATH) {
        deferred_promise.Resolve(
            Napi::String::New(Env(), std::any_cast<std::string>(any_result)));
    } else if (operation == DBM_COUNT || operation == DBM_GET_FILE_SIZE || operation == DBM_INCREMENT) {
        deferred_promise.Resolve(
            Napi::Number::New(Env(), std::any_cast<int64_t>(any_result)));
    } else if (operation == DBM_CLEAR) {
       deferred_promise.Resolve(Napi::Boolean::New(Env(), true));
    } else if (operation == DBM_GET_TIMESTAMP) {
        deferred_promise.Resolve(
            Napi::Number::New(Env(), std::any_cast<double>(any_result)));
    } else if (operation == DBM_INSPECT) {
        auto& vec = std::any_cast<std::vector<std::pair<std::string, std::string>>&>(any_result);
        Napi::Object obj = Napi::Object::New(Env());
        for (const auto& p : vec) {
            obj.Set(p.first, p.second);
        }
        deferred_promise.Resolve(obj);
    } else if (operation == DBM_SEARCH || operation == INDEX_GET_VALUES) {
        auto& vec = std::any_cast<std::vector<std::string>&>(any_result);
        Napi::Array arr = Napi::Array::New(Env(), vec.size());
        for (size_t i = 0; i < vec.size(); ++i) {
            arr.Set(i, Napi::String::New(Env(), vec[i]));
        }
        deferred_promise.Resolve(arr);
    }
    else if (operation == ITERATOR_GET || operation == INDEX_GET_ITERATOR_VALUE) {
        auto& pair = std::any_cast<std::pair<std::string, std::string>&>(any_result);
        Napi::Object obj = Napi::Object::New(Env());
        obj.Set("key", Napi::String::New(Env(), pair.first));
        obj.Set("value", Napi::String::New(Env(), pair.second));
        deferred_promise.Resolve(obj);
    }
    else {
        deferred_promise.Resolve(Napi::Boolean::New(Env(), true));
    }
}

void dbmAsyncWorker::OnError(const Napi::Error& err)
{
    deferred_promise.Reject(err.Value());
}