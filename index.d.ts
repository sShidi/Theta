// index.d.ts - Complete TypeScript definitions for tkrzw-node

declare module 'tkrzw-node' {
    /**
     * Configuration for opening a Tkrzw database
     */
    export interface DBMConfig {
        /** Database type: "HashDBM", "TreeDBM", "SkipDBM", "TinyDBM", "BabyDBM", "CacheDBM", "StdHashDBM", "StdTreeDBM" */
        dbm?: string;
        
        /** Offset width for records */
        offset_width?: string;
        
        /** Alignment power (2^align_pow bytes) */
        align_pow?: string;
        
        /** Number of hash buckets (for hash-based DBMs) */
        num_buckets?: string;
        
        /** File type: "MemoryMapAtomicFile", "MemoryMapParallelFile", "PositionalParallelFile", "PositionalAtomicFile" */
        file?: string;
        
        /** Minimum read size in bytes */
        min_read_size?: string;
        
        /** Whether to cache hash buckets */
        cache_buckets?: string;
        
        /** Update mode: "UPDATE_DEFAULT", "UPDATE_IN_PLACE", "UPDATE_APPENDING" */
        update_mode?: string;
        
        /** Restore mode flags */
        restore_mode?: string;
        
        /** Update log file prefix */
        ulog_prefix?: string;
        
        /** Maximum update log file size */
        ulog_max_file_size?: string;
        
        /** Update log server ID */
        ulog_server_id?: string;
        
        /** Update log DBM index */
        ulog_dbm_index?: string;
        
        /** Record compression mode: "RECORD_COMP_NONE", "RECORD_COMP_ZLIB", "RECORD_COMP_ZSTD", "RECORD_COMP_LZ4", "RECORD_COMP_LZMA" */
        record_comp_mode?: string;
        
        /** Page update mode (for B-tree based DBMs): "PAGE_UPDATE_NONE", "PAGE_UPDATE_WRITE" */
        page_update_mode?: string;
        
        /** Key comparator (for ordered DBMs): "LexicalKeyComparator", "DecimalKeyComparator", "RealNumberKeyComparator" */
        key_comparator?: string;
        
        /** Maximum number of branches in B-tree nodes */
        max_branches?: string;
        
        [key: string]: string | undefined;
    }

    /**
     * Index configuration for Tkrzw PolyIndex
     */
    export interface IndexConfig extends DBMConfig {}

    /**
     * Key-value pair returned by iterators
     */
    export interface KeyValuePair {
        key: string;
        value: string;
    }

    /**
     * Record processor function type
     * @param exists - Whether the key exists
     * @param key - The key being processed
     * @param value - The current value (empty string if key doesn't exist)
     * @returns New value to set, NOOP to keep unchanged, or REMOVE to delete
     */
    export type RecordProcessor = (
        exists: boolean,
        key: string,
        value: string
    ) => string | symbol | Promise<string | symbol>;

    /**
     * Search mode for key search operations
     */
    export type SearchMode = 
        | 'begin'      // Keys that begin with the pattern
        | 'contain'    // Keys that contain the pattern
        | 'end'        // Keys that end with the pattern
        | 'regex'      // Keys matching the regex pattern
        | 'edit'       // Keys within edit distance
        | 'editbin';   // Keys within binary edit distance

    /**
     * Main database class - Polymorphic database manager
     */
    export class polyDBM {
        /**
         * Symbol to return from RecordProcessor to indicate no operation
         */
        static readonly NOOP: symbol;
        
        /**
         * Symbol to return from RecordProcessor to indicate record removal
         */
        static readonly REMOVE: symbol;

        /**
         * Create a new polyDBM instance
         * @param config - Configuration object or JSON string
         * @param path - Database file path
         */
        constructor(config: DBMConfig | string, path: string);

        // ====== Basic Operations ======
        
        /**
         * Set a record (replaces existing value)
         * @param key - Record key
         * @param value - Record value
         */
        set(key: string, value: string): Promise<void>;

        /**
         * Get a record value
         * @param key - Record key
         * @returns Promise resolving to the value
         * @throws If key doesn't exist
         */
        get(key: string): Promise<string>;

        /**
         * Get a record value with default fallback
         * @param key - Record key
         * @param defaultValue - Value to return if key doesn't exist
         * @returns Promise resolving to the value or default
         */
        getSimple(key: string, defaultValue: string): Promise<string>;

        /**
         * Remove a record
         * @param key - Record key to remove
         */
        remove(key: string): Promise<void>;

        /**
         * Append data to an existing record
         * @param key - Record key
         * @param value - Value to append
         * @param delimiter - Optional delimiter to insert before appending
         */
        append(key: string, value: string, delimiter?: string): Promise<void>;

        // ====== Atomic Operations ======

        /**
         * Compare and exchange a record atomically
         * @param key - Record key
         * @param expected - Expected current value (null means "must not exist")
         * @param desired - Desired new value (null means "remove")
         * @returns Promise resolving on success, rejecting with actual value on failure
         */
        compareExchange(
            key: string,
            expected: string | null,
            desired: string | null
        ): Promise<void>;

        /**
         * Atomically increment a numeric value
         * @param key - Record key
         * @param increment - Amount to add (can be negative)
         * @param initial - Initial value if key doesn't exist (default: 0)
         * @returns Promise resolving to the new value
         */
        increment(key: string, increment: number, initial?: number): Promise<number>;

        /**
         * Compare and exchange multiple records atomically
         * @param expected - Array of expected key-value pairs
         * @param desired - Array of desired key-value pairs
         */
        compareExchangeMulti(
            expected: Array<{ key: string; value: string | null }>,
            desired: Array<{ key: string; value: string | null }>
        ): Promise<void>;

        /**
         * Rename a key atomically
         * @param oldKey - Current key name
         * @param newKey - New key name
         * @param overwrite - Whether to overwrite existing newKey (default: true)
         * @param copying - If true, copy instead of move (default: false)
         */
        rekey(
            oldKey: string,
            newKey: string,
            overwrite?: boolean,
            copying?: boolean
        ): Promise<void>;

        // ====== Record Processing ======

        /**
         * Process a single record with a callback
         * @param key - Record key to process
         * @param processor - Function to process the record
         * @param writable - Whether processor can modify the record
         */
        process(
            key: string,
            processor: RecordProcessor,
            writable: boolean
        ): Promise<void>;

        /**
         * Process multiple records with a callback
         * @param keys - Array of keys to process
         * @param processor - Function to process each record
         * @param writable - Whether processor can modify records
         */
        processMulti(
            keys: string[],
            processor: RecordProcessor,
            writable: boolean
        ): Promise<void>;

        /**
         * Process the first record in the database
         * @param processor - Function to process the record
         * @param writable - Whether processor can modify the record
         */
        processFirst(processor: RecordProcessor, writable: boolean): Promise<void>;

        /**
         * Process each record in the database
         * @param processor - Function to process each record
         * @param writable - Whether processor can modify records
         */
        processEach(processor: RecordProcessor, writable: boolean): Promise<void>;

        // ====== Database Information ======

        /**
         * Get the number of records in the database
         */
        count(): Promise<number>;

        /**
         * Get the file size in bytes
         */
        getFileSize(): Promise<number>;

        /**
         * Get the database file path
         */
        getFilePath(): Promise<string>;

        /**
         * Get the last modification timestamp (Unix time)
         */
        getTimestamp(): Promise<number>;

        /**
         * Get database inspection information
         * @returns Object with database metadata
         */
        inspect(): Promise<Record<string, string>>;

        /**
         * Check if database is open
         */
        isOpen(): boolean;

        /**
         * Check if database is writable
         */
        isWritable(): boolean;

        /**
         * Check if database is healthy
         */
        isHealthy(): boolean;

        /**
         * Check if database is ordered (supports iteration)
         */
        isOrdered(): boolean;

        // ====== Search Operations ======

        /**
         * Search for keys matching a pattern
         * @param mode - Search mode ('begin', 'contain', 'end', 'regex', etc.)
         * @param pattern - Search pattern
         * @param capacity - Maximum number of results (0 for unlimited)
         * @returns Array of matching keys
         */
        search(mode: SearchMode, pattern: string, capacity?: number): Promise<string[]>;

        // ====== Iterator Operations ======

        /**
         * Create an iterator for traversing records
         */
        makeIterator(): void;

        /**
         * Move iterator to the first record
         */
        iteratorFirst(): Promise<void>;

        /**
         * Move iterator to the last record
         */
        iteratorLast(): Promise<void>;

        /**
         * Jump iterator to a specific key
         * @param key - Key to jump to
         */
        iteratorJump(key: string): Promise<void>;

        /**
         * Jump iterator to lower bound
         * @param key - Key bound
         * @param inclusive - Whether to include the key itself
         */
        iteratorJumpLower(key: string, inclusive?: boolean): Promise<void>;

        /**
         * Jump iterator to upper bound
         * @param key - Key bound
         * @param inclusive - Whether to include the key itself
         */
        iteratorJumpUpper(key: string, inclusive?: boolean): Promise<void>;

        /**
         * Move iterator to next record
         */
        iteratorNext(): Promise<void>;

        /**
         * Move iterator to previous record
         */
        iteratorPrevious(): Promise<void>;

        /**
         * Get the current key-value pair from iterator
         */
        iteratorGet(): Promise<KeyValuePair>;

        /**
         * Set the value at current iterator position
         * @param value - New value
         */
        iteratorSet(value: string): Promise<void>;

        /**
         * Remove the record at current iterator position
         */
        iteratorRemove(): Promise<void>;

        /**
         * Free the iterator resources
         */
        freeIterator(): boolean;

        // ====== Database Maintenance ======

        /**
         * Check if database should be rebuilt for optimization
         */
        shouldBeRebuilt(): Promise<void>;

        /**
         * Rebuild database for optimization
         * @param config - Rebuild configuration
         */
        rebuild(config: DBMConfig | string): Promise<void>;

        /**
         * Synchronize database to disk
         * @param hard - If true, use hard sync (fsync)
         */
        sync(hard: boolean): Promise<void>;

        /**
         * Clear all records from the database
         */
        clear(): Promise<void>;

        // ====== Export/Import ======

        /**
         * Export database to flat records file
         * @param destPath - Destination file path
         */
        exportToFlatRecords(destPath: string): Promise<void>;

        /**
         * Import database from flat records file
         * @param srcPath - Source file path
         */
        importFromFlatRecords(srcPath: string): Promise<void>;

        /**
         * Export all keys as text lines
         * @param destPath - Destination file path
         */
        exportKeysAsLines(destPath: string): Promise<void>;

        // ====== Restoration ======

        /**
         * Restore database from update logs
         * @param oldFilePath - Path to old database file
         * @param newFilePath - Path for restored database
         * @param className - DBM class name (optional)
         * @param endOffset - End offset in update log (default: -1 for all)
         */
        restoreDatabase(
            oldFilePath: string,
            newFilePath: string,
            className?: string,
            endOffset?: number
        ): Promise<void>;

        /**
         * Close the database
         */
        close(): boolean;
    }

    /**
     * Index class for secondary indexing
     */
    export class polyIndex {
        /**
         * Create a new polyIndex instance
         * @param config - Index configuration
         * @param path - Index file path
         */
        constructor(config: IndexConfig | string, path: string);

        /**
         * Add a key-value pair to the index
         * @param key - Index key
         * @param value - Value to associate with the key
         */
        add(key: string, value: string): Promise<void>;

        /**
         * Get all values associated with a key
         * @param key - Index key
         * @param maxRecords - Maximum number of values to retrieve (0 for all)
         * @returns Array of values
         */
        getValues(key: string, maxRecords: number): Promise<string[]>;

        /**
         * Check if a key-value pair exists in the index
         * @param key - Index key
         * @param value - Value to check
         */
        check(key: string, value: string): Promise<void>;

        /**
         * Remove a key-value pair from the index
         * @param key - Index key
         * @param value - Value to remove
         */
        remove(key: string, value: string): Promise<void>;

        /**
         * Check if index should be rebuilt
         */
        shouldBeRebuilt(): Promise<void>;

        /**
         * Rebuild the index for optimization
         */
        rebuild(): Promise<void>;

        /**
         * Synchronize index to disk
         * @param hard - If true, use hard sync
         */
        sync(hard: boolean): Promise<void>;

        /**
         * Create an iterator for the index
         * @param partialKey - Partial key to start iteration from
         */
        makeJumpIterator(partialKey: string): Promise<void>;

        /**
         * Get current key-value pair from index iterator
         */
        getIteratorValue(): Promise<KeyValuePair>;

        /**
         * Move index iterator to next entry
         */
        continueIteration(): Promise<void>;

        /**
         * Free index iterator resources
         */
        freeIterator(): boolean;

        /**
         * Close the index
         */
        close(): boolean;
    }

    const tkrzw: {
        polyDBM: typeof polyDBM;
        polyIndex: typeof polyIndex;
    };

    export default tkrzw;
}