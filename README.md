# Systems Programming & Computer Architecture

A collection of academic exercises in **C, Linux/POSIX, Bash, and Python**, demonstrating process management, low-level file I/O, filesystem operations, and cache-memory concepts.

## Repository structure

| Directory | Files | Topics |
| --- | --- | --- |
| `linux/file-sync/` | `file_sync.c` | Directory traversal, file comparison, process creation, `fork()`, `execl()`, `wait()`, file descriptor redirection |
| `linux/file-processor/` | `file_processor.c` | `open()`, `read()`, `write()`, `lseek()`, dynamic memory and insertion into files |
| `linux/process-tournament/` | `tournament.c`, `gladiator.c` | Four child processes, `fork()`, `execv()`, `wait()`, configuration parsing and log files |
| `linux/directory-backup/` | `backup.c` | Recursive directory traversal, hard links, symbolic links, permission bits |
| `computer-architecture/cache-simulator/` | `cache.c`, `cache.h` | Set-associative cache representation, address decomposition, bit shifts/masks, dynamic allocation |
| `bash-scripting/` | `split_pgn.sh`, `chess_sim.sh`, `parse_moves.py` | Shell scripting, PGN splitting, interactive chess playback and PGN-to-UCI conversion |

## Building the C exercises

Run from the repository root on Linux:

```bash
gcc -std=c11 -Wall -Wextra linux/file-sync/file_sync.c -o file_sync
gcc -std=c11 -Wall -Wextra linux/file-processor/file_processor.c -o file_processor
gcc -std=c11 -Wall -Wextra linux/directory-backup/backup.c -o backup
gcc -std=c11 -Wall -Wextra linux/process-tournament/tournament.c -o tournament
gcc -std=c11 -Wall -Wextra linux/process-tournament/gladiator.c -o gladiator
gcc -std=c11 -Wall -Wextra -c computer-architecture/cache-simulator/cache.c -o cache.o
```

The cache implementation is a library-style exercise and does not define a `main()` function. The `cache.h` definitions were reconstructed from the accompanying C implementation.

## Usage examples

```bash
./file_sync source_dir destination_dir
./file_processor data.txt requests.txt
./backup source_dir backup_dir
```

The file processor expects request lines such as `R 0 4` (read bytes), `W 0 hello` (insert text), and `Q` (quit). It writes read output to `read_results.txt`.

### Process tournament

The tournament expects `./gladiator` and four configuration files named `G1.txt` through `G4.txt` in the working directory. The original configuration files are unavailable, so **no original configuration data is included**. Each file uses the format:

```text
health, attack, opponent1, opponent2, opponent3
```

For example, `100, 10, 2, 3, 4` is an **illustrative** `G1.txt` entry, not an original assignment file. Build both executables and run `./tournament` from the directory containing the executables and configuration files.

### Bash chess utilities

```bash
cd bash-scripting
python3 -m pip install -r requirements.txt
bash split_pgn.sh games.pgn output_directory
bash chess_sim.sh game.pgn
```

The chess simulator calls `parse_moves.py` by relative path, so run it from `bash-scripting/`. Its Python dependency is `python-chess`.

## Scope and limitations

These are **coursework exercises**, not production-ready applications. The cache implementation does not include a complete replacement policy or cleanup API. The C programs have not been provided with automated test suites. Check your institution's course-solution publication policy before sharing publicly.
