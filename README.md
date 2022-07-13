# Less

An `#include`-less implementation of `std::vector` written largely for fun.

API-compatible drop-in for `std::vector`, sans `Allocator`-based features.

Implementation differences:
* No `Allocator` support (instead we only use `::operator new` & `::operator delete`)
*  `std::initializer_list` constructor only supported with `#include <initializer_list>`
* Use `#include <iterator>` for more efficient construction from iterator pairs (otherwise a fallback implementation is used)
* Requires C++17 and up

```c++
#include <less/vector.hpp>
#include <initializer_list>

int main() {
  less::vector<int> v{1, 2, 3, 4, 5};
  return v.size() - 5;
}
```
