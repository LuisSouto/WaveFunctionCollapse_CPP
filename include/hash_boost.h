#ifndef HASH_BOOST_H
#define HASH_BOOST_H

#include <hash_set>

// This hash_combine implementation is based on the Boost Software License 1.0.
// Copyright (c) 2005-2014 Daniel James
// Distributed under the Boost Software License, Version 1.0.
// (See license at http://www.boost.org/LICENSE_1_0.txt)
template <class T> inline void hash_combine(std::size_t &seed, const T &v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

#endif // HASH_BOOST_H