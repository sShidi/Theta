#ifndef POLYDBM_WRAPPER_HPP
#define POLYDBM_WRAPPER_HPP

#include <tkrzw_dbm_poly.h>
#include "config_parser.hpp"
#include "dbm_async_worker.hpp"
#include <napi.h>
#include "utils/globals.hpp"
#include <iostream>

class polyDBM_wrapper : public Napi::ObjectWrap<polyDBM_wrapper>
{
    private:
        tkrzw::PolyDBM dbm;
        std::unique_ptr<tkrzw::DBM::Iterator> iterator;
    
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports);
        polyDBM_wrapper(const Napi::CallbackInfo& info);
        
        // Existing methods
        Napi::Value set(const Napi::CallbackInfo& info);
        Napi::Value append(const Napi::CallbackInfo& info);
        Napi::Value getSimple(const Napi::CallbackInfo& info);
        Napi::Value shouldBeRebuilt(const Napi::CallbackInfo& info);
        Napi::Value rebuild(const Napi::CallbackInfo& info);
        Napi::Value sync(const Napi::CallbackInfo& info);
        Napi::Value process(const Napi::CallbackInfo& info);
        Napi::Value close(const Napi::CallbackInfo& info);
        
        // NEW: Additional DBM methods
        Napi::Value get(const Napi::CallbackInfo& info);
        Napi::Value remove(const Napi::CallbackInfo& info);
        Napi::Value compareExchange(const Napi::CallbackInfo& info);
        Napi::Value increment(const Napi::CallbackInfo& info);
        Napi::Value compareExchangeMulti(const Napi::CallbackInfo& info);
        Napi::Value rekey(const Napi::CallbackInfo& info);
        Napi::Value processMulti(const Napi::CallbackInfo& info);
        Napi::Value processFirst(const Napi::CallbackInfo& info);
        Napi::Value processEach(const Napi::CallbackInfo& info);
        Napi::Value count(const Napi::CallbackInfo& info);
        Napi::Value getFileSize(const Napi::CallbackInfo& info);
        Napi::Value getFilePath(const Napi::CallbackInfo& info);
        Napi::Value getTimestamp(const Napi::CallbackInfo& info);
        Napi::Value clear(const Napi::CallbackInfo& info);
        Napi::Value inspect(const Napi::CallbackInfo& info);
        Napi::Value isOpen(const Napi::CallbackInfo& info);
        Napi::Value isWritable(const Napi::CallbackInfo& info);
        Napi::Value isHealthy(const Napi::CallbackInfo& info);
        Napi::Value isOrdered(const Napi::CallbackInfo& info);
        Napi::Value search(const Napi::CallbackInfo& info);
        
        // NEW: Iterator methods
        Napi::Value makeIterator(const Napi::CallbackInfo& info);
        Napi::Value iteratorFirst(const Napi::CallbackInfo& info);
        Napi::Value iteratorLast(const Napi::CallbackInfo& info);
        Napi::Value iteratorJump(const Napi::CallbackInfo& info);
        Napi::Value iteratorJumpLower(const Napi::CallbackInfo& info);
        Napi::Value iteratorJumpUpper(const Napi::CallbackInfo& info);
        Napi::Value iteratorNext(const Napi::CallbackInfo& info);
        Napi::Value iteratorPrevious(const Napi::CallbackInfo& info);
        Napi::Value iteratorGet(const Napi::CallbackInfo& info);
        Napi::Value iteratorSet(const Napi::CallbackInfo& info);
        Napi::Value iteratorRemove(const Napi::CallbackInfo& info);
        Napi::Value freeIterator(const Napi::CallbackInfo& info);
        
        // NEW: Export/Import methods
        Napi::Value exportToFlatRecords(const Napi::CallbackInfo& info);
        Napi::Value importFromFlatRecords(const Napi::CallbackInfo& info);
        Napi::Value exportKeysAsLines(const Napi::CallbackInfo& info);
        
        // NEW: Restoration methods
        Napi::Value restoreDatabase(const Napi::CallbackInfo& info);
        
        void Finalize(Napi::Env env);
};

#endif //POLYDBM_WRAPPER_HPP