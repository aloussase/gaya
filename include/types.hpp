#pragma once

#include <vector>

#include <env.hpp>
#include <object.hpp>

namespace gaya::eval
{
class interpreter;
}

namespace gaya::ast
{
struct expression;
using expression_ptr = std::shared_ptr<expression>;
}

namespace gaya::types
{

struct TypeConstraint
{
    std::shared_ptr<eval::env> closed_over_env;
    ast::expression_ptr condition;
};

enum class TypeKind {
    Any,
    Array,
    Dictionary,
    Function,
    Number,
    Sequence,
    String,
    Unit,
};

class Type final
{
public:
    explicit Type(TypeKind kind)
        : Type { "", kind, {} }
    {
    }

    Type(
        const std::string& declared_type_name,
        TypeKind kind,
        TypeConstraint constraints)
        : _declared_type_name { declared_type_name }
        , _kind { kind }
        , _constraint { std::move(constraints) }
    {
    }

    /**
     * @return The kind of this type.
     */
    [[nodiscard]] TypeKind kind() const noexcept;

    /**
     * Check that the provided object is of this type.
     */
    [[nodiscard]] bool
    check(eval::interpreter&, const eval::object::object&) const noexcept;

    /**
     * Try to parse a type from the provided string.
     */
    static std::optional<Type> from_string(const std::string&) noexcept;

    /**
     * @return A string describing this type.
     */
    [[nodiscard]] std::string to_string() const noexcept;

private:
    std::string _declared_type_name;
    TypeKind _kind;
    TypeConstraint _constraint;
};

}
