// comprehensive_example.mjs - Demonstrates all Tkrzw Node.js bindings features

import { polyDBM, polyIndex } from 'tkrzw-node';
import fs from 'fs';

const config = JSON.parse(fs.readFileSync('./tkrzw_config.json', 'utf8'));
const db = new polyDBM(config, './db/comprehensive_test.tkh');

async function demonstrateAllFeatures() {
    console.log('=== Tkrzw Node.js - Comprehensive Feature Demo ===\n');

    // ====== Basic Operations ======
    console.log('1. Basic Operations');
    await db.set('user:1', 'Alice');
    await db.set('user:2', 'Bob');
    console.log('✓ Set records');

    const value = await db.get('user:1');
    console.log('✓ Get record:', value);

    await db.append('user:1', ' Smith', ' ');
    console.log('✓ Append to record');

    const count = await db.count();
    console.log('✓ Total records:', count);

    // ====== Atomic Operations ======
    console.log('\n2. Atomic Operations');
    
    // Increment counter
    const newCount = await db.increment('counter', 10, 0);
    console.log('✓ Incremented counter:', newCount);
    
    // Compare and exchange (CAS)
    try {
        await db.compareExchange('user:1', 'Alice Smith', 'Alice Johnson');
        console.log('✓ Compare-exchange succeeded');
    } catch (err) {
        console.log('✗ Compare-exchange failed (expected):', err.message);
    }

    // Rename key
    await db.rekey('user:2', 'user:3', true, false);
    console.log('✓ Renamed key user:2 -> user:3');

    // ====== Batch Operations ======
    console.log('\n3. Batch Operations');
    
    // Multi-record compare-exchange
    await db.compareExchangeMulti(
        [{ key: 'batch:1', value: null }],  // Expected
        [{ key: 'batch:1', value: 'New' }, { key: 'batch:2', value: 'Values' }]  // Desired
    );
    console.log('✓ Multi compare-exchange');

    // Process multiple keys
    await db.processMulti(['batch:1', 'batch:2'], (exists, key, value) => {
        console.log(`  Processing ${key}: ${value}`);
        return polyDBM.NOOP;
    }, false);

    // ====== Record Processing ======
    console.log('\n4. Record Processing');
    
    // Process single record
    await db.process('user:1', (exists, key, value) => {
        if (exists) {
            console.log(`  ${key} = ${value}`);
            return value + ' [processed]';
        }
        return polyDBM.NOOP;
    }, true);
    console.log('✓ Process single record');

    // Process first record
    await db.processFirst((exists, key, value) => {
        console.log(`  First: ${key} = ${value}`);
        return polyDBM.NOOP;
    }, false);

    // Process all records
    let recordCount = 0;
    await db.processEach((exists, key, value) => {
        recordCount++;
        return polyDBM.NOOP;
    }, false);
    console.log(`✓ Processed ${recordCount} records`);

    // ====== Iterator Operations ======
    console.log('\n5. Iterator Operations');
    
    if (db.isOrdered()) {
        db.makeIterator();
        
        // Jump to first
        await db.iteratorFirst();
        let pair = await db.iteratorGet();
        console.log('✓ First record:', pair);
        
        // Jump to specific key
        await db.iteratorJump('user:');
        
        // Iterate through records
        console.log('  Iterating:');
        for (let i = 0; i < 3; i++) {
            try {
                pair = await db.iteratorGet();
                console.log(`    ${pair.key} = ${pair.value}`);
                await db.iteratorNext();
            } catch (err) {
                break;
            }
        }
        
        db.freeIterator();
        console.log('✓ Iterator operations completed');
    }

    // ====== Search Operations ======
    console.log('\n6. Search Operations');
    
    // Search by prefix
    const keys = await db.search('begin', 'user:', 10);
    console.log('✓ Keys starting with "user:":', keys);

    // Search with regex (if supported)
    try {
        const regexKeys = await db.search('regex', '^user:\\d+$', 10);
        console.log('✓ Regex search results:', regexKeys);
    } catch (err) {
        console.log('  Regex search not supported in this DBM type');
    }

    // ====== Database Information ======
    console.log('\n7. Database Information');
    
    console.log('✓ Is open:', db.isOpen());
    console.log('✓ Is writable:', db.isWritable());
    console.log('✓ Is healthy:', db.isHealthy());
    console.log('✓ Is ordered:', db.isOrdered());
    console.log('✓ File path:', await db.getFilePath());
    console.log('✓ File size:', await db.getFileSize(), 'bytes');
    console.log('✓ Timestamp:', new Date(await db.getTimestamp() * 1000));
    
    const inspection = await db.inspect();
    console.log('✓ Database inspection:', inspection);

    // ====== Synchronization ======
    console.log('\n8. Synchronization');
    
    await db.sync(true);
    console.log('✓ Hard sync completed');

    // Check if rebuild needed
    try {
        await db.shouldBeRebuilt();
        console.log('  Database needs rebuilding');
        await db.rebuild(config);
        console.log('✓ Database rebuilt');
    } catch (err) {
        console.log('  Database doesn\'t need rebuilding');
    }

    // ====== Export/Import ======
    console.log('\n9. Export/Import Operations');
    
    await db.exportToFlatRecords('./db/export.tkf');
    console.log('✓ Exported to flat records');
    
    await db.exportKeysAsLines('./db/keys.txt');
    console.log('✓ Exported keys as lines');
    
    // Import would overwrite existing data
    // await db.importFromFlatRecords('./db/export.tkf');

    // ====== Deletion Operations ======
    console.log('\n10. Deletion Operations');
    
    await db.remove('batch:1');
    console.log('✓ Removed single record');
    
    // Clear all records (commented out for safety)
    // await db.clear();
    // console.log('✓ Cleared all records');

    // ====== Index Operations ======
    console.log('\n11. Index Operations');
    
    const indexConfig = JSON.parse(fs.readFileSync('./tkrzw_index_config.json', 'utf8'));
    const idx = new polyIndex(indexConfig, './db/comprehensive_index.tkt');
    
    await idx.add('tags', 'javascript');
    await idx.add('tags', 'nodejs');
    await idx.add('tags', 'database');
    console.log('✓ Added index entries');
    
    const values = await idx.getValues('tags', 10);
    console.log('✓ Index values:', values);
    
    const exists = await idx.check('tags', 'nodejs').then(() => true).catch(() => false);
    console.log('✓ Index check (nodejs exists):', exists);
    
    // Index iteration
    await idx.makeJumpIterator('tags');
    try {
        const indexPair = await idx.getIteratorValue();
        console.log('✓ Index iterator:', indexPair);
        await idx.continueIteration();
    } catch (err) {
        console.log('  No more index entries');
    }
    idx.freeIterator();
    
    await idx.remove('tags', 'database');
    console.log('✓ Removed index entry');
    
    await idx.sync(true);
    console.log('✓ Synced index');
    
    idx.close();
    console.log('✓ Closed index');

    // ====== Restoration (Advanced) ======
    console.log('\n12. Database Restoration');
    console.log('  (Skipping restoration demo - requires update logs)');
    // await polyDBM.restoreDatabase(
    //     './db/old.tkh',
    //     './db/restored.tkh',
    //     'HashDBM',
    //     -1
    // );

    // ====== Cleanup ======
    console.log('\n13. Cleanup');
    db.close();
    console.log('✓ Closed database');

    console.log('\n=== Demo Complete ===');
}

// Run demonstration
demonstrateAllFeatures().catch(err => {
    console.error('Error:', err);
    process.exit(1);
});

// ====== Usage Examples for Specific Features ======

// Example: Atomic Counter
async function atomicCounter() {
    const db = new polyDBM(config, './db/counter.tkh');
    
    // Increment counter by 1, starting from 0
    const count = await db.increment('page_views', 1, 0);
    console.log('Page views:', count);
    
    db.close();
}

// Example: Compare-and-Swap Pattern
async function casPattern() {
    const db = new polyDBM(config, './db/cas.tkh');
    
    await db.set('lock', 'unlocked');
    
    // Try to acquire lock
    try {
        await db.compareExchange('lock', 'unlocked', 'locked');
        console.log('Lock acquired!');
        
        // Do work...
        
        // Release lock
        await db.compareExchange('lock', 'locked', 'unlocked');
        console.log('Lock released!');
    } catch (err) {
        console.log('Lock already held');
    }
    
    db.close();
}

// Example: Batch Processing
async function batchProcessing() {
    const db = new polyDBM(config, './db/batch.tkh');
    
    // Set multiple records
    for (let i = 0; i < 100; i++) {
        await db.set(`record:${i}`, `value${i}`);
    }
    
    // Process all records in batch
    const results = [];
    await db.processEach((exists, key, value) => {
        if (exists && value.includes('5')) {
            results.push({ key, value });
        }
        return polyDBM.NOOP;
    }, false);
    
    console.log('Found', results.length, 'records containing "5"');
    
    db.close();
}

// Example: Iterator Pattern
async function iteratorPattern() {
    const db = new polyDBM(config, './db/iterator.tkh');
    
    // Populate with sorted data
    for (let i = 1; i <= 10; i++) {
        await db.set(`key${i.toString().padStart(3, '0')}`, `value${i}`);
    }
    
    if (db.isOrdered()) {
        db.makeIterator();
        
        // Iterate from start
        await db.iteratorFirst();
        
        while (true) {
            try {
                const { key, value } = await db.iteratorGet();
                console.log(`${key}: ${value}`);
                await db.iteratorNext();
            } catch (err) {
                break;  // End of iteration
            }
        }
        
        db.freeIterator();
    }
    
    db.close();
}

// Example: Search Patterns
async function searchPatterns() {
    const db = new polyDBM(config, './db/search.tkh');
    
    await db.set('user:alice', 'Alice');
    await db.set('user:bob', 'Bob');
    await db.set('admin:charlie', 'Charlie');
    
    // Find all user keys
    const userKeys = await db.search('begin', 'user:', 100);
    console.log('User keys:', userKeys);
    
    // Find all admin keys
    const adminKeys = await db.search('begin', 'admin:', 100);
    console.log('Admin keys:', adminKeys);
    
    db.close();
}