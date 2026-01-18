/**
 *
 *  @file Errors.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_ERRORS_HPP
#define VIX_ERRORS_HPP

#include <string>
#include <stdexcept>

namespace vix::orm
{
  struct DBError : std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };

  struct TxnError : DBError
  {
    using DBError::DBError;
  };

  struct NotFound : DBError
  {
    using DBError::DBError;
  };
} // namespace Vix::orm

#endif // VIX_ERRORS_HPP
