/**
 * Copyright 2018 Onchere Bironga
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef WHACK_IDENTIFIER_HPP
#define WHACK_IDENTIFIER_HPP

#pragma once

#include "ast.hpp"
#include "ident.hpp"
#include "overloadid.hpp"
#include "scoperes.hpp"
#include <variant>

namespace whack::ast {

using identifier_t = std::variant<OverloadID, ScopeRes, Ident>;

static identifier_t getIdentifier(const mpc_ast_t* const ast) {
  const auto tag = getInnermostAstTag(ast);
  if (tag == "overloadid") {
    return static_cast<identifier_t>(OverloadID{ast});
  }
  if (tag == "scoperes") {
    return static_cast<identifier_t>(ScopeRes{ast});
  }
  if (tag == "ident") {
    return static_cast<identifier_t>(Ident{ast});
  }
  llvm_unreachable("invalid identifier kind!");
}

} // end namespace whack::ast

#endif // WHACK_IDENTIFIER_HPP
