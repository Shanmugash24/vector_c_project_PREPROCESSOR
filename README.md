# C Preprocessor

A lightweight C preprocessor written from scratch in C. It reads a `.c` file, handles comments, `#include`, and macros, and writes the result to a `.i` file, similar to `gcc -E`.

## Features

- **Comment removal**: strips `//` and `/* ... */` comments (including multi-line blocks) and keeps comment-like text inside string and char literals
- **File inclusion**: `#include "file.h"` is searched relative to the current file, then the working directory; `<file.h>` system headers are passed through unchanged
- **Nested includes**: up to 32 levels deep
- **Macros**: `#define` for object-like (`#define PI 3.14`) and function-like (`#define SQ(x) ((x)*(x))`) macros, and `#undef`
- **Recursive expansion**: macros that use other macros are expanded (depth limit 10)

## Project Structure

| File | Purpose |
|------|---------|
| `main.c` | Entry point: handles arguments and builds the output filename |
| `comment_removal.c` | Comment stripping logic |
| `file_inclusion.c` | `#include` handling and the line-by-line processing loop |
| `macro_handler.c` | `#define`, `#undef`, and macro expansion |
| `preprocessor.h` | Shared structs, limits, and function declarations |
| `Makefile` | Build script |

## Build

```bash
make
```

Requires `gcc` (or any C11 compiler). To remove build files:

```bash
make clean
```

## Usage

```bash
./mypreprocessor <input.c>
```

The output is written next to the input file with a `.i` extension (`input.c` becomes `input.i`).

### Example

`test.c`:
```c
#include "config.h"
#define SQ(x) ((x)*(x))   // square macro

/* block comment */
int a = SQ(VALUE);
```

`config.h`:
```c
#define VALUE 5
```

Run:
```bash
./mypreprocessor test.c
cat test.i
```

Output:
```c
int a = ((5)*(5));
```

## Limits

| Item | Limit |
|------|-------|
| Max macros | 256 |
| Macro name length | 127 chars |
| Macro value length | 2047 chars |
| Max function-like macro parameters | 32 |
| Max include depth | 32 |

## Not Yet Supported

- Conditional directives (`#if`, `#ifdef`, `#ifndef`, `#else`, `#endif`)
- Stringify (`#`) and token-pasting (`##`) operators
- Command-line include paths (`-I`)
- Predefined macros (`__FILE__`, `__LINE__`)

## Author

**Shanmugash**
Final-year B.Tech ECE, SASTRA Deemed to be University

## License

MIT, free to use and modify.
