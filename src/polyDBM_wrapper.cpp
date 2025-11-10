#include "../include/polyDBM_wrapper.hpp"
#include "../include/dbm_async_worker.hpp"
#include "../include/utils/tsfn_types.hpp"
#include <iostream>

// Constructor
polyDBM_wrapper::polyDBM_wrapper(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<polyDBM_wrapper>(info) {
    Napi::Env env = info.Env();

    std::map<std::string, std::string> optional_tuning_params = parseConfig(env, info[0]);
    std::string dbmPath = info[1].As<Napi::String>();

    tkrzw::Status opening_status =
        dbm.OpenAdvanced(dbmPath, true,
                         tkrzw::File::OPEN_DEFAULT | tkrzw::File::OPEN_SYNC_HARD,
                         optional_tuning_params).OrDie();
    if (opening_status != tkrzw::Status::SUCCESS) {
        Napi::TypeError::New(env, opening_status.GetMessage().c_str())
            .ThrowAsJavaScriptException();
    }
}

// Basic methods
Napi::Value polyDBM_wrapper::set(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for set").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().Utf8Value();
    std::string value = info[1].As<Napi::String>().Utf8Value();

    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_SET, key, value);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::append(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for append").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().Utf8Value();
    std::string value = info[1].As<Napi::String>().Utf8Value();
    std::string delimiter = info.Length() > 2 && info[2].IsString() ? info[2].As<Napi::String>().Utf8Value() : "";

    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_APPEND, key, value, delimiter);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::getSimple(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for getSimple").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().Utf8Value();
    std::string default_value = info.Length() > 1 && info[1].IsString() ? info[1].As<Napi::String>().Utf8Value() : "";

    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_GET_SIMPLE, key, default_value);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::shouldBeRebuilt(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_SHOULD_BE_REBUILT);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::rebuild(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::map<std::string, std::string> optional_tuning_params;
    if (info.Length() > 0 && info[0].IsObject()) {
        optional_tuning_params = parseConfig(env, info[0]);
    }
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_REBUILD, optional_tuning_params);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::sync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool sync_hard = info.Length() > 0 ? info[0].As<Napi::Boolean>() : false;
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_SYNC, sync_hard);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::process(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsFunction() || !info[2].IsBoolean()) {
        Napi::TypeError::New(env, "Invalid arguments for process").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().Utf8Value();
    Napi::Function jsprocessor = info[1].As<Napi::Function>();
    bool writable = info[2].As<Napi::Boolean>();

    TSFN tsfn = TSFN::New(env, jsprocessor, "processor_jsfunc_wrapper tsfn", 0, 1);

    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_PROCESS, key, writable, tsfn);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::close(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    tkrzw::Status close_status = dbm.Close();
    if (close_status != tkrzw::Status::SUCCESS) {
        Napi::TypeError::New(env, close_status.GetMessage().c_str()).ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    return Napi::Boolean::New(env, true);
}

// Additional DBM methods
Napi::Value polyDBM_wrapper::get(const Napi::CallbackInfo& info) {
    return getSimple(info);
}

Napi::Value polyDBM_wrapper::remove(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for remove").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().Utf8Value();
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_REMOVE, key);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::compareExchange(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for compareExchange").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().Utf8Value();
    std::string expected = info[1].As<Napi::String>().Utf8Value();
    std::string desired = info[2].As<Napi::String>().Utf8Value();
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_COMPARE_EXCHANGE, key, expected, desired);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::increment(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for increment").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().Utf8Value();
    int64_t inc = info.Length() > 1 ? info[1].As<Napi::Number>().Int64Value() : 1;
    int64_t init = info.Length() > 2 ? info[2].As<Napi::Number>().Int64Value() : 0;
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_INCREMENT, key, inc, init);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::compareExchangeMulti(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsArray() || !info[1].IsArray()) {
        Napi::TypeError::New(env, "Invalid arguments for compareExchangeMulti").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    Napi::Array expArr = info[0].As<Napi::Array>();
    Napi::Array desArr = info[1].As<Napi::Array>();
    std::vector<std::pair<std::string, std::string>> expected, desired;
    for (uint32_t i = 0; i < expArr.Length(); ++i) {
        if (!expArr.Get(i).IsObject()) continue;
        Napi::Object obj = expArr.Get(i).As<Napi::Object>();
        if (!obj.Has("key") || !obj.Has("value")) continue;
        std::string k = obj.Get("key").IsString() ? obj.Get("key").As<Napi::String>().Utf8Value() : "";
        std::string v = obj.Get("value").IsString() ? obj.Get("value").As<Napi::String>().Utf8Value() : "";
        if (obj.Get("value").IsNull() || obj.Get("value").IsUndefined()) v = "";
        expected.emplace_back(k, v);
    }
    for (uint32_t i = 0; i < desArr.Length(); ++i) {
        if (!desArr.Get(i).IsObject()) continue;
        Napi::Object obj = desArr.Get(i).As<Napi::Object>();
        if (!obj.Has("key") || !obj.Has("value")) continue;
        std::string k = obj.Get("key").IsString() ? obj.Get("key").As<Napi::String>().Utf8Value() : "";
        std::string v = obj.Get("value").IsString() ? obj.Get("value").As<Napi::String>().Utf8Value() : "";
        if (obj.Get("value").IsNull() || obj.Get("value").IsUndefined()) v = "";
        desired.emplace_back(k, v);
    }
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_COMPARE_EXCHANGE_MULTI, expected, desired);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::rekey(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for rekey").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string old_key = info[0].As<Napi::String>().Utf8Value();
    std::string new_key = info[1].As<Napi::String>().Utf8Value();
    bool overwrite = info.Length() > 2 ? info[2].As<Napi::Boolean>() : true;
    bool copying = info.Length() > 3 ? info[3].As<Napi::Boolean>() : false;
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_REKEY, old_key, new_key, overwrite, copying);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::processMulti(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsArray() || !info[1].IsFunction()) {
        Napi::TypeError::New(env, "Invalid arguments for processMulti").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    Napi::Array keysArr = info[0].As<Napi::Array>();
    Napi::Function jsprocessor = info[1].As<Napi::Function>();
    bool writable = info.Length() > 2 ? info[2].As<Napi::Boolean>() : false;
    std::vector<std::string> keys;
    for (uint32_t i = 0; i < keysArr.Length(); ++i) {
        keys.push_back(keysArr.Get(i).As<Napi::String>().Utf8Value());
    }
    TSFN tsfn = TSFN::New(env, jsprocessor, "processMulti tsfn", 0, 1);
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_PROCESS_MULTI, keys, tsfn, writable);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::processFirst(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Invalid arguments for processFirst").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    Napi::Function jsprocessor = info[0].As<Napi::Function>();
    bool writable = info.Length() > 1 ? info[1].As<Napi::Boolean>() : false;
    TSFN tsfn = TSFN::New(env, jsprocessor, "processFirst tsfn", 0, 1);
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_PROCESS_FIRST, tsfn, writable);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::processEach(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Invalid arguments for processEach").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    Napi::Function jsprocessor = info[0].As<Napi::Function>();
    bool writable = info.Length() > 1 ? info[1].As<Napi::Boolean>() : false;
    TSFN tsfn = TSFN::New(env, jsprocessor, "processEach tsfn", 0, 1);
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_PROCESS_EACH, tsfn, writable);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::count(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_COUNT);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::getFileSize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_GET_FILE_SIZE);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::getFilePath(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_GET_FILE_PATH);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::getTimestamp(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_GET_TIMESTAMP);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::clear(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_CLEAR);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::inspect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_INSPECT);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::isOpen(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, dbm.IsOpen());
}

Napi::Value polyDBM_wrapper::isWritable(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, dbm.IsWritable());
}

Napi::Value polyDBM_wrapper::isHealthy(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, dbm.IsHealthy());
}

Napi::Value polyDBM_wrapper::isOrdered(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, dbm.IsOrdered());
}

Napi::Value polyDBM_wrapper::search(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsNumber()) {
        Napi::TypeError::New(env, "Invalid arguments for search").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string mode = info[0].As<Napi::String>().Utf8Value();
    std::set<std::string> supported_modes{"contain", "begin", "end", "regex", "edit", "token", "tokenprefix"};
    if (supported_modes.find(mode) == supported_modes.end()) {
        Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
        deferred.Reject(Napi::TypeError::New(env, "Search failed: unknown search mode").Value());
        return deferred.Promise();
    }
    std::string pattern = info[1].As<Napi::String>().Utf8Value();
    size_t capacity = info[2].As<Napi::Number>().Int64Value();

    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_SEARCH, mode, pattern, capacity);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

// Iterator methods
Napi::Value polyDBM_wrapper::makeIterator(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    iterator = dbm.MakeIterator();
    return Napi::Boolean::New(env, true);
}

Napi::Value polyDBM_wrapper::iteratorFirst(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!iterator) {
        Napi::Promise::Deferred deferred(env);
        deferred.Reject(Napi::TypeError::New(env, "Iterator not created").Value());
        return deferred.Promise();
    }
    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, iterator, dbmAsyncWorker::ITERATOR_FIRST);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::iteratorLast(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!iterator) {
		Napi::Promise::Deferred deferred(env);
		deferred.Reject(Napi::TypeError::New(env, "Iterator not created").Value());
		return deferred.Promise();
	}
    auto* asyncWorker = new dbmAsyncWorker(env, iterator, dbmAsyncWorker::ITERATOR_LAST);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::iteratorJump(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!iterator) {
		Napi::Promise::Deferred deferred(env);
		deferred.Reject(Napi::TypeError::New(env, "Iterator not created").Value());
		return deferred.Promise();
	}
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for iteratorJump").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().Utf8Value();
    auto* asyncWorker = new dbmAsyncWorker(env, iterator, dbmAsyncWorker::ITERATOR_JUMP, key);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::iteratorJumpLower(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!iterator) {
		Napi::Promise::Deferred deferred(env);
		deferred.Reject(Napi::TypeError::New(env, "Iterator not created").Value());
		return deferred.Promise();
	}
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for iteratorJumpLower").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().Utf8Value();
    auto* asyncWorker = new dbmAsyncWorker(env, iterator, dbmAsyncWorker::ITERATOR_JUMP_LOWER, key);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::iteratorJumpUpper(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!iterator) {
		Napi::Promise::Deferred deferred(env);
		deferred.Reject(Napi::TypeError::New(env, "Iterator not created").Value());
		return deferred.Promise();
	}
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for iteratorJumpUpper").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().Utf8Value();
    auto* asyncWorker = new dbmAsyncWorker(env, iterator, dbmAsyncWorker::ITERATOR_JUMP_UPPER, key);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::iteratorNext(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!iterator) {
		Napi::Promise::Deferred deferred(env);
		deferred.Reject(Napi::TypeError::New(env, "Iterator not created").Value());
		return deferred.Promise();
	}
    auto* asyncWorker = new dbmAsyncWorker(env, iterator, dbmAsyncWorker::ITERATOR_NEXT);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::iteratorPrevious(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!iterator) {
		Napi::Promise::Deferred deferred(env);
		deferred.Reject(Napi::TypeError::New(env, "Iterator not created").Value());
		return deferred.Promise();
	}
    auto* asyncWorker = new dbmAsyncWorker(env, iterator, dbmAsyncWorker::ITERATOR_PREVIOUS);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::iteratorGet(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!iterator) {
		Napi::Promise::Deferred deferred(env);
		deferred.Reject(Napi::TypeError::New(env, "Iterator not created").Value());
		return deferred.Promise();
	}
    auto* asyncWorker = new dbmAsyncWorker(env, iterator, dbmAsyncWorker::ITERATOR_GET);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::iteratorSet(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!iterator) {
		Napi::Promise::Deferred deferred(env);
		deferred.Reject(Napi::TypeError::New(env, "Iterator not created").Value());
		return deferred.Promise();
	}
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for iteratorSet").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string value = info[0].As<Napi::String>().Utf8Value();
    auto* asyncWorker = new dbmAsyncWorker(env, iterator, dbmAsyncWorker::ITERATOR_SET, value);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::iteratorRemove(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!iterator) {
		Napi::Promise::Deferred deferred(env);
		deferred.Reject(Napi::TypeError::New(env, "Iterator not created").Value());
		return deferred.Promise();
	}
    auto* asyncWorker = new dbmAsyncWorker(env, iterator, dbmAsyncWorker::ITERATOR_REMOVE);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::freeIterator(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    iterator.reset(nullptr);
    return Napi::Boolean::New(env, true);
}

Napi::Value polyDBM_wrapper::exportKeysAsLines(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for exportKeysAsLines").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string dest_path = info[0].As<Napi::String>().Utf8Value();
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_EXPORT_KEYS_AS_LINES, dest_path);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

// Restoration methods
Napi::Value polyDBM_wrapper::restoreDatabase(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
        Napi::TypeError::New(env, "Invalid arguments for restoreDatabase").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string old_path = info[0].As<Napi::String>().Utf8Value();
    std::string new_path = info[1].As<Napi::String>().Utf8Value();
    std::string class_name = info.Length() > 2 ? info[2].As<Napi::String>().Utf8Value() : "";
    int64_t end_offset = info.Length() > 3 ? info[3].As<Napi::Number>().Int64Value() : -1;
    auto* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_RESTORE_DATABASE, old_path, new_path, class_name, end_offset);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Object polyDBM_wrapper::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function functionList = DefineClass(env, "polyDBM",
    {
        InstanceMethod<&polyDBM_wrapper::set>("set", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::append>("append", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::getSimple>("getSimple", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::shouldBeRebuilt>("shouldBeRebuilt", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::rebuild>("rebuild", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::sync>("sync", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::process>("process", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::close>("close", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::get>("get", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::remove>("remove", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::compareExchange>("compareExchange", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::increment>("increment", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::compareExchangeMulti>("compareExchangeMulti", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::rekey>("rekey", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::processMulti>("processMulti", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::processFirst>("processFirst", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::processEach>("processEach", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::count>("count", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::getFileSize>("getFileSize", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::getFilePath>("getFilePath", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::getTimestamp>("getTimestamp", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::clear>("clear", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::inspect>("inspect", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::isOpen>("isOpen", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::isWritable>("isWritable", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::isHealthy>("isHealthy", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::isOrdered>("isOrdered", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::search>("search", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        
        // NEW: Iterator methods
        InstanceMethod<&polyDBM_wrapper::makeIterator>("makeIterator", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::iteratorFirst>("iteratorFirst", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::iteratorLast>("iteratorLast", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::iteratorJump>("iteratorJump", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::iteratorJumpLower>("iteratorJumpLower", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::iteratorJumpUpper>("iteratorJumpUpper", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::iteratorNext>("iteratorNext", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::iteratorPrevious>("iteratorPrevious", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::iteratorGet>("iteratorGet", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::iteratorSet>("iteratorSet", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::iteratorRemove>("iteratorRemove", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::freeIterator>("freeIterator", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        
        // NEW: Export/Import methods
        InstanceMethod<&polyDBM_wrapper::exportToFlatRecords>("exportToFlatRecords", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::importFromFlatRecords>("importFromFlatRecords", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::exportKeysAsLines>("exportKeysAsLines", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        
        // NEW: Restoration methods
        InstanceMethod<&polyDBM_wrapper::restoreDatabase>("restoreDatabase", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        
        // Static symbols for processor return values
        StaticValue("NOOP", noopSym, static_cast<napi_property_attributes>(napi_enumerable)),
        StaticValue("REMOVE", removeSym, static_cast<napi_property_attributes>(napi_enumerable))
    });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(functionList);
    env.SetInstanceData<Napi::FunctionReference>(constructor);
    
    exports.Set("polyDBM", functionList);
    return exports;
}

void polyDBM_wrapper::Finalize(Napi::Env env)
{
    iterator.reset(nullptr);
    if( dbm.IsOpen() )
    {
        if( dbm.Close() != tkrzw::Status::SUCCESS)
        {
            std::cerr << "DBM finalize: Failed!" << std::endl;
        }
    }
}