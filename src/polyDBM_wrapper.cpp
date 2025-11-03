#include "../include/polyDBM_wrapper.hpp"


polyDBM_wrapper::polyDBM_wrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<polyDBM_wrapper>(info)
{
    Napi::Env env = info.Env();

    std::map<std::string, std::string> optional_tuning_params = parseConfig(env, info[0]);
    std::string dbmPath = info[1].As<Napi::String>();       //The operator std::string() is implicitly invoked

    tkrzw::Status opening_status = dbm.OpenAdvanced(dbmPath, true, tkrzw::File::OPEN_DEFAULT | tkrzw::File::OPEN_SYNC_HARD, optional_tuning_params).OrDie();
    if( opening_status != tkrzw::Status::SUCCESS)
    {
        Napi::TypeError::New(env, opening_status.GetMessage().c_str()).ThrowAsJavaScriptException();
    }
}

Napi::Value polyDBM_wrapper::set(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    std::string key = info[0].As<Napi::String>().ToString().Utf8Value();
    std::string value = info[1].As<Napi::String>().ToString().Utf8Value();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_SET, key, value);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
    
    /*tkrzw::Status set_status = dbm.Set(key,value);
    if( set_status != tkrzw::Status::SUCCESS)
    {
        Napi::TypeError::New(env, set_status.GetMessage().c_str()).ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    else
    { return Napi::Boolean::New(env, true); }*/
}

Napi::Value polyDBM_wrapper::append(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    std::string key = info[0].As<Napi::String>().ToString().Utf8Value();
    std::string value = info[1].As<Napi::String>().ToString().Utf8Value();
    std::string delimeter = info.Length() == 3 ? info[2].As<Napi::String>().ToString().Utf8Value() : "";

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_APPEND, key, value, delimeter);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::getSimple(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    std::string key = info[0].As<Napi::String>().ToString().Utf8Value();
    std::string default_value = info[1].As<Napi::String>().ToString().Utf8Value();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_GET_SIMPLE, key, default_value);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
    
    /*std::string getSimple_result = dbm.GetSimple(key,default_value);

    return Napi::String::New(env, getSimple_result);*/
}

Napi::Value polyDBM_wrapper::shouldBeRebuilt(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_SHOULD_BE_REBUILT);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
    
    /*std::string getSimple_result = dbm.GetSimple(key,default_value);

    return Napi::String::New(env, getSimple_result);*/
}

Napi::Value polyDBM_wrapper::rebuild(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    std::map<std::string, std::string> optional_tuning_params = parseConfig(env, info[0]);

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_REBUILD, optional_tuning_params);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
    
    /*std::string getSimple_result = dbm.GetSimple(key,default_value);

    return Napi::String::New(env, getSimple_result);*/
}

Napi::Value polyDBM_wrapper::sync(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    bool sync_hard = info[0].As<Napi::Boolean>();

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_SYNC, sync_hard);
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::process(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    std::string key = info[0].As<Napi::String>();
    Napi::Function jsprocessor = info[1].As<Napi::Function>();
    bool writable = info[2].As<Napi::Boolean>();

    TSFN tsfn = TSFN::New(
    env,
    jsprocessor,         // JS function to call
    "processor_jsfunc_wrapper tsfn",
    0,                  // Unlimited queue
    1                  // Initial thread count
    );

    dbmAsyncWorker* asyncWorker = new dbmAsyncWorker(env, dbm, dbmAsyncWorker::DBM_PROCESS, key, writable, tsfn );
    asyncWorker->Queue();
    return asyncWorker->deferred_promise.Promise();
}

Napi::Value polyDBM_wrapper::close(const Napi::CallbackInfo& info)
{
    std::cout << "CLOSE DBM" << std::endl;
    Napi::Env env = info.Env();
    tkrzw::Status close_status = dbm.Close();
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

Napi::Object polyDBM_wrapper::Init(Napi::Env env, Napi::Object exports)
{
    // Initialization
    ::noopSym = Napi::String::New(env, "___TKRZW_NOOP___");
    ::removeSym = Napi::String::New(env, "___TKRZW_REMOVE___");

    // Define all instance methods and static values
    Napi::Function functionList = DefineClass(env, "polyDBM",
    {
        // Existing methods
        InstanceMethod<&polyDBM_wrapper::set>("set", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::append>("append", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::getSimple>("getSimple", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::shouldBeRebuilt>("shouldBeRebuilt", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::rebuild>("rebuild", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::sync>("sync", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::process>("process", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&polyDBM_wrapper::close>("close", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        
        // NEW: Additional DBM methods
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

//-----------------JS-Requirements-----------------//