#include "../include/dbm_async_worker.hpp"
#include "../include/utils/processor_jsfunc_wrapper.hpp"
#include "../include/utils/tsfn_types.hpp"

void dbmAsyncWorker::Execute()
{
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
        // If result equals default, treat as not found
        if (std::any_cast<std::string>(any_result) ==
            std::any_cast<std::string>(params[1])) {
            SetError("Key not found");
        }
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
    else if (operation == DBM_PROCESS) {
        std::string key = std::any_cast<std::string>(params[0]);
        bool writable = std::any_cast<bool>(params[1]);
        Napi::ThreadSafeFunction tsfn = std::any_cast<Napi::ThreadSafeFunction>(params[2]);
        processor_jsfunc_wrapper processor(tsfn);
        tkrzw::Status s = dbmReference->Process(key, &processor, writable);
        tsfn.Release();
        if (s != tkrzw::Status::SUCCESS) SetError("DBM Process failed");
    }

    // ---------------- Iterator operations ----------------
    else if (operation == ITERATOR_FIRST) {
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

    // ---------------- Index operations (from your original cpp) ----------------
    else if (operation == INDEX_ADD) {
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
    if (operation == DBM_GET_SIMPLE) {
        deferred_promise.Resolve(
            Napi::String::New(Env(), std::any_cast<std::string>(any_result)));
    }
    else if (operation == INDEX_GET_VALUES) {
        std::vector<std::string>& vec =
            std::any_cast<std::vector<std::string>&>(any_result);
        Napi::Array arr = Napi::Array::New(Env(), vec.size());
        for (size_t i = 0; i < vec.size(); ++i) {
            arr.Set(i, Napi::String::New(Env(), vec[i]));
        }
        deferred_promise.Resolve(arr);
    }
    else if (operation == ITERATOR_GET) {
        auto& pair = std::any_cast<std::pair<std::string, std::string>&>(any_result);
        Napi::Object obj = Napi::Object::New(Env());
        obj.Set("key", Napi::String::New(Env(), pair.first));
        obj.Set("value", Napi::String::New(Env(), pair.second));
        deferred_promise.Resolve(obj);
    }
    else if (operation == INDEX_GET_ITERATOR_VALUE) {
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