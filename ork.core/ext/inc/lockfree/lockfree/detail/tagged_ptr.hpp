//  tagged pointer, for aba prevention
//
//  Copyright (C) 2008 Tim Blechmann
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

#ifndef BOOST_LOCKFREE_TAGGED_PTR_HPP_INCLUDED
#define BOOST_LOCKFREE_TAGGED_PTR_HPP_INCLUDED

#include <lockfree/lockfree/detail/prefix.hpp>

#ifndef BOOST_LOCKFREE_PTR_COMPRESSION
#include <lockfree/lockfree/detail/tagged_ptr_dcas.hpp>
#else
#include <lockfree/lockfree/detail/tagged_ptr_ptrcompression.hpp>
#endif

#endif /* BOOST_LOCKFREE_TAGGED_PTR_HPP_INCLUDED */