# Tkrzw Node.js Bindings - Complete Feature Implementation
## Summary
This implementation adds 40+ new methods to make the Tkrzw C++ library fully accessible from Node.js, covering all major functionality including atomic operations, batch processing, iterators, search, export/import, and database maintenance.

## New Features
### 1. Enhanced Get/Set Operations (3 methods)   
- get(key) - Get with proper error handling (vs. getSimple with default)   
- remove(key) - Remove a record   
- clear() - Clear all records from database   
### 2. Atomic Operations (4 methods)   
- compareExchange(key, expected, desired) - Atomic compare-and-swap   
- increment(key, increment, initial) - Atomic counter increment/decrement   
- compareExchangeMulti(expected, desired) - Multi-record atomic CAS   
- rekey(oldKey, newKey, overwrite, copying) - Atomic key rename/copy   
#### Use Cases:

- Distributed locks   
- Atomic counters (page views, likes, etc.)   
- Transaction-like operations   
- Versioned data updates   
### 3. Batch Processing (3 methods)
- processMulti(keys, processor, writable) - Process multiple specific keys   
- processFirst(processor, writable) - Process first record   
- processEach(processor, writable) - Process all records   
#### Use Cases:

- Bulk data transformations   
- Database migrations   
- Analytics over all records   
- Cleanup operations   
### 4. Iterator Operations (11 methods)
- makeIterator() - Create iterator   
- iteratorFirst() - Jump to first record   
- iteratorLast() - Jump to last record   
- iteratorJump(key) - Jump to specific key   
- iteratorJumpLower(key, inclusive) - Jump to lower bound   
- iteratorJumpUpper(key, inclusive) - Jump to upper bound   
- iteratorNext() - Move to next record   
- iteratorPrevious() - Move to previous record   
- iteratorGet() - Get current key-value pair   
- iteratorSet(value) - Update value at current position   
- iteratorRemove() - Remove record at current position   
- freeIterator() - Free iterator resources   
#### Use Cases:

- Range queries   
- Ordered data traversal   
- Pagination   
- Cursor-based iteration   
### 5. Search Operations (1 method)   
- search(mode, pattern, capacity) - Search keys by pattern   
#### Search Modes:

- 'begin' - Keys starting with pattern (prefix search)   
- 'contain' - Keys containing pattern (substring search)   
- 'end' - Keys ending with pattern (suffix search)   
- 'regex' - Keys matching regex pattern   
- 'edit' - Keys within edit distance   
- 'editbin' - Keys within binary edit distance   
#### Use Cases:

- Autocomplete   
- Full-text search (with proper indexing)   
- Pattern matching   
- Fuzzy search   
### 6. Database Information (8 methods)   
- count() - Get total record count   
- getFileSize() - Get database file size   
- getFilePath() - Get database file path   
- getTimestamp() - Get last modification time   
- inspect() - Get detailed database metadata   
- isOpen() - Check if database is open   
- isWritable() - Check if database is writable   
- isHealthy() - Check database health status   
- isOrdered() - Check if database supports ordering   
#### Use Cases:

- Monitoring and metrics   
- Health checks   
- Debugging   
- Database management tools   
### 7. Export/Import Operations (3 methods)
- exportToFlatRecords(destPath) - Export database to flat file   
- importFromFlatRecords(srcPath) - Import database from flat file   
- exportKeysAsLines(destPath) - Export all keys as text lines   
#### Use Cases:

- Database backups   
- Data migration between systems   
- Debugging and inspection   
- Data archival   
### 8. Database Restoration (1 method)
- restoreDatabase(oldPath, newPath, className, endOffset) - Restore from update logs   
#### Use Cases:

- Disaster recovery   
- Point-in-time recovery   
- Database cloning   
- Testing and development   
- Implementation Architecture   
- Async Worker Pattern   


## Technical Notes

#### All I/O operations use the dbmAsyncWorker class which:

- Runs operations in libuv thread pool (non-blocking)   
- Returns JavaScript Promises   
- Properly handles errors and results   
- Supports type-safe parameter passing via std::any   
- Thread-Safe Function Callbacks   

#### Record processors use Napi::TypedThreadSafeFunction to:

- Call JavaScript from C++ worker threads   
- Support both sync and async JavaScript callbacks   
- Handle promises returned from callbacks   
- Maintain proper memory management   
- Iterator Management   

#### Iterators use std::unique_ptr for:

- Automatic memory management   
- Safe resource cleanup on database finalization   
- Prevention of memory leaks   


## File Structure
```sh
include/
├── polyDBM_wrapper.hpp          # Extended with 40+ new methods
├── polyIndex_wrapper.hpp        # (unchanged from original)
├── dbm_async_worker.hpp         # Extended with new operation types
├── config_parser.hpp            # (unchanged)
└── utils/
    ├── globals.hpp              # (unchanged)
    ├── tsfn_types.hpp           # (unchanged)
    └── processor_jsfunc_wrapper.hpp  # (unchanged)

src/
├── polyDBM_wrapper.cpp          # Implementation of all new methods
├── polyIndex_wrapper.cpp        # (unchanged)
├── dbm_async_worker.cpp         # Extended Execute() and OnOK()
├── config_parser.cpp            # (unchanged)
├── tkrzw_main_export.cpp        # (unchanged)
└── utils/
    ├── globals.cpp              # (unchanged)
    ├── CallJS.cpp               # (unchanged)
    └── processor_jsfunc_wrapper.cpp  # (unchanged)
```
## Usage Examples
```js
// Atomic Counter
const count = await db.increment('page_views', 1, 0);
console.log('Views:', count);

// Compare-and-Swap Lock
try {
    await db.compareExchange('lock', 'unlocked', 'locked');
    // Critical section
    await db.compareExchange('lock', 'locked', 'unlocked');
} catch {
    console.log('Lock busy');
}

// Batch Processing
await db.processEach((exists, key, value) => {
    if (value.includes('outdated')) {
        return polyDBM.REMOVE;
    }
    return polyDBM.NOOP;
}, true);

// Iterator Pattern
if (db.isOrdered()) {
    db.makeIterator();
    await db.iteratorFirst();
    
    while (true) {
        try {
            const { key, value } = await db.iteratorGet();
            console.log(key, value);
            await db.iteratorNext();
        } catch {
            break;
        }
    }
    
    db.freeIterator();
}

// Search Operations
// Find all user keys
const keys = await db.search('begin', 'user:', 100);
// Fuzzy search
const similar = await db.search('edit', 'alice', 10);

// Export/Import
// Backup
await db.exportToFlatRecords('./backup.tkf');
// Restore
await db.importFromFlatRecords('./backup.tkf');
```
## Performance Considerations
- All I/O is async - No blocking of Node.js event loop   
- Batch operations - More efficient than individual operations   
- Iterator reuse - Create once, iterate many times   
- Atomic operations - Lock-free concurrency   
- Search limits - Use capacity parameter to limit results   

### Error Handling
All async methods return Promises that:

- Resolve on success (may return undefined, value, or array)   
- Reject with Error object containing Tkrzw status code and message   
```js
try {
    await db.get('nonexistent');
} catch (err) {
    console.error(err.message); // "[STATUS_NOT_FOUND_ERROR]: key not found"
}
```
### Compatibility
- Node-API Version: 9+ (for thread-safe functions)   
- Node.js Version: 18.17.0+, 20.3.0+, 21.0.0+   
- Tkrzw Version: 1.0.32   
- OS: Linux (primary support)   
- Testing Checklist   
 - Basic CRUD operations   
 - Atomic operations (CAS, increment)   
 - Batch processing (multi, first, each)   
 - Iterator operations (ordered DBMs)   
 - Search operations (all modes)   
 - Database information methods   
 - Export/Import functionality   
 - Error handling and edge cases   
 - Memory leak prevention   
-  Thread safety