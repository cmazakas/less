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
namespace detail {

struct new_tag_t {};

inline constexpr new_tag_t const new_tag;
}    // namespace detail
}    // namespace less

void* operator new(unsigned long, void* p, less::detail::new_tag_t) noexcept
{
  return p;
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
  unsigned long long size = 0u;

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
  using size_type       = unsigned long;
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

  template <class F>
  void construct(size_type size, size_type capacity, F f)
  {
    auto const p = static_cast<pointer>(::operator new(capacity * sizeof(value_type)));

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
      ::operator delete(p);
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
    ::operator delete(p_);
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

    auto const p = static_cast<pointer>(::operator new(new_capacity * sizeof(value_type)));

    try {
      auto guard = detail::alloc_destroyer<value_type>{0u, p};
      for (auto& i = guard.size; i < size_; ++i) {
        new (p + i, detail::new_tag) T(p_[i]);
      }
      new (p + size_, detail::new_tag) T(value);

      guard.size = 0u;
    }
    catch (...) {
      ::operator delete(p);
      throw;
    }

    this->clear();
    ::operator delete(p_);

    p_        = p;
    capacity_ = new_capacity;
    ++size_;
  }
};

}    // namespace less

#endif    // LESS_VECTOR_HPP
