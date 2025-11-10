# Tkrzw Node.js Bindings

High-performance Node.js bindings for [Tkrzw](https://dbmx.net/tkrzw/), a modern database management library written in C++. This package provides full async support for both PolyDBM (key-value database) and PolyIndex (secondary indexing) with comprehensive features including atomic operations, batch processing, iterators, and advanced search capabilities.

[![License: ISC](https://img.shields.io/badge/License-ISC-blue.svg)](https://opensource.org/licenses/ISC)

## Features

- **Full Async/Await Support** - All I/O operations return Promises
- **High Performance** - Direct C++ bindings with minimal overhead
- **Thread-Safe** - Operations execute in libuv thread pool
- **Atomic Operations** - CAS, increment, multi-record transactions
- **Batch Processing** - Efficient bulk operations with custom processors
- **Iterator Support** - Forward/backward traversal with range queries
- **Advanced Search** - Prefix, suffix, contain, regex, fuzzy search
- **Export/Import** - Database backup and migration tools
- **Update Logs** - Point-in-time recovery and replication support
- **TypeScript Ready** - Full type definitions included

## Installation

```bash
npm install tkrzw-node
```

### System Requirements

- **OS**: Linux (primary support)
- **Node.js**: 18.17.0+, 20.3.0+, 21.0.0+
- **Dependencies**:
    - cmake
    - ninja-build
    - build-essential
    - liblz4-dev

```bash
# Ubuntu/Debian
sudo apt-get install cmake ninja-build build-essential liblz4-dev

# RHEL/CentOS/Fedora
sudo dnf install cmake ninja-build gcc-c++ lz4-devel
```

## Quick Start

```javascript
import { polyDBM, polyIndex } from 'tkrzw-node';
import fs from 'fs';

// Load configuration
const config = JSON.parse(fs.readFileSync('./tkrzw_config.json', 'utf8'));

// Open database
const db = new polyDBM(config, './mydb.tkh');

// Basic operations
await db.set('user:1', 'Alice');
const value = await db.get('user:1');
console.log(value); // 'Alice'

// Clean up
db.close();
```

## API Reference

### polyDBM Class

#### Constructor

```javascript
new polyDBM(config, dbPath)
```

- `config`: Object or JSON string with database tuning parameters
- `dbPath`: Path to database file

#### Basic Operations

##### `set(key, value)` → `Promise<boolean>`
Store a record.

```javascript
await db.set('user:1', 'Alice');
```

##### `get(key, defaultValue?)` → `Promise<string>`
Retrieve a record. Returns empty string or default if not found.

```javascript
const value = await db.get('user:1');
const withDefault = await db.get('unknown', 'N/A');
```

##### `append(key, value, delimiter?)` → `Promise<boolean>`
Append to existing value.

```javascript
await db.set('log', 'entry1');
await db.append('log', 'entry2', '\n');
// Result: "entry1\nentry2"
```

##### `remove(key)` → `Promise<boolean>`
Delete a record.

```javascript
await db.remove('user:1');
```

##### `clear()` → `Promise<boolean>`
Delete all records.

```javascript
await db.clear();
```

#### Atomic Operations

##### `increment(key, increment, initial?)` → `Promise<number>`
Atomically increment/decrement a counter.

```javascript
// Initialize to 0, increment by 1
const views = await db.increment('page_views', 1, 0);

// Decrement
const count = await db.increment('inventory', -5, 100);
```

##### `compareExchange(key, expected, desired)` → `Promise<boolean>`
Atomic compare-and-swap operation.

```javascript
// Acquire lock
await db.set('lock', 'unlocked');
await db.compareExchange('lock', 'unlocked', 'locked');

// Release lock
await db.compareExchange('lock', 'locked', 'unlocked');
```

##### `compareExchangeMulti(expected, desired)` → `Promise<boolean>`
Multi-record atomic transaction.

```javascript
await db.compareExchangeMulti(
  [
    { key: 'account:1', value: '100' },
    { key: 'account:2', value: '50' }
  ],
  [
    { key: 'account:1', value: '50' },
    { key: 'account:2', value: '100' }
  ]
);
```

##### `rekey(oldKey, newKey, overwrite?, copying?)` → `Promise<boolean>`
Atomically rename or copy a record.

```javascript
// Move record
await db.rekey('old_key', 'new_key', false, false);

// Copy record
await db.rekey('source', 'backup', true, true);
```

#### Record Processing

##### `process(key, processor, writable)` → `Promise<boolean>`
Process a single record with custom function.

```javascript
await db.process('counter', (exists, key, value) => {
  if (exists) {
    const num = parseInt(value) || 0;
    return (num + 1).toString();
  }
  return '1'; // Initialize if not exists
}, true);
```

Processor return values:
- `polyDBM.NOOP` - Keep record unchanged
- `polyDBM.REMOVE` - Delete the record
- `string` - Set new value

##### `processMulti(keys, processor, writable)` → `Promise<boolean>`
Process multiple specific records.

```javascript
await db.processMulti(
  ['user:1', 'user:2', 'user:3'],
  (exists, key, value) => {
    if (exists) {
      return value.toUpperCase();
    }
    return polyDBM.NOOP;
  },
  true
);
```

##### `processFirst(processor, writable)` → `Promise<boolean>`
Process the first record.

```javascript
await db.processFirst((exists, key, value) => {
  console.log('First:', key, value);
  return polyDBM.NOOP;
}, false);
```

##### `processEach(processor, writable)` → `Promise<boolean>`
Process all records in database.

```javascript
// Extract all records
const records = [];
await db.processEach((exists, key, value) => {
  if (exists) {
    records.push({ key, value });
  }
  return polyDBM.NOOP;
}, false);

// Delete old records
await db.processEach((exists, key, value) => {
  if (value.includes('outdated')) {
    return polyDBM.REMOVE;
  }
  return polyDBM.NOOP;
}, true);
```

#### Iterator Operations

Iterators are only available for ordered databases (TreeDBM, SkipDBM).

##### `makeIterator()` → `boolean`
Create an iterator.

```javascript
if (db.isOrdered()) {
  db.makeIterator();
}
```

##### `iteratorFirst()` → `Promise<boolean>`
Jump to first record.

```javascript
await db.iteratorFirst();
```

##### `iteratorLast()` → `Promise<boolean>`
Jump to last record.

```javascript
await db.iteratorLast();
```

##### `iteratorJump(key)` → `Promise<boolean>`
Jump to specific key or nearest match.

```javascript
await db.iteratorJump('user:100');
```

##### `iteratorJumpLower(key)` → `Promise<boolean>`
Jump to largest key less than target.

```javascript
await db.iteratorJumpLower('user:100');
```

##### `iteratorJumpUpper(key)` → `Promise<boolean>`
Jump to smallest key greater than target.

```javascript
await db.iteratorJumpUpper('user:100');
```

##### `iteratorNext()` → `Promise<boolean>`
Move to next record.

```javascript
await db.iteratorNext();
```

##### `iteratorPrevious()` → `Promise<boolean>`
Move to previous record.

```javascript
await db.iteratorPrevious();
```

##### `iteratorGet()` → `Promise<{key: string, value: string}>`
Get current key-value pair.

```javascript
const { key, value } = await db.iteratorGet();
```

##### `iteratorSet(value)` → `Promise<boolean>`
Update value at current position.

```javascript
await db.iteratorSet('new_value');
```

##### `iteratorRemove()` → `Promise<boolean>`
Delete record at current position.

```javascript
await db.iteratorRemove();
```

##### `freeIterator()` → `boolean`
Release iterator resources.

```javascript
db.freeIterator();
```

**Complete Iterator Example:**

```javascript
if (db.isOrdered()) {
  db.makeIterator();
  await db.iteratorFirst();
  
  while (true) {
    try {
      const { key, value } = await db.iteratorGet();
      console.log(key, value);
      await db.iteratorNext();
    } catch (err) {
      break; // End of iteration
    }
  }
  
  db.freeIterator();
}
```

#### Search Operations

##### `search(mode, pattern, capacity)` → `Promise<string[]>`
Search for keys matching pattern.

**Search Modes:**

- `'begin'` - Prefix search (keys starting with pattern)
- `'contain'` - Substring search (keys containing pattern)
- `'end'` - Suffix search (keys ending with pattern)
- `'regex'` - Regular expression matching
- `'edit'` - Edit distance (fuzzy matching)

```javascript
// Find all user keys
const userKeys = await db.search('begin', 'user:', 100);

// Find keys containing 'admin'
const adminKeys = await db.search('contain', 'admin', 50);

// Find keys ending with '.jpg'
const images = await db.search('end', '.jpg', 1000);

// Regex search
const phoneNumbers = await db.search('regex', '^\\+1\\d{10}$', 100);

// Fuzzy search (edit distance ≤ 2)
const similar = await db.search('edit', 'alice', 10);
```

#### Database Information

##### `count()` → `Promise<number>`
Get total number of records.

```javascript
const total = await db.count();
```

##### `getFileSize()` → `Promise<number>`
Get database file size in bytes.

```javascript
const bytes = await db.getFileSize();
console.log(`Size: ${(bytes / 1024 / 1024).toFixed(2)} MB`);
```

##### `getFilePath()` → `Promise<string>`
Get database file path.

```javascript
const path = await db.getFilePath();
```

##### `getTimestamp()` → `Promise<number>`
Get last modification timestamp.

```javascript
const timestamp = await db.getTimestamp();
const date = new Date(timestamp * 1000);
```

##### `inspect()` → `Promise<object>`
Get detailed database metadata.

```javascript
const info = await db.inspect();
console.log(info);
// {
//   class: 'HashDBM',
//   num_records: '12345',
//   file_size: '1048576',
//   ...
// }
```

##### `isOpen()` → `boolean`
Check if database is open.

```javascript
if (db.isOpen()) {
  // ...
}
```

##### `isWritable()` → `boolean`
Check if database is writable.

```javascript
if (db.isWritable()) {
  await db.set('key', 'value');
}
```

##### `isHealthy()` → `boolean`
Check database health status.

```javascript
if (!db.isHealthy()) {
  console.error('Database corruption detected!');
}
```

##### `isOrdered()` → `boolean`
Check if database supports ordering.

```javascript
if (db.isOrdered()) {
  // Can use iterators and range queries
}
```

#### Maintenance Operations

##### `sync(hard?)` → `Promise<boolean>`
Flush changes to disk.

```javascript
// Soft sync (metadata only)
await db.sync(false);

// Hard sync (full fsync)
await db.sync(true);
```

##### `shouldBeRebuilt()` → `Promise<boolean>`
Check if database should be rebuilt for optimization.

```javascript
if (await db.shouldBeRebuilt()) {
  await db.rebuild();
}
```

##### `rebuild(params?)` → `Promise<boolean>`
Rebuild database for optimization.

```javascript
const rebuildConfig = {
  offset_width: '4',
  align_pow: '7',
  num_buckets: '2000000'
};
await db.rebuild(rebuildConfig);
```

##### `close()` → `boolean`
Close database.

```javascript
db.close();
```

#### Export/Import Operations

##### `exportKeysAsLines(destPath)` → `Promise<boolean>`
Export all keys to text file (one per line).

```javascript
await db.exportKeysAsLines('./keys.txt');
```

##### `restoreDatabase(oldPath, newPath, className?, endOffset?)` → `Promise<boolean>`
Restore database from update logs.

```javascript
// Restore from logs
await polyDBM.restoreDatabase(
  './db/mydb.tkh',
  './db/mydb_restored.tkh',
  'HashDBM',
  -1 // Full restore
);
```

### polyIndex Class

Secondary index for efficient value-to-key lookups.

#### Constructor

```javascript
new polyIndex(config, indexPath)
```

#### Methods

##### `add(key, value)` → `Promise<boolean>`
Add index entry.

```javascript
const idx = new polyIndex(config, './tags.tkt');
await idx.add('tags', 'javascript');
await idx.add('tags', 'nodejs');
```

##### `getValues(key, max)` → `Promise<string[]>`
Get all values for a key.

```javascript
const tags = await idx.getValues('tags', 100);
// ['javascript', 'nodejs', ...]
```

##### `check(key, value)` → `Promise<boolean>`
Check if entry exists.

```javascript
const exists = await idx.check('tags', 'python');
```

##### `remove(key, value)` → `Promise<boolean>`
Remove index entry.

```javascript
await idx.remove('tags', 'outdated');
```

##### `sync(hard)` → `Promise<boolean>`
Flush index to disk.

```javascript
await idx.sync(true);
```

##### `shouldBeRebuilt()` → `Promise<boolean>`
Check if index needs rebuilding.

```javascript
if (await idx.shouldBeRebuilt()) {
  await idx.rebuild();
}
```

##### `rebuild()` → `Promise<boolean>`
Rebuild index.

```javascript
await idx.rebuild();
```

#### Iterator Methods

##### `makeJumpIterator(prefix)` → `Promise<boolean>`
Create iterator starting at prefix.

```javascript
await idx.makeJumpIterator('user:');
```

##### `getIteratorValue()` → `Promise<{key: string, value: string}>`
Get current iterator position.

```javascript
const { key, value } = await idx.getIteratorValue();
```

##### `continueIteration()` → `Promise<boolean>`
Move iterator to next entry.

```javascript
await idx.continueIteration();
```

##### `freeIterator()` → `boolean`
Release iterator.

```javascript
idx.freeIterator();
```

##### `close()` → `boolean`
Close index.

```javascript
idx.close();
```

## Configuration

### Database Configuration (tkrzw_config.json)

```json
{
  "dbm": "HashDBM",
  "offset_width": "4",
  "align_pow": "7",
  "num_buckets": "2000000",
  "file": "MemoryMapParallelFile",
  "min_read_size": "128",
  "cache_buckets": "true",
  "update_mode": "UPDATE_APPENDING",
  "restore_mode": "RESTORE_SYNC:RESTORE_NO_SHORTCUTS:RESTORE_WITH_HARDSYNC",
  "ulog_prefix": "./db/dbmLog",
  "ulog_max_file_size": "128Mi",
  "ulog_server_id": "555",
  "ulog_dbm_index": "777",
  "record_comp_mode": "RECORD_COMP_LZ4"
}
```

### Index Configuration (tkrzw_index_config.json)

```json
{
  "dbm": "TreeDBM",
  "offset_width": "4",
  "align_pow": "7",
  "num_buckets": "2000000",
  "file": "MemoryMapParallelFile",
  "min_read_size": "128",
  "cache_buckets": "true",
  "update_mode": "UPDATE_IN_PLACE",
  "restore_mode": "RESTORE_SYNC:RESTORE_NO_SHORTCUTS:RESTORE_WITH_HARDSYNC",
  "ulog_prefix": "./db/indexLog",
  "ulog_max_file_size": "128Mi",
  "ulog_server_id": "555",
  "ulog_dbm_index": "777",
  "page_update_mode": "PAGE_UPDATE_WRITE",
  "key_comparator": "LexicalKeyComparator",
  "max_branches": "128"
}
```

### DBM Types

- **HashDBM** - Hash table (fastest, unordered)
- **TreeDBM** - B+ tree (ordered, range queries)
- **SkipDBM** - Skip list (ordered, concurrent)
- **TinyDBM** - Small databases
- **BabyDBM** - Simple B+ tree
- **CacheDBM** - LRU cache
- **StdHashDBM** - std::unordered_map wrapper
- **StdTreeDBM** - std::map wrapper

## Usage Patterns

### Atomic Counter

```javascript
async function incrementPageViews(page) {
  const count = await db.increment(`views:${page}`, 1, 0);
  console.log(`Page ${page} has ${count} views`);
  return count;
}
```

### Distributed Lock

```javascript
async function withLock(lockKey, timeout, callback) {
  const deadline = Date.now() + timeout;
  
  while (Date.now() < deadline) {
    try {
      await db.compareExchange(lockKey, 'unlocked', 'locked');
      try {
        return await callback();
      } finally {
        await db.compareExchange(lockKey, 'locked', 'unlocked');
      }
    } catch (err) {
      await new Promise(resolve => setTimeout(resolve, 100));
    }
  }
  
  throw new Error('Lock timeout');
}
```

### Batch Processing

```javascript
async function updateAllUsers(transform) {
  await db.processEach((exists, key, value) => {
    if (exists && key.startsWith('user:')) {
      return transform(value);
    }
    return polyDBM.NOOP;
  }, true);
}
```

### Range Query

```javascript
async function getUsersInRange(startId, endId) {
  if (!db.isOrdered()) {
    throw new Error('Range queries require ordered database');
  }
  
  const users = [];
  db.makeIterator();
  await db.iteratorJump(`user:${startId}`);
  
  while (true) {
    try {
      const { key, value } = await db.iteratorGet();
      
      if (!key.startsWith('user:')) break;
      
      const id = parseInt(key.split(':')[1]);
      if (id > endId) break;
      
      users.push({ id, data: value });
      await db.iteratorNext();
    } catch (err) {
      break;
    }
  }
  
  db.freeIterator();
  return users;
}
```

### Prefix Search

```javascript
async function searchByPrefix(prefix) {
  return await db.search('begin', prefix, 1000);
}

// Usage
const userKeys = await searchByPrefix('user:');
const sessionKeys = await searchByPrefix('session:');
```

### Secondary Index

```javascript
// Add entries
await idx.add('user:1', 'email:alice@example.com');
await idx.add('user:2', 'email:alice@example.com');

// Find all users with same email
const users = await idx.getValues('email:alice@example.com', 100);
// ['user:1', 'user:2']

// Prefix iteration
await idx.makeJumpIterator('email:alice');
while (true) {
  try {
    const { key, value } = await idx.getIteratorValue();
    if (!key.startsWith('email:alice')) break;
    console.log(`${value} has email ${key}`);
    await idx.continueIteration();
  } catch (err) {
    break;
  }
}
idx.freeIterator();
```

## Error Handling

All async methods return Promises that reject on failure:

```javascript
try {
  await db.get('nonexistent');
} catch (err) {
  console.error(err.message);
  // "[STATUS_NOT_FOUND_ERROR]: key not found"
}

try {
  await db.compareExchange('lock', 'wrong', 'value');
} catch (err) {
  console.error('CAS failed:', err.message);
}
```

## Performance Tips

1. **Use atomic operations** instead of get-modify-set patterns
2. **Batch operations** are more efficient than individual calls
3. **Reuse iterators** instead of creating new ones
4. **Choose appropriate DBM type**:
    - HashDBM for unordered key-value
    - TreeDBM for ordered/range queries
    - CacheDBM for LRU cache
5. **Enable compression** (`record_comp_mode: "RECORD_COMP_LZ4"`)
6. **Tune bucket count** based on expected records
7. **Use hard sync sparingly** (impacts write performance)
8. **Configure update logs** for point-in-time recovery

## Testing

```bash
npm test
```

The test suite includes comprehensive coverage of:
- Basic CRUD operations
- Atomic operations
- Batch processing
- Iterator operations
- Search functionality
- Database information methods
- Export/import operations
- Error handling

## License

ISC License - See LICENSE file for details

## Contributing

Contributions are welcome! Please submit issues and pull requests to the [GitHub repository](https://github.com/th3r0b0t/tkrzw-node).

## Credits

- **Tkrzw** - [Mikio Hirabayashi](https://dbmx.net/tkrzw/)
- **Node.js Bindings** - GROK and contributors

## Related Projects

- [Tkrzw C++ Library](https://dbmx.net/tkrzw/)
- [Tkrzw Python Bindings](https://github.com/estraier/tkrzw-python)
- [Tkrzw Ruby Bindings](https://github.com/estraier/tkrzw-ruby)

## Support

- [Documentation](https://dbmx.net/tkrzw/)
- [GitHub Issues](https://github.com/th3r0b0t/tkrzw-node/issues)
- [Tkrzw Mailing List](https://groups.google.com/g/tkrzw)