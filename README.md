# Less

An `#include`-less implementation of `std::vector` written largely for fun.

API-compatible drop-in for `std::vector`, sans `Allocator`-based features.

Implementation differences:
* No `Allocator` support (instead we only use `::operator new` & `::operator delete`)
*  `std::initializer_list` constructor only supported with `#include <initializer_list>`
* Use `#include <iterator>` for more efficient construction from iterator pairs (otherwise a fallback implementation is used)
* Requires C++17 and up
* no `bool` specialization
* supports default initialization of elements via `less::default_init` tag constructor
* `less::with_capacity` tag constructor for constructing a `less:vector` with a specified capacity

## Examples

### Reading in a file using `default_init`

```cpp
#include <less/vector.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string_view>

int main() {
  auto path = std::filesystem::path("example.txt");
  auto size = std::filesystem::file_size(path);

  // avoids the cost of zeroing out the memory here
  auto buf = less::vector<char>(less::default_init, size);

  {
    auto ifs = std::ifstream(path);
    ifs.read(buf.data(), buf.size());
  }

  auto strv = std::string_view(buf.data(), buf.size());
  std::cout << strv << std::endl;
}
```
