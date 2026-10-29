# Systems Programming & Computer Architecture

Academic exercises in C, Bash, and Python exploring Linux process management, POSIX APIs, file operations, scripting, and cache-memory fundamentals.

## Contents

### Linux File Synchronization Utility — `linux/file-sync/`
`file_sync.c` synchronizes files between directories. Demonstrates `fork()`, `execl()`, `wait()`, `open()`, `dup2()`, directory traversal, file metadata checks, and dynamic memory management.

### Cache Memory Simulator — `computer-architecture/cache-simulator/`
`cache.c` and `cache.h` implement a simplified set-associative cache read operation, extracting tag, set index, and block offset using bitwise shifts and masks. The header was reconstructed from the C source; this is an educational implementation, not a complete cache simulator. Cache-line replacement and allocation cleanup have limitations.

### Chess & Bash Scripting — `bash-scripting/`
- `split_pgn.sh`: splits a PGN file into separate games.
- `chess_sim.sh`: interactive chess-board simulation using Bash arrays and functions.
- `parse_moves.py`: converts PGN moves into UCI notation using the `python-chess` package.

To run the chess simulation, install the Python dependency and launch it **from the `bash-scripting` directory** (the Bash script calls `parse_moves.py` by relative path):

```bash
python3 -m pip install -r requirements.txt
bash chess_sim.sh path/to/game.pgn
```

## Basic build checks

```bash
gcc -std=c11 -Wall -Wextra -c computer-architecture/cache-simulator/cache.c -o /tmp/cache.o
gcc -std=c11 -Wall -Wextra linux/file-sync/file_sync.c -o /tmp/file_sync
bash -n bash-scripting/chess_sim.sh bash-scripting/split_pgn.sh
```

These commands check compilation and shell syntax; they do not constitute functional tests. `cache.c` has no `main()` entry point and is intended to be linked with a test driver.

## Notes

The repository contains coursework exercises, not production-ready utilities. Check your course's publication policy before making solutions public. Source code is otherwise retained as provided, with the cache header reconstructed from the accompanying implementation.
