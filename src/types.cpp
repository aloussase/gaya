#include <eval.hpp>
#include <types.hpp>

namespace gaya::types
{

TypeKind Type::kind() const noexcept
{
    return _kind;
}

bool Type::check(eval::interpreter& interp, const eval::object::object& o)
    const noexcept
{
    bool type_ok       = false;
    bool constraint_ok = true;

    switch (_kind)
    {
    case TypeKind::Any:
    {
        type_ok = true;
        break;
    }
    case TypeKind::Array:
    {
        type_ok = IS_ARRAY(o);
        break;
    }
    case TypeKind::Dictionary:
    {
        type_ok = IS_DICTIONARY(o);
        break;
    }
    case TypeKind::Function:
    {
        type_ok = IS_FUNCTION(o) || IS_BUILTIN_FUNCION(o);
        break;
    }
    case TypeKind::Number:
    {
        type_ok = IS_NUMBER(o);
        break;
    }
    case TypeKind::Sequence:
    {
        type_ok = eval::object::is_sequence(o);
        break;
    }
    case TypeKind::String:
    {
        type_ok = IS_STRING(o);
        break;
    }
    }

    if (_constraint.condition != nullptr)
    {
        using namespace gaya::eval;
        interp.begin_scope(env { _constraint.closed_over_env });
        interp.define(key::global("_"), o);
        auto result = _constraint.condition->accept(interp);
        interp.end_scope();
        if (!object::is_valid(result)) return false;
        constraint_ok = object::is_truthy(result);
    }

    return type_ok && constraint_ok;
}

std::string Type::to_string() const noexcept
{
    if (!_declared_type_name.empty())
    {
        return _declared_type_name;
    }

    switch (_kind)
    {
    case TypeKind::Any: return "Any";
    case TypeKind::Array: return "Array";
    case TypeKind::Dictionary: return "Dictionary";
    case TypeKind::Function: return "Function";
    case TypeKind::Number: return "Number";
    case TypeKind::Sequence: return "Sequence";
    case TypeKind::String: return "String";
    }

    assert(0 && "Type::to_string");
}

std::optional<Type> Type::from_string(const std::string& s) noexcept
{
    if (s == "Dictionary")
        return Type { TypeKind::Dictionary };
    else if (s == "Any")
        return Type { TypeKind::Any };
    else if (s == "Array")
        return Type { TypeKind::Array };
    else if (s == "Function")
        return Type { TypeKind::Function };
    else if (s == "Number")
        return Type { TypeKind::Number };
    else if (s == "Sequence")
        return Type { TypeKind::Sequence };
    else if (s == "String")
        return Type { TypeKind::String };
    else
        return {};
}

}