# LRU Cache Simulator in C

A terminal-based Least Recently Used (LRU) cache simulator implemented in C using a **hash table** and a **doubly linked list**.

## Features

- Insert or update integer key/value pairs
- Access keys and update their recency
- Automatically evict the least recently used entry when full
- Display cache contents from most recently used to least recently used
- Track cache hits, misses, and evictions
- Validate basic menu input and release allocated memory on exit

## Data structures

- **Hash table:** Finds a key quickly on average.
- **Doubly linked list:** Maintains the recency order. The front is most recently used; the tail is least recently used.

## Build and run

Requires a C compiler such as GCC.

```bash
gcc -std=c11 -Wall -Wextra -pedantic main.c -o lru_cache
./lru_cache
```

On Windows, run `lru_cache.exe` after compiling.

## Example

Set capacity to `3`, then insert these pairs:

- key `10`, value `100`
- key `20`, value `200`
- key `30`, value `300`

Access key `10`, then insert key `40`, value `400`. Key `20` should be evicted because it is the least recently used entry.

## Time complexity

| Operation | Average time |
|---|---:|
| Find key | O(1) |
| Insert/update key | O(1) |
| Access key | O(1) |
| Evict least-recently-used key | O(1) |
| Display cache | O(n) |

Hash table operations are average-case O(1); collisions can make lookup slower in the worst case.

## Project structure

```text
lru-cache-simulator/
├── main.c
├── README.md
└── .gitignore
```

## Future improvements

- Store string keys and values
- Add file-based save/load support
- Add automated tests
- Display a step-by-step eviction history

## License

This project is available under the MIT License. See `LICENSE` if you choose to add one.
