/*
 * Copyright (c) 2022 Christian Mazakas
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef LESS_VECTOR_HPP
#define LESS_VECTOR_HPP

namespace less {

using unsigned_long_type = decltype(sizeof(char));

namespace detail {

struct new_tag_t {};

inline constexpr new_tag_t const new_tag;
}    // namespace detail
}    // namespace less

#if __clang__
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

template <class T>
struct vector {
 public:
  using size_type       = unsigned_long_type;
  using value_type      = T;
  using pointer         = T*;
  using const_pointer   = T const*;
  using reference       = T&;
  using const_reference = T const&;
  using iterator        = pointer;
  using const_iterator  = const_pointer;

 private:
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

      auto guard = detail::alloc_destroyer<value_type>{0u, p};

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
#if defined(_LIBCPP_ITERATOR) || defined(_GLIBCXX_ITERATOR) || defined(_ITERATOR_)
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

#if defined(_LIBCPP_INITIALIZER_LIST) || defined(_INITIALIZER_LIST) || defined(_INITIALIZER_LIST_)
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

  auto data() noexcept -> T*
  {
    return p_;
  }

  auto data() const noexcept -> T const*
  {
    return p_;
  }

  auto size() const noexcept -> size_type
  {
    return size_;
  }

  auto capacity() const noexcept -> size_type
  {
    return capacity_;
  }

  auto operator[](size_type const idx) noexcept -> reference
  {
    return p_[idx];
  }

  auto operator[](size_type const idx) const noexcept -> const_reference
  {
    return p_[idx];
  }

  auto begin() noexcept -> iterator
  {
    return p_;
  }

  auto begin() const noexcept -> const_iterator
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

  bool empty() const noexcept
  {
    return size_ == 0u;
  }

  void clear() noexcept
  {
    if (!p_) { return; }

    {
      auto guard = detail::alloc_destroyer<value_type>{size_, p_};
      (void)guard;
    }
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

      auto guard = detail::alloc_destroyer<value_type>{0u, p};
      // TODO: conditionally invoke `less::detail::move()` here
      for (auto& i = guard.size; i < size_; ++i) {
        new (p + i, detail::new_tag) T(p_[i]);
      }
      guard.size = 0u;
    }
    catch (...) {
      this->deallocate(p);
      throw;
    }

    this->clear();
    this->deallocate(p_);

    p_        = p;
    capacity_ = new_capacity;
    ++size_;
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

        // TODO: conditionally invoke `less::detail::move()` here
        for (auto& i = guard2.size; i < size_; ++i) {
          new (p + i, detail::new_tag) T(p_[i]);
        }

        guard1.size = 0u;
        guard2.size = 0u;
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

#endif    // LESS_VECTOR_HPP
