/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef LESS_VECTOR_HPP
#define LESS_VECTOR_HPP

#if defined(_LIBCPP_INITIALIZER_LIST) || defined(_INITIALIZER_LIST) || defined(_INITIALIZER_LIST_)
#define LESS_HAS_INITIALIZER_LIST
#endif

#if defined(_LIBCPP_ITERATOR) || defined(_GLIBCXX_ITERATOR) || defined(_ITERATOR_)
#define LESS_HAS_ITERATOR
#endif

namespace less {

namespace detail {
using long_type_pointer_impl = char*;
}

using unsigned_long_type = decltype(sizeof(char));
using long_type          = decltype(detail::long_type_pointer_impl() - detail::long_type_pointer_impl());

namespace detail {

struct new_tag_t {};

inline constexpr new_tag_t const new_tag;
}    // namespace detail
}    // namespace less
#if __clang__
// technically, using our _own_ forward declaration here is UB
// but maybe it won't result in anything bad happening so we just do it anyway
//
namespace std {
enum class align_val_t : decltype(sizeof(char));
}
#endif

void* operator new(less::unsigned_long_type, void* p, less::detail::new_tag_t) noexcept
{
  return p;
}

// keep this around to make msvc happy because of our custom placement new above
//
void operator delete(void*, void*, less::detail::new_tag_t) noexcept
{
}

namespace less {
namespace detail {

// <type_traits> polyfills
//
template <class T>
struct type_identity {
  using type = T;
};

template <class T>
auto try_add_rvalue_reference(int) -> type_identity<T&&>;

template <class T>
auto try_add_rvalue_reference(...) -> type_identity<T>;

template <class T>
struct add_rvalue_reference : public decltype(try_add_rvalue_reference<T>(0)) {
};

template <class T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

template <class T>
inline constexpr bool const always_false = false;

template <class T>
auto declval() noexcept -> add_rvalue_reference_t<T>
{
  static_assert(always_false<T>);
}

struct true_type {
  constexpr static bool const value = true;
};

struct false_type {
  constexpr static bool const value = false;
};

template <class T, class... Args, class = decltype(T(declval<Args>()...))>
auto try_construct(int) -> true_type;

template <class T, class...>
auto try_construct(...) -> false_type;

template <class T, class... Args>
struct is_constructible : public decltype(try_construct<T, Args...>(0)) {
};

template <class T, bool B, class...>
struct is_nothrow_constructible_impl : public false_type {
};

template <class T, class... Args>
struct is_nothrow_constructible_impl<T, true, Args...> {
  constexpr static bool const value = noexcept(T(declval<Args>()...));
};

template <class T, class... Args>
struct is_nothrow_constructible
    : public is_nothrow_constructible_impl<T, is_constructible<T, Args...>::value, Args...> {
};

template <class T>
struct is_nothrow_move_constructible : public is_nothrow_constructible<T, add_rvalue_reference_t<T>> {
};

template <class T>
inline constexpr bool const is_nothrow_move_constructible_v = is_nothrow_move_constructible<T>::value;

template <class T>
struct remove_reference {
  typedef T type;
};
template <class T>
struct remove_reference<T&> {
  typedef T type;
};
template <class T>
struct remove_reference<T&&> {
  typedef T type;
};

template <class T>
using remove_reference_t = typename remove_reference<T>::type;

template <class T, class U>
struct is_same {
  constexpr static bool const value = false;
};

template <class T>
struct is_same<T, T> {
  constexpr static bool const value = true;
};

template <class T, class U>
inline constexpr bool const is_same_v = is_same<T, U>::value;

template <typename T>
constexpr auto move(T&& t) noexcept -> remove_reference_t<T>&&
{
  return static_cast<typename remove_reference<T>::type&&>(t);
}

template <class T>
struct alloc_destroyer {
  unsigned_long_type size = 0u;

  T* p = nullptr;

  ~alloc_destroyer()
  {
    if (size == 0) { return; }

    do {
      --size;
      (p + size)->~T();
    } while (size != 0);
  }
};

}    // namespace detail

struct default_init_t {};
inline constexpr default_init_t default_init;

struct with_capacity_t {};
inline constexpr with_capacity_t with_capacity;

struct out_of_range {};

template <class T>
struct vector {
 public:
  using value_type      = T;
  using size_type       = unsigned_long_type;
  using difference_type = long_type;
  using reference       = T&;
  using const_reference = T const&;
  using pointer         = T*;
  using const_pointer   = T const*;
  using iterator        = pointer;
  using const_iterator  = const_pointer;

 private:
  using alloc_destroyer = detail::alloc_destroyer<value_type>;

  pointer   p_        = nullptr;
  size_type size_     = 0u;
  size_type capacity_ = 0u;

  static auto allocate(size_type capacity) -> pointer
  {
    auto const p =
        static_cast<pointer>(::operator new (capacity * sizeof(value_type), std::align_val_t{alignof(value_type)}));

    return p;
  }

  static void deallocate(pointer p)
  {
    ::operator delete (p, std::align_val_t{alignof(value_type)});
  }

  template <class F>
  void construct(size_type size, size_type capacity, F f)
  {
    auto const p = this->allocate(capacity);

    try {
      constexpr size_type const stride = 32;

      auto guard = alloc_destroyer{0u, p};

      auto& i = guard.size;
      for (; (i + stride) < size; i += stride) {
        for (auto j = 0u; j < stride; ++j) {
          f(p + i + j, i);
        }
      }

      for (; i < size; ++i) {
        f(p + i, i);
      }

      guard.size = 0u;

      p_        = p;
      size_     = size;
      capacity_ = capacity;
    }
    catch (...) {
      this->deallocate(p);
      throw;
    }
  }

 public:
  vector() noexcept
  {
  }

  vector(default_init_t, size_type const size)
  {
    this->construct(size, size, [](auto p, auto) { new (p, detail::new_tag) T; });
  }

  vector(size_type size)
  {
    this->construct(size, size, [](auto p, auto) { new (p, detail::new_tag) T(); });
  }

  vector(with_capacity_t, size_type const capacity)
  {
    this->construct(0u, capacity, [](auto, auto) {});
  }

  vector(size_type size, T const& value)
  {
    this->construct(size, size, [&](auto p, auto) { new (p, detail::new_tag) T(value); });
  }

  vector(vector const& rhs)
  {
    auto const size = rhs.size();
    this->construct(size, size, [&](auto p, auto idx) { new (p, detail::new_tag) T(rhs[idx]); });
  }

  vector(vector&& rhs) noexcept
  {
    p_        = rhs.p_;
    size_     = rhs.size_;
    capacity_ = rhs.capacity_;

    rhs.p_        = nullptr;
    rhs.size_     = 0u;
    rhs.capacity_ = 0u;
  }

  template <class Iterator>
  vector(Iterator begin, Iterator end)
  {
#ifdef LESS_HAS_ITERATOR
    using category = typename std::iterator_traits<Iterator>::iterator_category;

    if constexpr (detail::is_same_v<category, std::random_access_iterator_tag>) {
      size_type size = (end - begin);
      this->construct(size, size, [&](auto p, auto idx) { new (p, detail::new_tag) T(begin[idx]); });
    }
    else {
      auto v = vector();
      while (begin != end) {
        v.push_back(*begin);
        ++begin;
      }

      *this = detail::move(v);
    }
#else
    auto v = vector();
    while (begin != end) {
      v.push_back(*begin);
      ++begin;
    }

    *this = detail::move(v);
#endif
  }

#ifdef LESS_HAS_INITIALIZER_LIST
  vector(std::initializer_list<T> list)
  {
    auto const size = list.size();
    auto const pos  = list.begin();

    this->construct(size, size, [&](auto p, auto idx) { new (p, detail::new_tag) T(pos[idx]); });
  }
#endif

  ~vector()
  {
    this->clear();
    this->deallocate(p_);
  }

  auto operator=(vector&& rhs) noexcept -> vector&
  {
    p_        = rhs.p_;
    size_     = rhs.size_;
    capacity_ = rhs.capacity_;

    rhs.p_        = nullptr;
    rhs.size_     = 0u;
    rhs.capacity_ = 0u;
    return *this;
  }

  // Element access

  auto at(size_type const pos) -> reference
  {
    if (pos >= size_) { throw out_of_range{}; }

    return p_[pos];
  }

  auto at(size_type const pos) const -> const_reference
  {
    if (pos >= size_) { throw out_of_range{}; }

    return p_[pos];
  }

  auto operator[](size_type const pos) -> reference
  {
    return p_[pos];
  }

  auto operator[](size_type const pos) const -> const_reference
  {
    return p_[pos];
  }

  auto front() -> reference
  {
    return *this->begin();
  }

  auto front() const -> const_reference
  {
    return *this->begin();
  }

  auto back() -> reference
  {
    return *(--this->end());
  }

  auto back() const -> const_reference
  {
    return *(--this->end());
  }

  auto data() noexcept -> T*
  {
    return p_;
  }

  auto data() const noexcept -> T const*
  {
    return p_;
  }

  // Iterators

  auto begin() noexcept -> iterator
  {
    return p_;
  }

  auto begin() const noexcept -> const_iterator
  {
    return p_;
  }

  auto cbegin() const noexcept -> const_iterator
  {
    return p_;
  }

  auto end() noexcept -> iterator
  {
    return p_ + size_;
  }

  auto end() const noexcept -> const_iterator
  {
    return p_ + size_;
  }

  auto cend() const noexcept -> const_iterator
  {
    return p_ + size_;
  }

  // Capacity

  bool empty() const noexcept
  {
    return size_ == 0u;
  }

  auto size() const noexcept -> size_type
  {
    return size_;
  }

  auto max_size() const noexcept -> size_type
  {
#ifdef __PTRDIFF_MAX__
    return static_cast<size_type>(__PTRDIFF_MAX__);
#elif defined(_MSC_VER)
    return ~(size_type{1} << (_INTEGRAL_MAX_BITS - 1));
#else
#error "Unsupported platform!"
#endif
  }

  void reserve(size_type new_cap)
  {
    if (new_cap <= capacity_) { return; }

    auto const p = static_cast<pointer>(this->allocate(new_cap));
    if constexpr (detail::is_nothrow_move_constructible_v<value_type>) {
      for (auto i = 0u; i < size_; ++i) {
        new (p + i, detail::new_tag) T(detail::move(p_[i]));
      }
    }
    else {
      try {
        auto guard = alloc_destroyer{0u, p};
        for (auto& i = guard.size; i < size_; ++i) {
          new (p + i, detail::new_tag) T(p_[i]);
        }
        guard.size = 0u;
      }
      catch (...) {
        this->deallocate(p);
        throw;
      }
    }

    p_        = p;
    capacity_ = new_cap;
  }

  auto capacity() const noexcept -> size_type
  {
    return capacity_;
  }

  // Modifiers

  void clear() noexcept
  {
    if (!p_) { return; }

    {
      auto guard = detail::alloc_destroyer<value_type>{size_, p_};
      (void)guard;
    }

    size_ = 0;
  }

  void push_back(T const& value)
  {
    // TODO: add some impl of BOOST_LIKELY here
    if (size_ < capacity_) {
      new (p_ + size_, detail::new_tag) T(value);
      ++size_;
      return;
    }

    auto const new_capacity = (capacity_ == 0 ? 16 : 2 * capacity_);

    auto const p = this->allocate(new_capacity);

    try {
      new (p + size_, detail::new_tag) T(value);

      if constexpr (detail::is_nothrow_move_constructible_v<value_type>) {
        for (auto i = 0u; i < size_; ++i) {
          new (p + i, detail::new_tag) T(detail::move(p_[i]));
        }
      }
      else {
        auto guard1 = alloc_destroyer{0, p};
        auto guard2 = alloc_destroyer{1, p_ + size_};
        for (auto& i = guard1.size; i < size_; ++i) {
          new (p + i, detail::new_tag) T(p_[i]);
        }
        guard1.size = 0;
        guard2.size = 0;
      }
    }
    catch (...) {
      this->deallocate(p);
      throw;
    }

    auto const old_size = size_;
    this->clear();
    this->deallocate(p_);

    p_        = p;
    capacity_ = new_capacity;
    size_     = old_size + 1;
  }

  template <class F>
  void resize_and_overwrite(size_type n, F f)
  {
    if (n <= size_) {
      auto erase_begin = p_ + f(p_, n);
      auto erase_end   = p_ + size_;

      for (; erase_begin < erase_end; ++erase_begin) {
        erase_begin->~T();
      }
      size_ = n;
      return;
    }

    if (n > capacity_) {
      auto const p = this->allocate(n);

      try {
        // we get better exception guarantees if `new T;` throws by doing this first
        //
        auto guard2 = detail::alloc_destroyer<value_type>{0u, p};
        auto guard1 = detail::alloc_destroyer<value_type>{0u, p + size_};
        for (auto& i = guard1.size; i < (n - size_); ++i) {
          new (p + i + size_, detail::new_tag) T;
        }

        if constexpr (detail::is_nothrow_move_constructible_v<value_type>) {
          for (auto i = 0u; i < size_; ++i) {
            new (p + i, detail::new_tag) T(detail::move(p_[i]));
          }
        }
        else {
          for (auto& i = guard2.size; i < size_; ++i) {
            new (p + i, detail::new_tag) T(p_[i]);
          }
          guard2.size = 0u;
        }

        guard1.size = 0u;
      }
      catch (...) {
        this->deallocate(p);
        throw;
      }

      this->clear();
      this->deallocate(p_);

      p_        = p;
      capacity_ = n;
    }
    else {
      auto guard = detail::alloc_destroyer<value_type>{0u, p_ + size_};
      for (auto& i = guard.size; i < n; ++i) {
        new (p_ + size_ + i, detail::new_tag) T;
      }
      guard.size = 0u;
    }

    size_ = n;

    auto new_len     = f(p_, n);
    auto erase_begin = p_ + new_len;
    auto erase_end   = p_ + n;

    for (; erase_begin < erase_end; ++erase_begin) {
      erase_begin->~T();
    }

    size_ -= (n - new_len);
  }
};

}    // namespace less

#ifdef LESS_HAS_INITIALIZER_LIST
#undef LESS_HAS_INITIALIZER_LIST
#endif

#ifdef LESS_HAS_ITERATOR
#undef LESS_HAS_ITERATOR
#endif

#endif    // LESS_VECTOR_HPP
