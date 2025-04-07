# MBASE SDK Release

## v0.2.4 Latest - 2025-04-06

### SDK Fix

- In simple-conversation program, `#include <locale.h>` changed to `#include <clocale>`. Former was causing compiler errors on macOS. f782666

- llama kv cache methods names are updated. 0ab7b66

- Fixed a problem in `io_file.h` file which were causing compiler errors on windows. e64c869

- Phi model template fixed. 124e9d2

### SDK Added

- File operations were only accepting wide strings, now they also accept multibyte strings. 7c67a55

- Implemented `mbase::max_element` and `mbase::min_element`. d188bc1






