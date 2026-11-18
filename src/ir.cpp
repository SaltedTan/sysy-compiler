#include "ir.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <unordered_map>

using namespace ir;

Type *Type::i32() {
  static Type i32{TypeKind::I32, nullptr};
  return &i32;
}

Type *Type::unit() {
  static Type unit{TypeKind::Unit, nullptr};
  return &unit;
}

Type *Type::ptr(Type *base) {
  static std::unordered_map<Type *, std::unique_ptr<Type>> pointer_types;

  if (base == nullptr) {
    throw std::runtime_error("pointer base type is null");
  }

  auto it = pointer_types.find(base);
  if (it != pointer_types.end()) {
    return it->second.get();
  }

  auto ptr_type = std::make_unique<Type>();
  ptr_type->kind = TypeKind::Ptr;
  ptr_type->base = base;

  Type *raw_ptr = ptr_type.get();

  pointer_types.emplace(base, std::move(ptr_type));

  return raw_ptr;
}

bool Instruction::isTerminator() const {
  switch (kind) {
  case InstKind::Jump:
  case InstKind::Branch:
  case InstKind::Return:
    return true;
  default:
    return false;
  }
}

bool BasicBlock::terminated() const {
  if (insts.empty())
    return false;
  return insts.back()->isTerminator();
}
