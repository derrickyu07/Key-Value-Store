# kvstore

A small persistent key value store written in C++, inspired by Redis. It runs as an interactive command line program, keeps data in memory, persists every write to an append only log, and evicts the least recently used key when it reaches capacity.

I built this to learn modern C++ from the ground up: STL containers, iterators, RAII, file I/O, CMake, and unit testing with GoogleTest.

## Example

```
$ ./kvstore
> SET name derrick
OK
> SET greeting hello world
OK
> GET greeting
hello world
> DEL name
OK
> GET name
(nil)
> EXIT
```

Restart the program and `greeting` is still there.

## Commands

| Command | Description | Response |
|---|---|---|
| `SET key value` | Store a value. Values may contain spaces. | `OK` |
| `GET key` | Read a value. | the value, or `(nil)` |
| `DEL key` | Delete a key. | `OK`, or `(nil)` if missing |
| `EXIT` | Quit. | |

Anything else returns `ERR invalid command`.

## Building

Requires CMake 3.20+ and a C++20 compiler. GoogleTest is downloaded automatically on first configure.

```
cmake -S . -B build
cmake --build build
./build/kvstore
```

Run the tests:

```
ctest --test-dir build --output-on-failure
```

In Visual Studio, open the folder as a CMake project, then use Build All and Test Explorer.

## Design

```
main.cpp ──► parse() ──► Database::execute()
                              │
                  ┌───────────┴───────────┐
                  ▼                       ▼
           WriteAheadLog              KVStore
        (append + replay)       (hash map + LRU list)
```

**`parse`** turns a line of text into a `Command` struct. Malformed input becomes `Invalid` instead of throwing.

**`KVStore`** holds the data. It combines a `std::unordered_map` with a `std::list` ordered by recency. Each map entry stores an iterator to its own list node, so `get`, `set`, `del`, and eviction are all O(1). Using a key moves its node to the front with `std::list::splice`, which relinks the node without invalidating the stored iterator. When the store is full, the node at the back is evicted.

**`WriteAheadLog`** appends every write to a text file and flushes it, then replays the file on startup to rebuild state. The log uses the same format as the commands themselves, so replay reuses `parse`:

```
SET name derrick
SET greeting hello world
DEL name
```

**`Database`** coordinates the two. Every write goes to the log first and then to the store, so the log is always the source of truth.

## Tradeoffs and limitations

These are deliberate simplifications, documented so the behavior is predictable.

- **Eviction order is not preserved across restarts.** `GET` changes recency but is not logged. Replay only sees writes, so after a restart the store holds the most recently *written* keys, which may differ from the most recently *used* keys before shutdown.
- **The log grows forever.** Overwritten, deleted, and evicted keys stay in the file. Log compaction (rewriting it with only the live keys) would fix this.
- **Flush is not fsync.** Each write is flushed to the operating system, which survives a program crash but not necessarily a power loss.
- **Partial writes are skipped.** If the program dies midway through writing a line, replay parses the incomplete line as invalid and ignores it rather than failing.
- **Keys cannot contain spaces, and values cannot contain newlines,** because the protocol and log are both line and whitespace based.
- **Single threaded.** There is no locking.
- **Capacity must be at least 1.** A capacity of zero is not supported.

## Testing

Tests live in `tests/` and cover each layer:

- `kv_store_test.cpp`: basic operations and LRU behavior, including recency on read, overwrites, and deletes freeing space
- `command_test.cpp`: parsing valid and malformed input
- `wal_test.cpp`: replaying sets, deletes, overwrites, and values with spaces
- `database_test.cpp`: end to end behavior, including data surviving a restart

File based tests use a fixture that creates and cleans up a temporary log for each test.

## Project structure

```
kvstore/
├── CMakeLists.txt
├── src/
│   ├── main.cpp        REPL loop
│   ├── command.h/.cpp  command parsing
│   ├── kv_store.h/.cpp in memory store with LRU eviction
│   ├── wal.h/.cpp      write ahead log
│   └── database.h/.cpp coordinates log and store
└── tests/
```

## Possible next steps

- Key expiration (TTL), e.g. `SET key value EX 60`
- Log compaction
- Serving commands over TCP