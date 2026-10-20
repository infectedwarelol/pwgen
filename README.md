# pwgen

Small command-line password generator written in C++20. No runtime dependencies.

## Build

Requires CMake 3.20+ and a C++20 compiler (GCC 13+ or Clang 18+).

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Tests use doctest, fetched automatically by CMake. Skip them with `-DPWGEN_BUILD_TESTS=OFF`.

## Usage

```
$ ./build/pwgen
1fpXDGPmiWYVemVj0JgN

$ ./build/pwgen -l 16 -c 3 --symbols --require-each
uIv:SjNS~1?N$$O!
iyWS@by8TC*&iX2=
bX_&;YE5rkvEb1v5

$ ./build/pwgen -l 12 --exclude-ambiguous --entropy
msVQMTBqRdZq
entropy: 69.7 bits (strong)

$ ./build/pwgen --no-lower --no-upper -l 6 --entropy
399697
entropy: 19.9 bits (weak)

$ ./build/pwgen -l 2 --symbols --require-each
pwgen: length 2 is smaller than the 4 required character sets
Try 'pwgen --help' for usage.

$ ./build/pwgen --length abc
pwgen: invalid number for --length: 'abc'
Try 'pwgen --help' for usage.
```

| Flag | Meaning |
| --- | --- |
| `-l, --length N` | password length, default 20, range 1-1024 |
| `-c, --count N` | number of passwords, default 1 |
| `--no-lower`, `--no-upper`, `--no-digits` | disable a character set |
| `--symbols` | enable symbols `!@#$%^&*()-_=+[]{};:,.<>/?~` |
| `--exclude-ambiguous` | remove `0 O o 1 l I` |
| `--exclude CHARS` | remove the given characters |
| `--require-each` | at least one character from every enabled set |
| `--entropy` | print entropy and a strength label |
| `-h, --help` | show help |

Errors go to stderr with exit code 2.

## Entropy

Entropy is `length * log2(pool size)`, where pool size is the number of distinct characters left after
all sets and exclusions are applied. A 20-character password over 62 characters (default) gives
`20 * log2(62) = 119.1` bits.

| Bits | Label |
| --- | --- |
| < 40 | weak |
| 40 - 59.9 | fair |
| 60 - 79.9 | strong |
| >= 80 | very strong |

`--require-each` slightly reduces the real entropy because it constrains the output; the printed value
is an upper bound.

## Security notes

- Characters are drawn from `std::random_device` with `std::uniform_int_distribution`, so there is no
  modulo bias. On Linux with libstdc++ and libc++, `std::random_device` reads from the OS CSPRNG
  (`getrandom` / `/dev/urandom`). The standard does not guarantee this on every platform; verify your
  toolchain before relying on it elsewhere.
- Passwords are printed to stdout. Avoid running it where terminal output is logged or shared, and be
  careful with shell history when piping.
- Memory holding generated passwords is not explicitly wiped.
