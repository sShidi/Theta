import {polyDBM, polyIndex} from 'tkrzw-node';
import fs from 'fs';
import {expect} from 'chai';
import {afterEach, beforeEach, describe, it} from 'mocha';

const configPath = './tkrzw_config.json';
const indexConfigPath = './tkrzw_index_config.json';
const dbPath = 'db/comprehensive_test.tkh';
const indexPath = 'db/comprehensive_index.tkt';

let db;
let idx;
let config;
let indexConfig;

describe('Tkrzw Node.js Bindings - Basic Operations', function () {
	this.timeout(10000);

	before(async () => {  // Note: make it async
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	after(() => {
		db.close();
	});

	it('should set and get records successfully', async () => {
		await db.set('user:1', 'Alice');
		await db.set('user:2', 'Bob');
		const value1 = await db.get('user:1');
		expect(value1).to.equal('Alice');
		const value2 = await db.get('user:2');
		expect(value2).to.equal('Bob');
	});

	it('should handle get with default value', async () => {
		const value = await db.get('nonexistent', 'default');
		expect(value).to.equal('default');
	});

	it('should append to record successfully', async () => {
		await db.set('user:1', 'Alice');
		await db.append('user:1', 'Smith', ' ');
		const value = await db.get('user:1');
		expect(value).to.equal('Alice Smith');
	});

	it('should count records correctly', async () => {
		await db.set('count:1', 'one');
		await db.set('count:2', 'two');
		const count = await db.count();
		expect(count).to.be.at.least(2);
	});

	it('should throw error on invalid set arguments', async () => {
		try {
			await db.set(123, 'value');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});

	it('should throw error on invalid get arguments', async () => {
		try {
			await db.get(123);
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});

	it('should throw error on invalid append arguments', async () => {
		try {
			await db.append(123, 'append');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});
});

describe('Tkrzw Node.js Bindings - Atomic Operations', function () {
	this.timeout(10000);

	before(async () => {  // Note: make it async
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	after(() => {
		db.close();
	});

	it('should increment counter successfully', async () => {
		const newCount = await db.increment('counter', 10, 0);
		expect(newCount).to.equal(10);
		const updated = await db.increment('counter', 10);
		expect(updated).to.equal(20);
	});

	it('should handle compare-exchange successfully', async () => {
		await db.set('ce:test', 'old');
		await db.compareExchange('ce:test', 'old', 'new');
		const value = await db.get('ce:test');
		expect(value).to.equal('new');
	});

	it('should fail compare-exchange on mismatch', async () => {
		await db.set('ce:test', 'old');
		try {
			await db.compareExchange('ce:test', 'wrong', 'new');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('CompareExchange failed');
		}
	});

	it('should rekey successfully', async () => {
		await db.set('oldkey', 'value');
		await db.rekey('oldkey', 'newkey', false, false);
		const value = await db.get('newkey');
		expect(value).to.equal('value');
		try {
			await db.get('oldkey');
			expect.fail('Key not found');
		} catch (err) {
			expect(err.message).to.include('Key not found');
		}
	});

	it('should throw error on invalid increment arguments', async () => {
		try {
			await db.increment(123, 10);
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});

	it('should throw error on invalid compareExchange arguments', async () => {
		try {
			await db.compareExchange('key', 123, 'new');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});

	it('should throw error on invalid rekey arguments', async () => {
		try {
			await db.rekey(123, 'newkey');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});
});

describe('Tkrzw Node.js Bindings - Batch Operations', function () {
	this.timeout(10000);

	before(async () => {  // Note: make it async
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	after(() => {
		db.close();
	});

	it('should perform multi compare-exchange successfully', async () => {
		await db.set('batch:1', '');
		await db.compareExchangeMulti([{key: 'batch:1', value: null}], [{key: 'batch:1', value: 'New'}, {
			key: 'batch:2',
			value: 'Values'
		}]);
		const value1 = await db.get('batch:1');
		expect(value1).to.equal('New');
		const value2 = await db.get('batch:2');
		expect(value2).to.equal('Values');
	});

	it('should process multiple keys successfully', async () => {
		await db.set('pm:1', 'one');
		await db.set('pm:2', 'two');
		let processed = [];
		await db.processMulti(['pm:1', 'pm:2'], (exists, key, value) => {
			if (exists) processed.push(value);
			return polyDBM.NOOP;
		}, false);
		expect(processed).to.deep.equal(['one', 'two']);
	});

	it('should throw error on invalid compareExchangeMulti arguments', async () => {
		try {
			await db.compareExchangeMulti('invalid', []);
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});

	it('should throw error on invalid processMulti arguments', async () => {
		try {
			await db.processMulti([], 'not a function');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});
});

describe('Tkrzw Node.js Bindings - Record Processing', function () {
	this.timeout(10000);

	before(async () => {  // Note: make it async
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	after(() => {
		db.close();
	});

	it('should process single record successfully', async () => {
		await db.set('rp:1', 'original');
		await db.process('rp:1', (exists, key, value) => {
			if (exists) return value + ' [processed]';
			return polyDBM.NOOP;
		}, true);
		const value = await db.get('rp:1');
		expect(value).to.equal('original [processed]');
	});

	it('should process first record successfully', async () => {
		await db.set('rp:first', 'first');
		let processed = false;
		await db.processFirst((exists, key, value) => {
			if (exists) processed = true;
			return polyDBM.NOOP;
		}, false);
		expect(processed).to.be.true;
	});

	it('should process each record successfully', async () => {
		await db.set('rp:e1', 'one');
		await db.set('rp:e2', 'two');
		let count = 0;
		await db.processEach((exists, key, value) => {
			if (exists) count++;
			return polyDBM.NOOP;
		}, false);
		expect(count).to.be.at.least(2);
	});

	it('should throw error on invalid process arguments', async () => {
		try {
			await db.process('key', 'not function', true);
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});

	it('should throw error on invalid processFirst arguments', async () => {
		try {
			await db.processFirst('not function');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});

	it('should throw error on invalid processEach arguments', async () => {
		try {
			await db.processEach('not function');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});
});

describe('Tkrzw Node.js Bindings - Iterator Operations', function () {
	this.timeout(10000);

	before(async () => {  // Note: make it async
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	after(() => {
		db.close();
	});

	it('should perform iterator operations successfully', async function () {
		if (db.isOrdered()) {
			await db.set('it:1', 'one');
			await db.set('it:2', 'two');
			db.makeIterator();
			await db.iteratorFirst();
			const pair1 = await db.iteratorGet();
			expect(pair1.value).to.equal('one');
			await db.iteratorNext();
			const pair2 = await db.iteratorGet();
			expect(pair2.value).to.equal('two');
			db.freeIterator();
		} else {
			// this.skip();
		}
	});

	it('should handle iterator jump successfully', async function () {
		if (db.isOrdered()) {
			await db.set('jump:aaa', 'aaa');
			await db.set('jump:bbb', 'bbb');
			db.makeIterator();
			await db.iteratorJump('jump:bbb');
			const pair = await db.iteratorGet();
			expect(pair.value).to.equal('bbb');
			db.freeIterator();
		} else {
			// this.skip();
		}
	});

	it('should throw error on iterator operations without iterator', async () => {
		try {
			await db.iteratorFirst();
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err).to.exist;
		}
	});
});

describe('Tkrzw Node.js Bindings - Search Operations', function () {
	this.timeout(10000);

	before(async () => {  // Note: make it async
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	after(() => {
		db.close();
	});

	it('should search by prefix successfully', async () => {
		await db.set('search:user1', 'user1');
		await db.set('search:user2', 'user2');
		const keys = await db.search('begin', 'search:user', 10);
		expect(keys).to.have.lengthOf(2);
	});

	it('should search by regex successfully', async () => {
		await db.set('regex:123', '123');
		await db.set('regex:456', '456');
		const keys = await db.search('regex', '^regex:\\d+$', 10);
		expect(keys).to.have.lengthOf(2);
	});

	it('should handle unsupported search mode', async () => {
		try {
			await db.search('invalid', 'pattern', 10);
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Search failed');
		}
	});

	it('should throw error on invalid search arguments', async () => {
		try {
			await db.search(123, 'pattern');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});
});

describe('Tkrzw Node.js Bindings - Database Information', function () {
	this.timeout(10000);

	before(async () => {  // Note: make it async
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	after(() => {
		db.close();
	});

	it('should get database info successfully', async () => {
		expect(db.isOpen()).to.be.true;
		expect(db.isWritable()).to.be.true;
		expect(db.isHealthy()).to.be.true;
		const path = await db.getFilePath();
		expect(path).to.equal(dbPath);
		const size = await db.getFileSize();
		expect(size).to.be.a('number');
		const timestamp = await db.getTimestamp();
		expect(timestamp).to.be.a('number');
		const inspection = await db.inspect();
		expect(inspection).to.be.an('object');
	});
});

describe('Tkrzw Node.js Bindings - Synchronization', function () {
	this.timeout(10000);

	before(async () => {  // Note: make it async
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	after(() => {
		db.close();
	});

	it('should sync successfully', async () => {
		await db.sync(true);
	});
});

describe('Tkrzw Node.js Bindings - Deletion Operations', function () {
	this.timeout(10000);

	beforeEach(async () => {  // Note: make it async
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	afterEach(() => {
		db.close();
	});

	it('should remove single record successfully', async () => {
		await db.set('del:1', 'to delete');
		await db.remove('del:1');
		const res = await db.get('del:1');
		expect(res).to.equal('');
	});

	it('should clear all records successfully', async () => {
		await db.set('clear:1', 'one');
		await db.clear();
		const count = await db.count();
		expect(count).to.equal(0);
	});

	it('should throw error on remove non-existent key', async () => {
		try {
			await db.remove('nonexistent');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Remove failed');
		}
	});
});

describe('Tkrzw Node.js Bindings - Index Operations', function () {
	this.timeout(10000);

	beforeEach(() => {
		indexConfig = JSON.parse(fs.readFileSync(indexConfigPath, 'utf8'));
		idx = new polyIndex(indexConfig, indexPath);
	});

	afterEach(() => {
		idx.close();
	});

	it('should add index entries successfully', async () => {
		await idx.add('tags', 'javascript');
		await idx.add('tags', 'nodejs');
	});

	it('should get index values successfully', async () => {
		const values = await idx.getValues('tags', 10);
		expect(values).to.include('javascript');
	});

	it('should check index entry successfully', async () => {
		const exists = await idx.check('tags', 'nodejs');
		expect(exists).to.be.true;
	});

	it('should perform index iteration successfully', async () => {
		await idx.makeJumpIterator('tags');
		const pair = await idx.getIteratorValue();
		expect(pair).to.have.property('key');
		await idx.continueIteration();
		idx.freeIterator();
	});

	it('should remove index entry successfully', async () => {
		await idx.add('tags', 'database');
		await idx.remove('tags', 'database');
	});

	it('should sync index successfully', async () => {
		await idx.sync(true);
	});

	it('should throw error on invalid add arguments', async () => {
		try {
			await idx.add(123, 'value');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Add failed');
		}
	});

	it('should throw error on check non-existent', async () => {
		try {
			await idx.check('nonexistent', 'value');
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Check failed');
		}
	});
});

describe('Tkrzw Node.js Bindings - Atomic Counter Example', function () {
	this.timeout(10000);

	let counterDb;

	beforeEach(() => {
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		counterDb = new polyDBM(config, './db/counter.tkh');
	});

	afterEach(() => {
		counterDb.close();
	});

	it('should increment counter in atomicCounter example', async () => {
		const count = await counterDb.increment('page_views', 1, 0);
		expect(count).to.be.a('number');
	});
});

describe('Tkrzw Node.js Bindings - CAS Pattern Example', function () {
	this.timeout(10000);

	let casDb;

	beforeEach(() => {
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		casDb = new polyDBM(config, './db/cas.tkh');
	});

	afterEach(() => {
		casDb.close();
	});

	it('should acquire and release lock in casPattern example', async () => {
		await casDb.set('lock', 'unlocked');
		await casDb.compareExchange('lock', 'unlocked', 'locked');
		const value = await casDb.get('lock');
		expect(value).to.equal('locked');
		await casDb.compareExchange('lock', 'locked', 'unlocked');
	});
});

describe('Tkrzw Node.js Bindings - Batch Processing Example', function () {
	this.timeout(10000);

	let batchDb;

	beforeEach(() => {
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		batchDb = new polyDBM(config, './db/batch.tkh');
	});

	afterEach(() => {
		batchDb.close();
	});

	it('should process batch in batchProcessing example', async () => {
		for (let i = 0; i < 10; i++) {
			await batchDb.set(`record:${i}`, `value${i}`);
		}
		let count = 0;
		await batchDb.processEach((exists, key, value) => {
			if (exists && value.includes('5')) count++;
			return polyDBM.NOOP;
		}, false);
		expect(count).to.equal(1);
	});
});

describe('Tkrzw Node.js Bindings - Iterator Pattern Example', function () {
	this.timeout(10000);

	let iterDb;

	beforeEach(() => {
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		iterDb = new polyDBM(config, './db/iterator.tkh');
	});

	afterEach(() => {
		iterDb.close();
	});

	it('should iterate in iteratorPattern example', async function () {
		if (iterDb.isOrdered()) {
			for (let i = 1; i <= 3; i++) {
				await iterDb.set(`key${i.toString().padStart(3, '0')}`, `value${i}`);
			}
			iterDb.makeIterator();
			await iterDb.iteratorFirst();
			let count = 0;
			while (true) {
				try {
					const pair = await iterDb.iteratorGet();
					expect(pair.value).to.equal(`value${count + 1}`);
					await iterDb.iteratorNext();
					count++;
				} catch (err) {
					break;
				}
			}
			expect(count).to.equal(3);
			iterDb.freeIterator();
		} else {
			this.skip();
		}
	});
});

describe('Tkrzw Node.js Bindings - Search Patterns Example', function () {
	this.timeout(10000);

	let searchDb;

	beforeEach(() => {
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		searchDb = new polyDBM(config, './db/search.tkh');
	});

	afterEach(() => {
		searchDb.close();
	});

	it('should search patterns in searchPatterns example', async () => {
		await searchDb.set('user:alice', 'Alice');
		await searchDb.set('user:bob', 'Bob');
		const userKeys = await searchDb.search('begin', 'user:', 100);
		expect(userKeys).to.have.lengthOf(2);
	});
});

describe('Tkrzw Node.js Bindings - Extract All Records', function() {
	this.timeout(10000);

	beforeEach(async () => {
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	afterEach(() => {
		db.close();
	});

	it('should extract all records using processEach', async () => {
		// Add sample records
		await db.set('key1', 'value1');
		await db.set('key2', 'value2');
		await db.set('key3', 'value3');

		const extracted = [];
		await db.processEach((exists, key, value) => {
			if (exists) {
				extracted.push({ key, value });
			}
			return polyDBM.NOOP;
		}, false);

		expect(extracted).to.have.lengthOf(3);
		expect(extracted).to.deep.include.members([
			{ key: 'key1', value: 'value1' },
			{ key: 'key2', value: 'value2' },
			{ key: 'key3', value: 'value3' }
		]);
	});

	it('should handle empty database with processEach', async () => {
		const extracted = [];
		await db.processEach((exists, key, value) => {
			if (exists) {
				extracted.push({ key, value });
			}
			return polyDBM.NOOP;
		}, false);

		expect(extracted).to.have.lengthOf(0);
	});

	it('should throw error on invalid processEach arguments', async () => {
		try {
			await db.processEach('invalid', false);
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});
});

describe('Tkrzw Node.js Bindings - Search Features', function() {
	this.timeout(10000);

	beforeEach(async () => {
		config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
		db = new polyDBM(config, dbPath);
		await db.clear();
	});

	afterEach(() => {
		db.close();
	});

	it('should search with contain mode successfully', async () => {
		// Add sample records
		await db.set('animal:cat', 'feline');
		await db.set('animal:scatter', 'disperse');
		await db.set('vehicle:car', 'automobile');
		await db.set('animal:dog', 'canine');

		const matches = await db.search('contain', 'cat', 10);
		expect(matches).to.have.lengthOf(2);
		expect(matches).to.have.members(['animal:cat', 'animal:scatter']);
	});

	it('should search with end mode successfully', async () => {
		// Add sample records
		await db.set('file:report.pdf', 'document');
		await db.set('image:photo.jpg', 'picture');
		await db.set('data:backup.zip', 'archive');
		await db.set('text:note.txt', 'notes');

		const matches = await db.search('end', '.pdf', 10);
		expect(matches).to.have.lengthOf(1);
		expect(matches).to.have.members(['file:report.pdf']);
	});

	it('should return empty array for no matches in contain mode', async () => {
		await db.set('key1', 'value1');
		await db.set('key2', 'value2');

		const matches = await db.search('contain', 'nonexistent', 10);
		expect(matches).to.be.an('array').that.is.empty;
	});

	it('should return empty array for no matches in end mode', async () => {
		await db.set('key1', 'value1');
		await db.set('key2', 'value2');

		const matches = await db.search('end', '.ext', 10);
		expect(matches).to.be.an('array').that.is.empty;
	});

	it('should throw error on invalid search mode', async () => {
		try {
			await db.search('invalid', 'pattern', 10);
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('unknown search mode');
		}
	});

	it('should throw error on invalid search arguments', async () => {
		try {
			await db.search('contain', 123, 10);
			expect.fail('Should have thrown');
		} catch (err) {
			expect(err.message).to.include('Invalid arguments');
		}
	});
});