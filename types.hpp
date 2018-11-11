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
#ifndef WHACK_TYPES_HPP
#define WHACK_TYPES_HPP

#pragma once

#include <llvm-c/Core.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>

namespace whack {

#define CAST_TYPE(...) reinterpret_cast<llvm::Type*>((__VA_ARGS__))

inline static /*const*/ llvm::StringMap<llvm::Type*> BasicTypes{
    {"void", CAST_TYPE(LLVMVoidType())},
    {"bool", CAST_TYPE(LLVMInt1Type())},
    {"char", CAST_TYPE(LLVMInt8Type())},
    {"short", CAST_TYPE(LLVMInt16Type())},
    {"int", CAST_TYPE(LLVMInt32Type())},
    {"int64", CAST_TYPE(LLVMInt64Type())},
    {"int128", CAST_TYPE(LLVMInt128Type())},
    {"half", CAST_TYPE(LLVMHalfType())},
    {"double", CAST_TYPE(LLVMDoubleType())},
    {"float", CAST_TYPE(LLVMFloatType())},
    {"auto", CAST_TYPE(LLVMStructCreateNamed(LLVMGetGlobalContext(),
                                             "auto"))}}; // placeholder

#undef CAST_TYPE

// @todo References?
static auto getTypeName(llvm::Type* type) {
  size_t numPointers = 0;
  while (type->isPointerTy()) {
    ++numPointers;
    type = type->getPointerElementType();
  }
  std::string typeName;
  llvm::raw_string_ostream os{typeName};
  if (!type->isPointerTy()) {
    // @todo Signed integers, enums, data classes, functions?
    if (type->isIntegerTy()) {
      switch (type->getPrimitiveSizeInBits()) {
      case 1:
        os << "bool";
        break;
      case 8:
        os << "char";
        break;
      case 16:
        os << "short";
        break;
      case 32:
        os << "int";
        break;
      case 64:
        os << "int64";
        break;
      case 128:
        os << "int128";
        break;
      }
    } else if (type->isFloatingPointTy()) {
      switch (type->getPrimitiveSizeInBits()) {
      case 16:
        os << "half";
        break;
      case 32:
        os << "float";
        break;
      case 64:
        os << "double";
        break;
      }
    } else if (type->isStructTy()) {
      const auto name = type->getStructName();
      if (name.startswith("interface::")) {
        constexpr static auto nameOffset = std::strlen("interface::");
        os << name.substr(nameOffset);
      } else {
        os << name;
      }
    } else {
      // @todo Do/Should we even get here?
      warning("unhandled type kind in getTypeName");
    }
  }
  while (numPointers--) {
    os << '*';
  }
  return typeName;
}

} // end namespace whack

#endif // WHACK_TYPES_HPP
