# kvstore

A small persistent key value store written in C++, inspired by Redis. It runs as an interactive command line program, keeps data in memory, persists every write to an append only log, evicts the least recently used key when it reaches capacity, and supports expiring keys.

I built this to learn modern C++ from the ground up: STL containers, iterators, RAII, `<chrono>`, file I/O, CMake, and unit testing with GoogleTest.

## Example

```
$ ./kvstore
> SET name derrick
OK
> SET greeting hello world
OK
> GET greeting
hello world
> SET session abc123
OK
> EXPIRE session 60
OK
> TTL session
57
> DEL name
OK
> GET name
(nil)
> EXIT
```

Restart the program and `greeting` is still there, and `session` still expires on its original schedule.

## Commands

| Command | Description | Response |
|---|---|---|
| `SET key value` | Store a value. Values may contain spaces. Clears any expiration on the key. | `OK` |
| `GET key` | Read a value. | the value, or `(nil)` |
| `DEL key` | Delete a key. | `OK`, or `(nil)` if missing |
| `EXPIRE key seconds` | Expire a key after the given number of seconds. | `OK`, or `(nil)` if missing |
| `TTL key` | Seconds until a key expires. | seconds left, `-1` if no expiration, `-2` if missing |
| `COMPACT` | Rewrite the log to contain only live keys. | `OK` |
| `EXIT` | Quit. | |

Anything else returns `ERR invalid command`. The `TTL` return values follow the Redis convention.

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
   (append, replay, compact)   (hash map + LRU list + deadlines)
```

**`parse`** turns a line of text into a `Command` struct. Malformed input becomes `Invalid` instead of throwing.

**`KVStore`** holds the data. It combines a `std::unordered_map` with a `std::list` ordered by recency. Each map entry stores an iterator to its own list node, so `get`, `set`, `del`, and eviction are all O(1). Using a key moves its node to the front with `std::list::splice`, which relinks the node without invalidating the stored iterator. When the store is full, the node at the back is evicted.

**`WriteAheadLog`** appends every write to a text file and flushes it, then replays the file on startup to rebuild state. The log uses the same format as the commands themselves, so replay reuses `parse`:

```
SET name derrick
SET session abc123
EXPIREAT session 1792540800000
DEL name
```

**`Database`** coordinates the two. Every write goes to the log first and then to the store, so the log is always the source of truth.

### Expiration

Each entry can carry an optional deadline. Expiration is **lazy**: nothing runs in the background, and a key is checked and removed when it is next accessed. Every lookup goes through a single `findLive` helper, so expired keys behave exactly like missing ones everywhere.

`EXPIRE` takes a duration, but the log records an **absolute deadline** (`EXPIREAT`, in milliseconds since the Unix epoch). Logging the duration would restart the countdown on every replay, letting keys outlive their expiration across restarts. Deadlines use `std::chrono::system_clock` because, unlike `steady_clock`, its values stay meaningful after the program restarts.

The clock is injected into `KVStore` as a function. Production uses the real clock, and tests pass a fake clock they can move forward instantly, so expiration is tested without sleeping.

### Compaction

`COMPACT` rewrites the log with one `SET` per live key (plus an `EXPIREAT` for keys with a deadline). The new log is written to a temporary file and then renamed over the original, so a crash during compaction leaves the old log intact. The log stream is closed before the rename and reopened after, because Windows will not replace a file that is still open.

Keys are written from least recently used to most recently used. Replay pushes each key to the front of the LRU list, so this order rebuilds the exact recency order from before compaction.

## Tradeoffs and limitations

These are deliberate simplifications, documented so the behavior is predictable.

- **Recency order survives compaction but not an ordinary restart.** `GET` changes recency but is not logged, so replaying an uncompacted log restores the most recently *written* keys rather than the most recently *used* ones.
- **Compaction is manual.** The log grows until `COMPACT` is run. Real databases trigger compaction automatically once the log passes a size threshold.
- **Expired keys hold space until accessed.** Lazy expiration means an expired key still counts toward capacity until it is read, written, or evicted. A periodic background sweep would fix this.
- **Expiration follows the system clock.** Changing the machine's time can make keys expire early or late.
- **Flush is not fsync.** Each write is flushed to the operating system, which survives a program crash but not necessarily a power loss.
- **Malformed log lines are skipped silently.** This lets replay recover from a partial line left by a crash, but it also means a bug that writes bad lines would go unnoticed. Counting and reporting skipped lines would make that visible.
- **Keys cannot contain spaces, and values cannot contain newlines,** because the protocol and log are both line and whitespace based.
- **Single threaded.** There is no locking.
- **Capacity must be at least 1.** A capacity of zero is not supported.

## Testing

Tests live in `tests/` and cover each layer:

- `kv_store_test.cpp`: basic operations; LRU behavior, including recency on read, overwrites, and deletes freeing space; and expiration, using a fake clock
- `command_test.cpp`: parsing valid and malformed input
- `wal_test.cpp`: replaying sets, deletes, overwrites, and deadlines; compaction keeping only live keys, preserving recency order and deadlines, and accepting writes afterward
- `database_test.cpp`: end to end behavior, including data and expirations surviving a restart

File based tests use a fixture that creates and cleans up a temporary log for each test.

## Project structure

```
kvstore/
├── CMakeLists.txt
├── src/
│   ├── main.cpp        REPL loop
│   ├── command.h/.cpp  command parsing
│   ├── kv_store.h/.cpp in memory store with LRU eviction and expiration
│   ├── wal.h/.cpp      write ahead log with replay and compaction
│   └── database.h/.cpp coordinates log and store
└── tests/
```

## Possible next steps

- Serving commands over TCP
- Automatic compaction once the log passes a size threshold
- A background sweep to remove expired keys
- Reporting malformed lines found during replay
