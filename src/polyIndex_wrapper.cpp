#include "../include/polyIndex_wrapper.hpp"

polyIndex_wrapper::polyIndex_wrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<polyIndex_wrapper>(info)
{
    Napi::Env env = info.Env();
    
    std::map<std::string, std::string> optional_tuning_params = parseConfig(env, info[0]);
    std::string indexPath = info[1].As<Napi::String>();         //The operator std::string() is implicitly invoked

    tkrzw::Status opening_status = index.Open(indexPath, true, tkrzw::File::OPEN_DEFAULT | tkrzw::File::OPEN_SYNC_HARD, optional_tuning_params).OrDie();
    if( opening_status != tkrzw::Status::SUCCESS)
    {
        Napi::TypeError::New(env, opening_status.GetMessage().c_str()).ThrowAsJavaScriptException();
    }
}

Napi::Value polyIndex_wrapper::add(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString())
    {
        Napi::TypeError::New(env, "Add failed").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string key = info[0].As<Napi::String>().ToString().Utf8Value();
    std::string value = info[1].As<Napi::String>().ToString().Utf8Value();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, index, dbmAsyncWorker::INDEX_ADD, key, value);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyIndex_wrapper::getValues(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    std::string key = info[0].As<Napi::String>().ToString().Utf8Value();
    size_t max_number_of_records = info[1].As<Napi::Number>().Int64Value();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, index, dbmAsyncWorker::INDEX_GET_VALUES, key, max_number_of_records);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyIndex_wrapper::check(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    std::string key = info[0].As<Napi::String>().ToString().Utf8Value();
    std::string value = info[1].As<Napi::String>().ToString().Utf8Value();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, index, dbmAsyncWorker::INDEX_CHECK, key, value);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyIndex_wrapper::remove(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    std::string key = info[0].As<Napi::String>().ToString().Utf8Value();
    std::string value = info[1].As<Napi::String>().ToString().Utf8Value();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, index, dbmAsyncWorker::INDEX_REMOVE, key, value);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyIndex_wrapper::shouldBeRebuilt(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, index, dbmAsyncWorker::INDEX_SHOULD_BE_REBUILT);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyIndex_wrapper::rebuild(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, index, dbmAsyncWorker::INDEX_REBUILD);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyIndex_wrapper::sync(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    bool sync_hard = info[0].As<Napi::Boolean>();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, index, dbmAsyncWorker::INDEX_SYNC, sync_hard);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyIndex_wrapper::makeJumpIterator(const Napi::CallbackInfo& info)
{
    /**
     * `jump_iter` is a member of `polyIndex_wrapper` class so it wouldn't go outta scope.
     * `std::unique_ptr::reset()` should be called on it once it's no longer needed!
     */
    /**
     * According to copilot, `tkrzw::PolyDBM::MakeIterator` call is light enough to happen in the main thread;
     * It just initializes the iterator (no I/O)!
     * But `tkrzw::PolyIndex::Iterator::Jump` call needs to do some processing to find the starting position of the iterator,
     * So it's moved to `Execute` method to be async
     */
    /**
     * `std::unique_ptr` is movable, not copyable! Therefor:
     * When putting a std::unique_ptr into a std::any, you are moving it into the std::any, which means the original pointer is now empty.
     * If you later do std::any_cast<std::unique_ptr<PolyIndex::Iterator>>(...), you will again move it out of the std::any, after which the std::any no longer holds it (and you cannot use it again).
     * This breaks the intended ownership and lifecycle!
     * Instead a pointer to the pointer to the internal managed object will be send to `dbmAsyncWorker` as pointers are copyable!
     */
    /**
     * `Tkrzw::PolyIndex` class uses babyDBM and TreeDBM which both are based on b+-tree!
     * Therefor it is always an ordered DBM and there's no `Tkrzw::PolyIndex::IsOrdered()` method to check!
     * Refer to:
     * 1. https://dbmx.net/tkrzw/api/classtkrzw_1_1PolyIndex.html#aa4842f0c37234b87d49642ab7a5a688c
     * 2. https://dbmx.net/tkrzw/#index_overview
     */
    Napi::Env env = info.Env();
    jump_iter = index.MakeIterator();
    std::string partialKey = info[0].As<Napi::String>();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, index, dbmAsyncWorker::INDEX_MAKE_JUMP_ITERATOR, partialKey, jump_iter.get());
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyIndex_wrapper::getIteratorValue(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, index, dbmAsyncWorker::INDEX_GET_ITERATOR_VALUE, jump_iter.get());
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyIndex_wrapper::continueIteration(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, index, dbmAsyncWorker::INDEX_CONTINUE_ITERATION, jump_iter.get());
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyIndex_wrapper::freeIterator(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    jump_iter.reset(nullptr);       //Same as `reset()` with no argument. Calls deleter of the current internal pointer if not `nullptr` already.
    return Napi::Boolean::New(env, true);
}

Napi::Value polyIndex_wrapper::close(const Napi::CallbackInfo& info)
{
    std::cout << "CLOSE INDEX" << std::endl;
    Napi::Env env = info.Env();
    tkrzw::Status close_status = index.Close();
    if( close_status != tkrzw::Status::SUCCESS)
    {
        Napi::TypeError::New(env, close_status.GetMessage().c_str()).ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    else
    { return Napi::Boolean::New(env, true); }
}

//====================================================================================================================

//-----------------JS-Requirements-----------------//
Napi::Object polyIndex_wrapper::Init(Napi::Env env, Napi::Object exports)
{
    // This method is used to hook the accessor and method callbacks
    Napi::Function functionList = DefineClass(env, "polyIndex",
    {
        InstanceMethod<&polyIndex_wrapper::add>("add", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::getValues>("getValues", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::check>("check", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::remove>("remove", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::shouldBeRebuilt>("shouldBeRebuilt", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::rebuild>("rebuild", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::sync>("sync", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::makeJumpIterator>("makeJumpIterator", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::getIteratorValue>("getIteratorValue", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::continueIteration>("continueIteration", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::freeIterator>("freeIterator", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyIndex_wrapper::close>("close", static_cast<napi_property_attributes>(napi_writable | napi_configurable))
    });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();

    *constructor = Napi::Persistent(functionList);
    exports.Set("polyIndex", functionList);
    env.SetInstanceData<Napi::FunctionReference>(constructor);

    return exports;
}

void polyIndex_wrapper::Finalize(Napi::Env env)
{
    jump_iter.reset(nullptr);       //Same as `reset()` with no argument. Calls deleter of the current internal pointer if not `nullptr` already.
    if( index.IsOpen() )
    {
        if( index.Close() != tkrzw::Status::SUCCESS)
        {
            std::cerr << "Index finalize: Failed!" << std::endl;
        }
    }
}
//-----------------JS-Requirements-----------------//