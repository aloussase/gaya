#pragma once

#include <memory>
#include <optional>
#include <vector>

namespace gaya::eval
{

class interpreter;
class env;

namespace object
{
    struct object;
}

}

namespace gaya::ast
{
struct expression;
using expression_ptr = std::shared_ptr<expression>;
struct StructDeclaration;
}

namespace gaya::types
{

/*
 * A C type.
 */
enum class ForeignType {
    c_Int,
    c_Pointer,
    c_Void,
};

std::optional<ForeignType> foreign_type_from_string(const std::string&);

struct TypeConstraint
{
    [[nodiscard]] TypeConstraint
        with_closed_over_env(std::shared_ptr<eval::env>) const noexcept;

    std::shared_ptr<eval::env> closed_over_env = nullptr;
    ast::expression_ptr condition              = nullptr;
};

enum class TypeKind {
    Any,
    Array,
    Dictionary,
    Function,
    Number,
    Sequence,
    String,
    Struct,
    Unit,
};

class Type final
{
public:
    explicit Type(TypeKind kind)
        : Type { "", kind }
    {
    }

    Type(
        const std::string& declared_type_name,
        TypeKind kind,
        TypeConstraint constraints = {})
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

    /**
     * Return this type's constraint.
     */
    [[nodiscard]] TypeConstraint constraint() const noexcept;

    /**
     * Return a copy of this type with the specified type constraint.
     */
    [[nodiscard]] Type with_constraint(TypeConstraint) const noexcept;

private:
    std::string _declared_type_name;
    TypeKind _kind;
    TypeConstraint _constraint;
};

}
