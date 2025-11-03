#ifndef POLYINDEX_WRAPPER_HPP
#define POLYINDEX_WRAPPER_HPP

#include <tkrzw_index.h>
#include "config_parser.hpp"
#include "dbm_async_worker.hpp"

#include <memory>       //For std::unique_ptr
#include <napi.h>

class polyIndex_wrapper : public Napi::ObjectWrap<polyIndex_wrapper>
{
    private:
        tkrzw::PolyIndex index;
        std::unique_ptr<tkrzw::PolyIndex::Iterator> jump_iter;

    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports);          //required by Node!
        polyIndex_wrapper(const Napi::CallbackInfo& info);
        Napi::Value add(const Napi::CallbackInfo& info);                        //async
        Napi::Value getValues(const Napi::CallbackInfo& info);                  //async
        Napi::Value check(const Napi::CallbackInfo& info);                      //async
        Napi::Value remove(const Napi::CallbackInfo& info);                     //async
        Napi::Value shouldBeRebuilt(const Napi::CallbackInfo& info);            //async
        Napi::Value rebuild(const Napi::CallbackInfo& info);                    //async
        Napi::Value sync(const Napi::CallbackInfo& info);                       //async
        Napi::Value makeJumpIterator(const Napi::CallbackInfo& info);           //async
        Napi::Value getIteratorValue(const Napi::CallbackInfo& info);           //async
        Napi::Value continueIteration(const Napi::CallbackInfo& info);          //async
        Napi::Value freeIterator(const Napi::CallbackInfo& info);
        Napi::Value close(const Napi::CallbackInfo& info);
        void Finalize(Napi::Env env);
};


#endif //POLYINDEX_WRAPPER_HPP