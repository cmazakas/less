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
* implements experimental `resize_and_overwrite()` API

## Examples

### Reading in a file using default initialization

```cpp
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string_view>

// this include should always be at the bottom
#include <less/vector.hpp>

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

### Using resize_and_overwrite()

`resize_and_overwrite` is a hypothetical API for `std::basic_string` but we can
easily extend it to `vector` as well. This permits resizing the container as
well as efficiently overwriting it. When resizing, the container uses default
initialization so while reading elements is potentially UB, writing to them is
not.

This function is roughly equivalent to:
```cpp
// given user-provided `f`
v.resize(default_init, new_size);
auto actual_size = f(v.data(), new_size);
v.erase(v.begin() + actual_size, v.end());
```

If an exception is thrown during resizing, this function has no effect.

If an exception is thrown from the user-provided Callable, then the effects of
the resizing persist.

```cpp
#include <cassert>

#include <less/vector.hpp>

int main() {
  auto const size = 512u;

  less::vector<unsigned> v(less::with_capacity, size);
  for (auto i = 0u; i < size; ++i) {
    v.push_back(i);
  }

  v.resize_and_overwrite(2 * size, [=](unsigned* p, unsigned n) {
    // n here is the same thing as `2 * size`
    // by capturing `size` from above, we ensure that we're _appending_ to the existing `vector`
    // if we wanted to, we could easily overwrite the contents of the entire container
    for (auto i = size; i < n; ++i) {
      p[i] = i;
    }
    return n;
  });

  for (auto i = 0u; i < 2 * size; ++i) {
    assert(v[i] == i);
  }
}
```
