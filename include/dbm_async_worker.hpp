#ifndef DBM_ASYNC_WORKER_HPP
#define DBM_ASYNC_WORKER_HPP

#include <napi.h>
#include <tkrzw_dbm_poly.h>
#include <tkrzw_index.h>
#include <any>
#include <vector>
#include <map>
#include <memory>

// Async worker for DBM and Index operations
class dbmAsyncWorker : public Napi::AsyncWorker {
public:
    // Operation types supported
    enum OPERATION_TYPE {
        // DBM operations
        DBM_SET,
        DBM_APPEND,
        DBM_GET_SIMPLE,
        DBM_SHOULD_BE_REBUILT,
        DBM_REBUILD,
        DBM_SYNC,
        DBM_PROCESS,

        // Iterator operations
        ITERATOR_FIRST,
        ITERATOR_LAST,
        ITERATOR_JUMP,
        ITERATOR_JUMP_LOWER,
        ITERATOR_JUMP_UPPER,
        ITERATOR_NEXT,
        ITERATOR_PREVIOUS,
        ITERATOR_GET,
        ITERATOR_SET,
        ITERATOR_REMOVE,

        // Index operations (already present in your cpp)
        INDEX_ADD,
        INDEX_GET_VALUES,
        INDEX_CHECK,
        INDEX_REMOVE,
        INDEX_SHOULD_BE_REBUILT,
        INDEX_REBUILD,
        INDEX_SYNC,
        INDEX_MAKE_JUMP_ITERATOR,
        INDEX_GET_ITERATOR_VALUE,
        INDEX_CONTINUE_ITERATION
    };

    // Constructors
    template <typename... argTypes>
    dbmAsyncWorker(const Napi::Env& env,
                   tkrzw::PolyDBM& dbmReference,
                   OPERATION_TYPE operation,
                   argTypes... paramPack)
        : Napi::AsyncWorker(env),
          dbmReference(&dbmReference),
          operation(operation),
          deferred_promise{Env()} {
        (params.emplace_back(std::any(paramPack)), ...);
    }

    template <typename... argTypes>
    dbmAsyncWorker(const Napi::Env& env,
                   std::unique_ptr<tkrzw::PolyDBM::Iterator>& iteratorReference,
                   OPERATION_TYPE operation,
                   argTypes... paramPack)
        : Napi::AsyncWorker(env),
          iteratorReference(&iteratorReference),
          operation(operation),
          deferred_promise{Env()} {
        (params.emplace_back(std::any(paramPack)), ...);
    }

    template <typename... argTypes>
    dbmAsyncWorker(const Napi::Env& env,
                   tkrzw::PolyIndex& indexReference,
                   OPERATION_TYPE operation,
                   argTypes... paramPack)
        : Napi::AsyncWorker(env),
          indexReference(&indexReference),
          operation(operation),
          deferred_promise{Env()} {
        (params.emplace_back(std::any(paramPack)), ...);
    }

    // Core async methods
    void Execute() override;
    void OnOK() override;
    void OnError(const Napi::Error& err) override;

    // Promise handle
    Napi::Promise::Deferred deferred_promise;

private:
    // References to DBM, Iterator, or Index
    tkrzw::PolyDBM* dbmReference = nullptr;
    std::unique_ptr<tkrzw::PolyDBM::Iterator>* iteratorReference = nullptr;
    tkrzw::PolyIndex* indexReference = nullptr;

    OPERATION_TYPE operation;
    std::vector<std::any> params;
    std::any any_result;
};

#endif // DBM_ASYNC_WORKER_HPP
