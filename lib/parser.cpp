#include <iostream>
#include <memory>

#include <fmt/core.h>

#include <ast.hpp>
#include <file_reader.hpp>
#include <parser.hpp>

#define IGNORE(e) ((void)(e));

namespace gaya
{

using namespace std::string_literals;

parser::parser()
    : _lexer { nullptr }
{

    begin_scope();

    /* Define builtins */

    define("typeof"s);
    define("assert"s);
    define("tostring"s);
    define("tosequence"s);
    define("issequence"s);
    define("md5"s);

    define("io.println"s);
    define("io.print"s);
    define("io.readline"s);
    define("io.readfile"s);
    define("io.listdir"s);

    define("string.length"s);
    define("string.concat"s);
    define("string.tonumber"s);
    define("string.index"s);
    define("string.substring"s);
    define("string.startswith"s);
    define("string.endswith"s);
    define("string.trim"s);

    define("array.length"s);
    define("array.concat"s);
    define("array.push"s);
    define("array.pop"s);
    define("array.sort"s);
    define("array.set"s);

    define("dict.length"s);
    define("dict.set"s);
    define("dict.remove"s);
    define("dict.contains"s);

    define("seq.next"s);
    define("seq.make"s);
    define("seq.copy"s);

    define("math.ceil"s);
    define("math.floor"s);

    define("system.args"s);
}

std::vector<diagnostic::diagnostic>& parser::diagnostics() noexcept
{
    return _diagnostics;
}

void parser::report_diagnostics() const noexcept
{
    for (const auto& diagnostic : _diagnostics)
    {
        fmt::println("{}", diagnostic.to_string());
    }
}

bool parser::had_error() const noexcept
{
    return !_diagnostics.empty();
}

const std::vector<parser::scope>& parser::scopes() const noexcept
{
    return _scopes;
}

void parser::clear_diagnostics() noexcept
{
    _diagnostics.clear();
}

ast::node_ptr parser::parse_expression(
    const std::string& filename,
    const char* source) noexcept
{
    _filename = filename;
    _lexer    = lexer { source };

    auto t = _lexer.next_token();
    if (!t) return nullptr;

    return expression(*t);
}

ast::node_ptr parser::parse_statement(
    const std::string& filename,
    const char* source) noexcept
{
    _filename = filename;
    _lexer    = lexer { source };
    return toplevel_stmt();
}

void parser::parser_error(span s, const std::string& message)
{
    _diagnostics
        .emplace_back(s, message, diagnostic::severity::error, _filename);
}

void parser::parser_hint(span s, const std::string& message)
{
    _diagnostics
        .emplace_back(s, message, diagnostic::severity::hint, _filename);
}

bool parser::match(std::optional<token> t, token_type tt) noexcept
{
    return t && t->type == tt;
}

bool parser::match(token_type tt) noexcept
{
    auto t = _lexer.peek_token();
    if (!t) return false;

    if (t->type == tt)
    {
        (void)_lexer.next_token();
        return true;
    }

    return false;
}

void parser::begin_scope() noexcept
{
    _scopes.push_back({});
}

void parser::end_scope() noexcept
{
    assert(_scopes.size() > 0);
    _scopes.pop_back();
}

void parser::define(eval::key key) noexcept
{
    assert(_scopes.size() > 0);
    _scopes.back().insert(key);
}

void parser::define(ast::identifier& ident) noexcept
{
    define(ident.key);
}

bool parser::assign_scope(std::shared_ptr<ast::identifier>& ident) noexcept
{
    assert(_scopes.size() > 0);

    for (int i = _scopes.size() - 1; i >= 0; i--)
    {
        if (_scopes[i].contains(ident->key))
        {
            ident->depth            = _scopes.size() - 1 - i;
            ident->did_assign_scope = true;
            return true;
        }
    }
    return false;
}

bool parser::assign_scope(std::unique_ptr<ast::identifier>& ident) noexcept
{
    assert(_scopes.size() > 0);

    for (int i = _scopes.size() - 1; i >= 0; i--)
    {
        if (_scopes[i].contains(ident->key))
        {
            ident->depth            = _scopes.size() - 1 - i;
            ident->did_assign_scope = true;
            return true;
        }
    }
    return false;
}

bool parser::is_valid_assignment_target(ast::identifier& ident) noexcept
{
    assert(_scopes.size() > 0);

    for (int i = _scopes.size() - 1; i >= 0; i--)
    {
        if (auto it = _scopes[i].find(ident.key); it != _scopes[i].end())
        {
            return it->kind == eval::identifier_kind::local;
        }
    }
    return false;
}

bool parser::is_local_stmt(token t) noexcept
{
    switch (t.type)
    {
    case token_type::ampersand:
    case token_type::for_:
    case token_type::while_: return true;
    default: return false;
    }
}

ast::expression_ptr parser::try_parse_statement_as_expression(token t) noexcept
{
    if (!is_local_stmt(t)) return nullptr;

    auto stmt = local_stmt(t);
    if (!stmt) return nullptr;

    return ast::make_node<ast::perform_expression>(t, stmt);
}

ast::node_ptr
parser::parse(const std::string& filename, const char* source) noexcept
{
    /* Create a new lexer for the given source code. */
    _filename = filename;
    _lexer    = lexer { source };

    auto program   = ast::make_node<ast::program>();
    program->stmts = stmts();

    /* Merge any lexer diagnostics. */
    for (const auto& diag : _lexer.diagnostics())
    {
        _diagnostics.insert(_diagnostics.begin(), diag);
    }

    return program;
}

std::vector<ast::stmt_ptr> parser::stmts() noexcept
{
    std::vector<ast::stmt_ptr> _stmts;

    for (;;)
    {
        if (auto stmt = toplevel_stmt(); stmt)
        {
            _stmts.push_back(std::move(stmt));
        }
        else
        {
            break;
        }
    }

    return _stmts;
}

ast::stmt_ptr parser::toplevel_stmt() noexcept
{
    auto token = _lexer.next_token();
    if (!token) return nullptr;

    switch (token->type)
    {
    case token_type::identifier:
    {
        if (auto colon_colon = _lexer.peek_token();
            match(colon_colon, token_type::colon_colon))
        {
            return declaration_stmt(token.value());
        }
        else
        {
            return expression_stmt(token.value());
        }
    }
    case token_type::include:
    {
        return include_stmt(token.value());
    }
    case token_type::type:
    {
        return type_declaration(*token);
    }
    case token_type::struct_:
    {
        return struct_declaration(*token);
    }
    case token_type::enum_:
    {
        return enum_declaration(*token);
    }
    case token_type::for_:
    {
        return for_in_stmt(*token);
    }
    default:
    {
        return expression_stmt(token.value());
    }
    }
}

ast::stmt_ptr parser::local_stmt(token token) noexcept
{
    switch (token.type)
    {
    case token_type::ampersand:
    {
        return assignment_stmt(token);
    }
    case token_type::while_:
    {
        return while_stmt(token);
    }
    case token_type::for_:
    {
        return for_in_stmt(token);
    }
    default:
    {
        return expression_stmt(token);
    }
    }
}

ast::stmt_ptr parser::assignment_stmt(token ampersand) noexcept
{
    assert(ampersand.type == token_type::ampersand);

    auto span = ampersand.span;

    auto t_t = _lexer.next_token();
    auto t   = t_t ? expression(*t_t) : nullptr;
    if (!t)
    {
        parser_error(ampersand.span, "Expected an expression after '&'");
        return nullptr;
    }

    if (!match(token_type::back_arrow))
    {
        parser_error(
            ampersand.span,
            "Expected '<-' after expression in assignment statement");
        return nullptr;
    }

    auto e_t = _lexer.next_token();
    auto e   = e_t ? expression(*e_t) : nullptr;
    if (!e)
    {
        parser_error(
            ampersand.span,
            "Expected expression after '<-' in assignment statement");
        return nullptr;
    }

    std::optional<ast::AssignmentKind> assignment_kind = std::nullopt;

    if (auto identifier = std::dynamic_pointer_cast<ast::identifier>(t))
    {
        if (!is_valid_assignment_target(*identifier))
        {
            parser_error(span, "Invalid assignment target");
            parser_hint(span, "Only local variables may be assigned to");
            return nullptr;
        }

        IGNORE(assign_scope(identifier));
        assignment_kind = ast::AssignmentKind::Identifier;
    }

    if (auto get_expr = std::dynamic_pointer_cast<ast::get_expression>(t))
    {
        assignment_kind = ast::AssignmentKind::GetExpression;
    }

    if (std::dynamic_pointer_cast<ast::call_expression>(t))
    {
        assignment_kind = ast::AssignmentKind::CallExpression;
    }

    if (!assignment_kind)
    {
        parser_error(span, "Invalid assignment target");
        return nullptr;
    }

    auto assignment_stmt = ast::make_node<ast::assignment_stmt>(
        *assignment_kind,
        std::move(t),
        e);

    assignment_stmt->target->set_parent(assignment_stmt);
    assignment_stmt->expression->set_parent(assignment_stmt);

    return assignment_stmt;
}

ast::stmt_ptr parser::while_stmt(token while_) noexcept
{
    assert(while_.type == token_type::while_);

    begin_scope();

    std::optional<ast::while_stmt::initializer> initializer;

    if (match(token_type::let))
    {
        /* Initializer
         *
         * NOTE: Using let for while initializers means we can't use it in the
         * condition without complicating parsing. If it turns out to be a
         * common case to want to use let in while condition, I'll implement
         * support for it.
         */
        auto i_t = _lexer.next_token();
        if (!match(i_t, token_type::identifier))
        {
            parser_error(
                while_.span,
                "Expected an identifier in while statement initializer");
            end_scope();
            return nullptr;
        }

        if (!match(token_type::equal))
        {
            parser_error(
                while_.span,
                "Expected a '=' after identifier in while statement "
                "initializer");
            end_scope();
            return nullptr;
        }

        define(eval::key::local(i_t->span.to_string()));
        auto e_t = _lexer.next_token();
        auto e   = e_t ? expression(*e_t) : nullptr;
        if (!e)
        {
            parser_error(
                while_.span,
                "Expected an expression after '=' in while statement "
                "initializer");
            end_scope();
            return nullptr;
        }

        initializer = ast::while_stmt::initializer {
            i_t->span.to_string(),
            e,
        };

        if (!match(token_type::colon))
        {
            parser_error(
                while_.span,
                "Expected a ':' after initializer in while statement");
            end_scope();
            return nullptr;
        }
    }

    auto span            = while_.span;
    auto condition_token = _lexer.next_token();
    if (!condition_token)
    {
        parser_error(span, "Expected a condition after 'while'");
        end_scope();
        return nullptr;
    }

    auto condition = expression(condition_token.value());
    if (!condition)
    {
        parser_error(span, "Expected a condition after 'while'");
        end_scope();
        return nullptr;
    }

    ast::stmt_ptr continuation = nullptr;

    if (match(token_type::colon))
    {
        auto a_t     = _lexer.next_token();
        continuation = a_t ? assignment_stmt(*a_t) : nullptr;
        if (!continuation)
        {
            parser_error(
                while_.span,
                "Expected a an assignment statement after ':' in while");
            end_scope();
            return nullptr;
        }
    }

    std::vector<ast::stmt_ptr> body;

    for (;;)
    {
        auto stmt_token = _lexer.next_token();
        if (!stmt_token) break;

        if (match(stmt_token, token_type::end))
        {
            _lexer.push_back(stmt_token.value());
            break;
        }

        auto stmt = local_stmt(stmt_token.value());
        if (!stmt)
        {
            parser_error(
                while_.span,
                "Expected a local statement in while body");
            end_scope();
            return nullptr;
        }

        body.push_back(std::move(stmt));
    }

    end_scope();

    if (auto end_token = _lexer.next_token();
        !match(end_token, token_type::end))
    {
        parser_error(
            span,
            fmt::format(
                "Expected 'end' after while body, but got '{}'",
                end_token ? end_token->span.to_string() : "nothing"));
        parser_hint(span, "You can only use local statements in while body");
        return nullptr;
    }

    auto while_stmt = ast::make_node<ast::while_stmt>(
        span,
        condition,
        std::move(body),
        continuation,
        initializer);

    while_stmt->condition->set_parent(while_stmt);

    if (while_stmt->continuation)
    {
        while_stmt->continuation->set_parent(while_stmt);
    }

    // TODO: Set parent for body nodes as well.

    return while_stmt;
}

ast::stmt_ptr parser::for_in_stmt(token for_) noexcept
{
    assert(for_.type == token_type::for_);

    begin_scope();

    auto ident = _lexer.next_token();
    if (!match(ident, token_type::identifier))
    {
        parser_error(for_.span, "Expected an identifier after 'for'");
        end_scope();
        return nullptr;
    }

    auto identifier
        = ast::make_node<ast::identifier>(ident->span, ident->span.to_string());
    define(identifier->key);

    if (!match(token_type::in))
    {
        parser_error(for_.span, "Expected 'in' after identifier in for-loop");
        end_scope();
        return nullptr;
    }

    auto e_t = _lexer.next_token();
    if (!e_t)
    {
        parser_error(
            for_.span,
            "Expected an expression after 'in' in for-loop");
        end_scope();
        return nullptr;
    }

    auto e = expression(*e_t);
    if (!e)
    {
        end_scope();
        return nullptr;
    }

    std::vector<ast::stmt_ptr> body;

    for (;;)
    {
        auto s_t = _lexer.next_token();
        if (!s_t) break;

        if (match(s_t, token_type::end))
        {
            _lexer.push_back(s_t.value());
            break;
        }

        auto stmt = local_stmt(s_t.value());
        if (!stmt)
        {
            parser_error(for_.span, "Expected a local statement in for body");
            end_scope();
            return nullptr;
        }

        body.push_back(std::move(stmt));
    }

    end_scope();

    if (!match(token_type::end))
    {
        parser_error(for_.span, "Expected 'end' after for-loop");
        return nullptr;
    }

    return ast::make_node<ast::for_in_stmt>(
        for_.span,
        std::move(identifier),
        std::move(e),
        std::move(body));
}

ast::stmt_ptr parser::declaration_stmt(token identifier)
{
    auto colon_colon = _lexer.next_token();
    if (!colon_colon || colon_colon->type != token_type::colon_colon)
    {
        parser_error(identifier.span, "Expected a '::' after identifier");
        return nullptr;
    }

    auto ident = std::make_unique<ast::identifier>(
        identifier.span,
        identifier.span.to_string());

    /*
     * Here we are defining the identifier before evaluating its expression,
     * which means that we do support top level recursive declarations.
     *
     * NOTE: We still need to add support for this in the evaluator.
     * NOTE: I might not have updated this comment since adding support to the
     *       evaluator.
     */
    define(*ident);

    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(colon_colon->span, "Expected an expression after '::'");
        return nullptr;
    }

    auto expr = expression(token.value());
    if (!expr) return nullptr;

    return ast::make_node<ast::declaration_stmt>(std::move(ident), expr);
}

ast::stmt_ptr parser::expression_stmt(token token)
{
    auto expr = expression(token);
    if (!expr) return nullptr;

    if (!match(token_type::dot))
    {
        parser_error(
            token.span,
            "Expected '.' after expression in expression statement");
        parser_hint(token.span, "To print a value, use io.println(value)");
        return nullptr;
    }

    return ast::make_node<ast::expression_stmt>(expr);
}

ast::stmt_ptr parser::include_stmt(token include) noexcept
{
    auto t = _lexer.next_token();
    if (!match(t, token_type::string))
    {
        parser_error(include.span, "Expected a string after include");
        return nullptr;
    }

    std::filesystem::path file_path = t->span.to_string();
    auto span                       = include.span;

    if (file_path.has_extension() && file_path.extension() != ".gaya")
    {
        parser_error(span, "Can only include gaya files");
        return nullptr;
    }
    else
    {
        file_path = file_path.replace_extension("gaya");
    }

    if (!std::filesystem::exists(file_path))
    {
        /* If the file is not found, try relative to the script file. */
        auto script_file = std::filesystem::absolute(_filename);
        file_path        = script_file.replace_filename(file_path);
    }

    if (!std::filesystem::exists(file_path))
    {
        file_path = GAYA_RUNTIME / file_path.filename();
    }

    file_reader reader { file_path };
    if (!reader)
    {
        parser_error(
            span,
            fmt::format(
                "Failed to read the contents of {}",
                file_path.string()));
        return nullptr;
    }

    if (_included_files.contains(file_path))
    {
        /* If we've already processed that file, just ignore it. */
        return ast::make_node<ast::expression_stmt>(
            ast::make_node<ast::unit>(span));
    }

    auto* source = reader.slurp();
    assert(source);

    auto lexer    = _lexer;
    auto filename = _filename;

    auto ast = parse(file_path, source);
    if (!ast)
    {
        parser_error(
            span,
            fmt::format(
                "There were errors while parsing {}",
                file_path.string()));
        return nullptr;
    }

    _lexer    = lexer;
    _filename = filename;

    return ast::make_node<ast::include_stmt>(
        include.span,
        t->span.to_string(),
        ast);
}

void parser::define_type(
    span span,
    const std::string& identifier,
    types::TypeKind kind) noexcept
{
    auto type_declaration = ast::make_node<ast::TypeDeclaration>(
        span,
        identifier,
        types::Type { identifier, kind },
        nullptr);
    _type_declarations.insert({ identifier, type_declaration });
    define(identifier);
}

ast::stmt_ptr parser::type_declaration(token type) noexcept
{
    assert(type.type == token_type::type);

    auto d_t = _lexer.next_token();
    if (!match(d_t, token_type::identifier))
    {
        parser_error(type.span, "Expected a type identifier after 'type'");
        return nullptr;
    }

    if (!match(token_type::of))
    {
        parser_error(type.span, "Expected 'of' after type identifier");
        return nullptr;
    }

    auto u_t_token = _lexer.next_token();
    auto u_t = u_t_token ? types::Type::from_string(u_t_token->span.to_string())
                         : std::nullopt;
    if (!u_t)
    {
        parser_error(type.span, "Expected a type after 'of'");
        return nullptr;
    }

    ast::expression_ptr c = nullptr;
    if (match(token_type::with))
    {
        begin_scope();
        auto e_t = _lexer.next_token();
        auto e   = e_t ? expression(*e_t) : nullptr;
        end_scope();

        if (!e)
        {
            parser_error(type.span, "Expected an expression after 'with'");
            return nullptr;
        }

        c = e;
    }

    auto declared_type    = d_t->span.to_string();
    auto type_declaration = ast::make_node<ast::TypeDeclaration>(
        type.span,
        declared_type,
        *u_t,
        c);

    /* We use this when resolving types in function parameters. */
    _type_declarations.insert({ declared_type, type_declaration });

    return type_declaration;
}

std::vector<ast::StructField> parser::struct_fields() noexcept
{
    std::vector<ast::StructField> fields;

    for (;;)
    {
        auto i_t = _lexer.next_token();
        if (!match(i_t, token_type::identifier))
        {
            if (i_t) _lexer.push_back(*i_t);
            break;
        }

        if (!match(token_type::colon))
        {
            parser_error(i_t->span, "Expected a ':' after identifier");
            return {};
        }

        auto identifier = i_t->span.to_string();
        auto type       = type_id();

        if (!type)
        {
            parser_error(i_t->span, "Expected a type after ':'");
            return {};
        }

        fields.push_back(ast::StructField { identifier, *type });
    }

    return fields;
}

ast::stmt_ptr parser::struct_declaration(token struct_) noexcept
{
    assert(struct_.type == token_type::struct_);

    auto span = struct_.span;
    auto i_t  = _lexer.next_token();
    if (!match(i_t, token_type::identifier))
    {
        parser_error(struct_.span, "Expected an identifier after 'struct'");
        return nullptr;
    }

    auto identifier = i_t->span.to_string();
    auto fields     = struct_fields();
    if (fields.empty())
    {
        parser_error(span, "Expected at least one field in struct");
        return nullptr;
    }

    if (!match(token_type::end))
    {
        parser_error(span, "Expected 'end' after struct declaration");
        return nullptr;
    }

    define_type(span, identifier, types::TypeKind::Struct);

    return ast::make_node<ast::StructDeclaration>(span, identifier, fields);
}

ast::stmt_ptr parser::enum_declaration(token enum_) noexcept
{
    assert(enum_.type == token_type::enum_);

    auto span       = enum_.span;
    auto i_t        = _lexer.next_token();
    auto identifier = i_t ? i_t->span.to_string() : "";
    if (!match(i_t, token_type::identifier))
    {
        parser_error(enum_.span, "Expected an identifier after 'enum'");
        return nullptr;
    }

    std::vector<std::string> variants;
    for (;;)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        if (match(t, token_type::end))
        {
            _lexer.push_back(*t);
            break;
        }

        if (!match(t, token_type::identifier))
        {
            parser_error(t->span, "Expected an identifier in enum body");
            return nullptr;
        }

        variants.emplace_back(t->span.to_string());
    }

    if (!match(token_type::end))
    {
        parser_error(span, "Expected 'end' after enum declaration");
        return nullptr;
    }

    define_type(span, identifier, types::TypeKind::Enum);

    return ast::make_node<ast::EnumDeclaration>(span, identifier, variants);
}

ast::expression_ptr parser::expression(token token)
{
    switch (token.type)
    {
    case token_type::do_: return do_expression(token);
    case token_type::cases: return case_expression(token);
    default:
    {
        auto stmt_expr = try_parse_statement_as_expression(token);
        if (stmt_expr) return stmt_expr;

        return logical_expression(token);
    }
    }
}

std::optional<types::Type> parser::type_id() noexcept
{
    auto t_t = _lexer.next_token();
    if (!match(t_t, token_type::identifier))
    {
        return {};
    }

    auto span       = t_t->span;
    auto identifier = span.to_string();

    auto m_t = types::Type::from_string(identifier);
    if (m_t) return m_t;

    auto it = _type_declarations.find(identifier);
    if (it == _type_declarations.end())
    {
        parser_error(
            t_t->span,
            fmt::format("Invalid type identifier: {}", identifier));
        return {};
    }

    auto type_declaration = it->second;
    return types::Type {
        type_declaration->declared_type,
        type_declaration->underlying_type.kind(),
        { nullptr, type_declaration->constraint },
    };
}

ast::expression_ptr parser::function_expression(token lcurly)
{
    assert(lcurly.type == token_type::lcurly);

    begin_scope();

    std::vector<ast::function_param> params;

    bool parsed_default_argument = false;

    for (;;)
    {
        auto p_t = _lexer.next_token();
        if (!p_t || match(p_t, token_type::arrow))
        {
            if (p_t) _lexer.push_back(*p_t);
            break;
        }

        auto p = match_pattern(*p_t, true, &eval::key::param);
        if (!p)
        {
            parser_error(
                p_t->span,
                "Expected a pattern in function parameter list");
            end_scope();
            return nullptr;
        }

        types::Type t           = types::Type { types::TypeKind::Any };
        ast::expression_ptr d_a = nullptr;

        if (match(token_type::colon))
        {
            if (auto m_t = type_id(); m_t)
            {
                t = *m_t;
            }
            else
            {
                parser_error(
                    lcurly.span,
                    "Expected a type after ':' in function parameter");
                end_scope();
                return nullptr;
            }
        }

        if (match(token_type::equal))
        {
            auto d_t = _lexer.next_token();
            auto d   = d_t ? expression(*d_t) : nullptr;

            if (!d)
            {
                parser_error(
                    lcurly.span,
                    "Expected an expression after '=' in default argument");
                end_scope();
                return nullptr;
            }

            d_a                     = d;
            parsed_default_argument = true;
        }
        else if (parsed_default_argument)
        {
            parser_error(
                lcurly.span,
                "Positional parameters cannot follow default arguments");
            end_scope();
            return nullptr;
        }

        ast::function_param param = { std::move(*p), t, d_a };
        params.push_back(std::move(param));

        if (!match(token_type::comma))
        {
            break;
        }
    }

    if (!match(token_type::arrow))
    {
        parser_error(lcurly.span, "Expected '=>' after function parameters");
        end_scope();
        return nullptr;
    }

    if (auto r_t = _lexer.next_token(); match(r_t, token_type::rcurly))
    {
        end_scope();
        return ast::make_node<ast::function_expression>(
            lcurly.span,
            std::move(params),
            ast::make_node<ast::unit>(r_t->span));
    }
    else if (r_t)
    {
        auto e = expression(*r_t);
        if (!e)
        {
            parser_error(
                lcurly.span,
                "Expected an expression after '=>' in function literal");
            end_scope();
            return nullptr;
        }

        if (!match(token_type::rcurly))
        {
            parser_error(lcurly.span, "Expected a '}' after function body");
            end_scope();
            return nullptr;
        }

        end_scope();
        return ast::make_node<ast::function_expression>(
            lcurly.span,
            std::move(params),
            std::move(e));
    }
    else
    {
        parser_error(lcurly.span, "Expected a '}' after function body");
        end_scope();
        return nullptr;
    }
}

ast::expression_ptr parser::logical_expression(token token) noexcept
{
    auto lhs = comparison_expression(token);
    if (!lhs) return nullptr;

    auto done = false;
    while (!done)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        switch (t->type)
        {
        case token_type::or_:
        case token_type::and_:
        {
            auto op = t.value();

            t = _lexer.next_token();
            if (!t)
            {
                parser_error(
                    op.span,
                    fmt::format(
                        "Expected an expression after {}",
                        op.span.to_string()));
                return nullptr;
            }

            auto rhs = comparison_expression(t.value());
            if (!rhs) return nullptr;

            lhs = ast::make_node<ast::binary_expression>(lhs, op, rhs);

            break;
        }
        default:
        {
            _lexer.push_back(t.value());
            done = true;
            break;
        }
        }
    }

    return lhs;
}

ast::expression_ptr parser::comparison_expression(token token) noexcept
{
    auto lhs = pipe_expression(token);
    if (!lhs) return nullptr;

    auto is_comparison_op = [](auto t) {
        switch (t.type)
        {
        case token_type::less_than:
        case token_type::less_than_eq:
        case token_type::greater_than:
        case token_type::greater_than_eq:
        case token_type::equal_equal:
        case token_type::not_equals: return true;
        default: return false;
        }
    };

    for (;;)
    {
        auto op = _lexer.next_token();
        if (!op) break;

        if (!is_comparison_op(op.value()))
        {
            _lexer.push_back(op.value());
            break;
        }

        auto expr_token = _lexer.next_token();
        if (!expr_token)
        {
            parser_error(
                token.span,
                fmt::format(
                    "Expected an expression after {}",
                    token.span.to_string()));
            return nullptr;
        }

        auto rhs = pipe_expression(expr_token.value());

        if (!rhs) return nullptr;

        lhs = ast::make_node<ast::binary_expression>(lhs, op.value(), rhs);
    }

    return lhs;
}

ast::expression_ptr parser::pipe_expression(token token) noexcept
{
    auto lhs = bitwise_expression(token);
    if (!lhs) return nullptr;

    for (;;)
    {
        auto pipe_token = _lexer.next_token();
        if (!match(pipe_token, token_type::pipe))
        {
            if (pipe_token)
            {
                _lexer.push_back(pipe_token.value());
            }
            break;
        }

        auto expr_token = _lexer.next_token();
        if (!expr_token)
        {
            parser_error(
                token.span,
                "Expected an expression after pipe operator");
            return nullptr;
        }

        begin_scope();
        auto rhs = bitwise_expression(expr_token.value());
        end_scope();

        if (!rhs) return nullptr;

        assert(lhs && rhs);

        lhs = ast::make_node<ast::binary_expression>(
            lhs,
            pipe_token.value(),
            rhs);
    }

    return lhs;
}

ast::expression_ptr parser::bitwise_expression(token token) noexcept
{
    auto lhs = term_expression(token);
    if (!lhs) return nullptr;

    auto done = false;
    while (!done)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        switch (t->type)
        {
        case token_type::land:
        case token_type::lor:
        case token_type::xor_:
        case token_type::lshift:
        case token_type::rshift:
        {
            auto op = t.value();

            t = _lexer.next_token();
            if (!t)
            {
                parser_error(
                    op.span,
                    fmt::format(
                        "Expected an expression after {}",
                        op.span.to_string()));
                return nullptr;
            }

            auto rhs = term_expression(t.value());
            if (!rhs) return nullptr;

            lhs = ast::make_node<ast::binary_expression>(lhs, op, rhs);

            break;
        }
        default:
        {
            _lexer.push_back(t.value());
            done = true;
            break;
        }
        }
    }

    return lhs;
}

ast::expression_ptr parser::term_expression(token token) noexcept
{
    auto lhs = factor_expression(token);
    if (!lhs) return nullptr;

    auto done = false;
    while (!done)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        switch (t->type)
        {
        case token_type::plus:
        case token_type::dash:
        case token_type::diamond:
        {
            auto op = t.value();

            t = _lexer.next_token();
            if (!t)
            {
                parser_error(
                    op.span,
                    fmt::format(
                        "Expected an expression after {}",
                        op.span.to_string()));
                return nullptr;
            }

            auto rhs = factor_expression(t.value());
            if (!rhs) return nullptr;

            auto new_lhs = ast::make_node<ast::binary_expression>(lhs, op, rhs);

            lhs->set_parent(new_lhs);
            rhs->set_parent(new_lhs);

            lhs = new_lhs;

            break;
        }
        default:
        {
            _lexer.push_back(t.value());
            done = true;
            break;
        }
        }
    }

    return lhs;
}

ast::expression_ptr parser::factor_expression(token token) noexcept
{
    auto lhs = unary_expression(token);
    if (!lhs) return nullptr;

    auto done = false;
    while (!done)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        switch (t->type)
        {
        case token_type::star:
        case token_type::slash:
        {
            auto op = t.value();

            t = _lexer.next_token();
            if (!t)
            {
                parser_error(
                    op.span,
                    fmt::format(
                        "Expected an expression after {}",
                        op.span.to_string()));
                return nullptr;
            }

            auto rhs = unary_expression(t.value());
            if (!rhs) return nullptr;

            lhs = ast::make_node<ast::binary_expression>(lhs, op, rhs);

            break;
        }
        default:
        {
            _lexer.push_back(t.value());
            done = true;
            break;
        }
        }
    }

    return lhs;
}

ast::expression_ptr parser::unary_expression(token op) noexcept
{
    switch (op.type)
    {
    case token_type::perform: return perform_expression(op);
    case token_type::not_: return not_expression(op);
    case token_type::lnot: return lnot_expression(op);
    default: return call_expression(op);
    }
}

ast::expression_ptr parser::lnot_expression(token op) noexcept
{
    assert(op.type == token_type::lnot);
    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(
            op.span,
            fmt::format(
                "Expected an expression after '~'",
                op.span.to_string()));
        return nullptr;
    }

    auto expr = call_expression(token.value());
    if (!expr) return nullptr;

    return ast::make_node<ast::lnot_expression>(op, expr);
}

ast::expression_ptr parser::not_expression(token op) noexcept
{
    assert(op.type == token_type::not_);
    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(
            op.span,
            fmt::format(
                "Expected an expression after 'not'",
                op.span.to_string()));
        return nullptr;
    }

    auto expr = call_expression(token.value());
    if (!expr) return nullptr;

    return ast::make_node<ast::not_expression>(op, expr);
}

ast::expression_ptr parser::perform_expression(token op) noexcept
{
    assert(op.type == token_type::perform);
    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(op.span, "Expected a statement after 'perform'");
        return nullptr;
    }

    auto stmt = local_stmt(token.value());
    if (!stmt) return nullptr;

    return ast::make_node<ast::perform_expression>(op, stmt);
}

std::shared_ptr<ast::function_expression>
parser::finish_trailing_function(span span, token lparen) noexcept
{
    auto function = function_expression(lparen);
    if (!function)
    {
        parser_error(span, "Expected a trailing function expression");
        return nullptr;
    }

    return std::static_pointer_cast<ast::function_expression>(function);
}

std::shared_ptr<ast::get_expression>
parser::finish_get_expression(span span, ast::expression_ptr target) noexcept
{
    auto i_t = _lexer.next_token();
    if (!match(i_t, token_type::identifier))
    {
        parser_error(span, "Expected an identifier after '@'");
        return nullptr;
    }

    auto ident_span  = i_t->span;
    auto ident_value = ident_span.to_string();

    return ast::make_node<ast::get_expression>(
        span,
        target,
        ast::identifier { ident_span, ident_value });
}

std::shared_ptr<ast::call_expression>
parser::finish_call_expression(span span, ast::expression_ptr target) noexcept
{
    std::vector<ast::expression_ptr> args;

    for (;;)
    {
        auto e_t = _lexer.next_token();
        if (match(e_t, token_type::rparen))
        {
            _lexer.push_back(e_t.value());
            break;
        }

        auto arg = e_t ? expression(e_t.value()) : nullptr;
        if (!arg) return nullptr;

        args.push_back(std::move(arg));

        if (!match(token_type::comma))
        {
            break;
        }
    }

    if (!match(token_type::rparen))
    {
        parser_error(span, "Missing ')' after function call");
        return nullptr;
    }

    if (auto t = _lexer.next_token(); match(t, token_type::lcurly))
    {
        auto function = finish_trailing_function(span, *t);
        if (!function) return nullptr;

        args.push_back(std::move(function));
    }
    else if (t)
    {
        _lexer.push_back(*t);
    }

    return ast::make_node<ast::call_expression>(span, target, std::move(args));
}

ast::expression_ptr parser::call_expression(token token)
{
    auto expr = primary_expression(token);
    if (!expr) return nullptr;

    for (;;)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        if (match(t, token_type::lparen))
        {
            auto e = finish_call_expression(token.span, expr);
            if (!e) return nullptr;

            expr = e;
        }
        else if (match(t, token_type::lcurly))
        {
            ast::expression_ptr function
                = finish_trailing_function(token.span, *t);
            if (!function) return nullptr;

            return ast::make_node<ast::call_expression>(
                token.span,
                expr,
                std::vector { function });
        }
        else if (match(t, token_type::at))
        {
            auto e = finish_get_expression(token.span, expr);
            if (!e) return nullptr;

            expr = e;
        }
        else
        {
            _lexer.push_back(t.value());
            break;
        }
    }

    return expr;
}

ast::expression_ptr parser::let_expression(token let)
{
    assert(let.type == token_type::let);

    begin_scope();

    auto let_binding = [&](token p_t) -> std::optional<ast::let_binding> {
        auto p = match_pattern(p_t, false);
        if (!p) return {};

        if (!match(token_type::equal))
        {
            parser_error(
                p_t.span,
                "Expected '=' after pattern in let expression");
            return std::nullopt;
        }

        auto e_t = _lexer.next_token();
        if (!e_t)
        {
            parser_error(
                p_t.span,
                "Expected an expression after '=' in let expression");
            return std::nullopt;
        }

        auto e = expression(e_t.value());
        if (!e) return std::nullopt;

        if (p->kind == ast::match_pattern::kind::capture)
        {
            auto e_    = std::get<ast::expression_ptr>(p->value);
            auto ident = std::static_pointer_cast<ast::identifier>(e_);
            define(ident->key);
        }

        return ast::let_binding { p_t.span, std::move(*p), std::move(e) };
    };

    std::vector<ast::let_binding> bindings;
    for (;;)
    {
        auto p_t = _lexer.next_token();
        if (!p_t) break;

        if (match(p_t, token_type::in))
        {
            _lexer.push_back(*p_t);
            break;
        }

        auto binding = let_binding(*p_t);
        if (!binding) return nullptr;

        bindings.push_back(std::move(*binding));

        if (!match(token_type::comma))
        {
            break;
        }
    }

    if (bindings.empty())
    {
        parser_error(
            let.span,
            "Expected at least one binding in let expression");
        end_scope();
        return nullptr;
    }

    if (!match(token_type::in))
    {
        parser_error(
            let.span,
            "Expected 'in' after bindings in let expression");
        end_scope();
        return nullptr;
    }

    auto e_t = _lexer.next_token();
    if (!e_t)
    {
        parser_error(let.span, "Expected an expression after 'in'");
        end_scope();
        return nullptr;
    }

    auto e = expression(*e_t);
    if (!e)
    {
        end_scope();
        return nullptr;
    }

    end_scope();

    return ast::make_node<ast::let_expression>(std::move(bindings), e);
}

ast::expression_ptr parser::case_expression(token cases)
{
    std::vector<ast::case_branch> branches;

    for (;;)
    {
        auto g_t = _lexer.next_token();
        if (!g_t) break;

        if (!match(g_t, token_type::given))
        {
            if (match(g_t, token_type::otherwise)
                || match(g_t, token_type::end))
            {
                _lexer.push_back(g_t.value());
                break;
            }
            else
            {
                return match_expression(g_t.value());
            }
        }

        auto c_t       = _lexer.next_token();
        auto condition = c_t ? expression(*c_t) : nullptr;
        if (!condition)
        {
            parser_error(
                g_t->span,
                "Expected an expression after 'given' in cases");
            return nullptr;
        }

        if (!match(token_type::arrow))
        {
            parser_error(g_t->span, "Expected a '=>' after condition in case");
            return nullptr;
        }

        auto e_t  = _lexer.next_token();
        auto body = e_t ? expression(*e_t) : nullptr;
        if (!body)
        {
            parser_error(
                g_t->span,
                "Expected an expression after condition in case");
            return nullptr;
        }

        branches.push_back(ast::case_branch {
            std::move(condition),
            std::move(body),
        });
    }

    ast::expression_ptr otherwise = nullptr;

    if (match(token_type::otherwise))
    {
        if (!match(token_type::arrow))
        {
            parser_error(cases.span, "Expected '=>' after otherwise");
            return nullptr;
        }

        auto e_t  = _lexer.next_token();
        otherwise = e_t ? expression(*e_t) : nullptr;
        if (!otherwise)
        {
            parser_error(cases.span, "Expected an expression after otherwise");
            return nullptr;
        }
    }

    if (!match(token_type::end))
    {
        parser_error(cases.span, "Expected 'end' after cases");
        return nullptr;
    }

    return ast::make_node<ast::case_expression>(
        cases.span,
        std::move(branches),
        otherwise);
}

std::vector<ast::match_pattern>
parser::match_patterns(token pattern_token) noexcept
{
    std::vector<ast::match_pattern> patterns;

    for (;;)
    {
        auto t = _lexer.next_token();
        if (!t)
        {
            parser_error(
                pattern_token.span,
                "Expected a pattern after '(' in match branch");
            return {};
        }

        if (match(t, token_type::rparen))
        {
            _lexer.push_back(t.value());
            break;
        }

        auto pattern = match_pattern(t.value());
        if (!pattern) return {};

        patterns.push_back(pattern.value());

        if (!match(token_type::comma))
        {
            break;
        }
    }

    if (!match(token_type::rparen))
    {
        parser_error(
            pattern_token.span,
            "Expected a ')' after array pattern in case branch");
        return {};
    }

    return patterns;
}

std::optional<ast::match_pattern> parser::match_pattern(
    const token& pattern_token,
    bool define_matched_identifier,
    std::function<eval::key(const std::string&)> to_key,
    std::shared_ptr<ast::identifier> as_pattern) noexcept
{
#define DEFINE_AS_PATTERN                        \
    if (as_pattern && define_matched_identifier) \
        define(to_key(as_pattern->value));

    switch (pattern_token.type)
    {
    case token_type::underscore:
    {
        DEFINE_AS_PATTERN;
        return ast::match_pattern {
            ast::match_pattern::kind::wildcard,
            nullptr,
            as_pattern,
        };
    }
    case token_type::identifier:
    {
        if (auto t = _lexer.peek_token(); match(t, token_type::lparen))
        {
            auto name = pattern_token.span.to_string();
            if (isupper(name[0]))
            {
                IGNORE(_lexer.next_token());

                auto patterns = match_patterns(pattern_token);
                if (patterns.empty()) return {};

                auto struct_pattern = ast::match_pattern::struct_pattern {
                    name,
                    patterns,
                };

                DEFINE_AS_PATTERN;
                return ast::match_pattern {
                    ast::match_pattern::kind::struct_pattern,
                    struct_pattern,
                    as_pattern,
                };
            }
            else
            {
                /* Not an identifier, a function call. */
                auto e = expression(pattern_token);
                if (!e) return {};

                DEFINE_AS_PATTERN;
                return ast::match_pattern {
                    ast::match_pattern::kind::expr,
                    e,
                    as_pattern,
                };
            }
        }
        else
        {
            /* Here we have an identifier. */
            auto identifier = ast::make_node<ast::identifier>(
                pattern_token.span,
                pattern_token.span.to_string());
            identifier->key = to_key(identifier->value);

            if (match(token_type::at))
            {
                auto t = _lexer.next_token();
                if (!t)
                {
                    parser_error(
                        pattern_token.span,
                        "Expected a pattern after '@'");
                    return {};
                }
                return match_pattern(
                    *t,
                    define_matched_identifier,
                    to_key,
                    identifier);
            }

            if (define_matched_identifier)
            {
                /*
                 * We do this conditionally because in let expressions we need
                 * to define the identifier after evaluating the binding's
                 * body.
                 *
                 * This is because the binding value may be a variable of the
                 * same name as the capture pattern.
                 *
                 * This could also technically happen in match expressions, but
                 * we'll see.
                 */
                define(*identifier);
            }

            DEFINE_AS_PATTERN;

            return ast::match_pattern {
                ast::match_pattern::kind::capture,
                identifier,
                as_pattern,
            };
        }
    }
    case token_type::lparen:
    {
        /*
         * We are reusing the array syntax for array matches.
         *
         * This means we cannot match on dictionaries. I'll change this
         * according to how useful it proves to match on dictionaries.
         */
        auto patterns = match_patterns(pattern_token);
        if (patterns.empty()) return {};

        DEFINE_AS_PATTERN;

        return ast::match_pattern {
            ast::match_pattern::kind::array_pattern,
            patterns,
            as_pattern,
        };
    }
    default:
    {
        /* Otherwise, the pattern is an arbitrary expression. */
        auto e = expression(pattern_token);
        if (!e) return {};

        DEFINE_AS_PATTERN;

        return ast::match_pattern {
            ast::match_pattern::kind::expr,
            e,
            as_pattern,
        };
    }
    }
#undef DEFINE_AS_PATTERN
}

ast::expression_ptr parser::match_expression(token target)
{
    auto target_expr = expression(target);
    if (!target_expr)
    {
        parser_error(
            target.span,
            "Expected an expression after 'cases' in match expression");
        return nullptr;
    }

    std::vector<ast::match_branch> branches;
    for (;;)
    {
        /* Begin a new scope for the current branch. */
        begin_scope();

        auto given_token = _lexer.next_token();
        if (!match(given_token, token_type::given))
        {
            if (given_token) _lexer.push_back(given_token.value());
            end_scope();
            break;
        }

        auto pattern_token = _lexer.next_token();
        if (!pattern_token)
        {
            parser_error(
                given_token->span,
                "Expected a pattern after 'given' in match expression");
            end_scope();
            return nullptr;
        }

        auto pattern = match_pattern(pattern_token.value());
        if (!pattern)
        {
            parser_error(
                given_token->span,
                "Expected a pattern in match branch");
            end_scope();
            return nullptr;
        }

        /* An optional when clause. */
        ast::expression_ptr condition = nullptr;

        if (match(token_type::when))
        {
            auto when_expr_token = _lexer.next_token();
            if (!when_expr_token)
            {
                parser_error(
                    given_token->span,
                    "Expected an expression after when in match "
                    "expression");
                end_scope();
                return nullptr;
            }

            auto when_expr = expression(when_expr_token.value());
            if (!when_expr)
            {
                parser_error(
                    given_token->span,
                    "Expected an expression after when in match "
                    "expression");
                end_scope();
                return nullptr;
            }

            condition = when_expr;
        }

        /* A mandatory arrow. */
        if (!match(token_type::arrow))
        {
            parser_error(
                given_token->span,
                "Expected a '=>' in match expression branch");
            end_scope();
            return nullptr;
        }

        /* The branch body. */
        auto expr_token = _lexer.next_token();
        if (!expr_token)
        {
            parser_error(
                given_token->span,
                "Expected an expression after '=>' in match branch");
            end_scope();
            return nullptr;
        }

        auto expr = expression(expr_token.value());
        if (!expr)
        {
            parser_error(
                given_token->span,
                "Expected an expression after '=>' in match branch");
            end_scope();
            return nullptr;
        }

        /* End the scope for the branch. */
        end_scope();

        branches.emplace_back(pattern.value(), expr, condition);
    }

    /* An optional otherwise brach. */
    ast::expression_ptr otherwise = nullptr;
    auto otherwise_token          = _lexer.next_token();

    if (match(otherwise_token, token_type::otherwise))
    {
        if (!match(token_type::arrow))
        {
            parser_error(
                otherwise_token->span,
                "Expected a '=>' after otherwise");
            return nullptr;
        }

        auto expr_token = _lexer.next_token();
        if (!expr_token)
        {
            parser_error(
                otherwise_token->span,
                "Expected an expression after otherwise");
            return nullptr;
        }

        auto expr = expression(expr_token.value());
        if (!expr)
        {
            parser_error(
                otherwise_token->span,
                "Expected an expression after otherwise");
            return nullptr;
        }

        otherwise = expr;
    }
    else
    {
        _lexer.push_back(otherwise_token.value());
    }

    /* A mandatory end token. */
    if (auto end = _lexer.next_token(); !match(end, token_type::end))
    {
        parser_error(
            target.span,
            fmt::format(
                "Expected 'end' after match expression, but got '{}'",
                end ? end->span.to_string() : "nothing"));
        return nullptr;
    }

    return ast::make_node<ast::match_expression>(
        target.span,
        target_expr,
        std::move(branches),
        otherwise);
}

ast::expression_ptr parser::do_expression(token token)
{
    begin_scope();

    std::vector<ast::node_ptr> body;
    bool parsed_final_expression = false;

    while (!parsed_final_expression)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        if (match(t, token_type::end))
        {
            _lexer.push_back(*t);
            break;
        }

        switch (t->type)
        {
        case token_type::ampersand:
        case token_type::while_:
        case token_type::for_:
        {
            auto stmt = local_stmt(*t);
            if (!stmt)
            {
                end_scope();
                return nullptr;
            }
            body.push_back(stmt);
            break;
        }
        default:
        {
            auto expr = expression(*t);
            if (!expr)
            {
                end_scope();
                return nullptr;
            }

            if (match(token_type::dot))
            {
                body.push_back(ast::make_node<ast::expression_stmt>(expr));
            }
            else
            {
                body.push_back(expr);
                parsed_final_expression = true;
            }
            break;
        }
        }
    }

    if (!parsed_final_expression)
    {
        body.push_back(ast::make_node<ast::unit>(token.span));
    }

    end_scope();

    if (auto end = _lexer.next_token(); !match(end, token_type::end))
    {
        auto actual = end ? end->span.to_string() : "nothing";

        parser_error(
            token.span,
            fmt::format(
                "Expected 'end' after last expression in do block, but got "
                "'{}'",
                actual));

        if (actual == "<-")
        {
            parser_hint(
                token.span,
                "To perform assignment, you need to use the '&' operator");
        }
        else
        {
            parser_hint(
                token.span,
                "Check that you didn't forget a '.' after an expression");
        }

        return nullptr;
    }

    return ast::make_node<ast::do_expression>(token.span, std::move(body));
}

ast::expression_ptr parser::primary_expression(token token)
{
    switch (token.type)
    {
    case token_type::number:
    {
        return number(token);
    }
    case token_type::string:
    {
        return string(token);
    }
    case token_type::identifier:
    {
        auto ident = ast::make_node<ast::identifier>(
            token.span,
            token.span.to_string());

        IGNORE(assign_scope(ident));
        return ident;
    }
    case token_type::unit:
    {
        return ast::make_node<ast::unit>(token.span);
    }
    case token_type::lcurly:
    {
        return function_expression(token);
    }
    case token_type::let:
    {
        return let_expression(token);
    }
    case token_type::lparen:
    {
        if (match(token_type::thin_arrow))
        {
            return dictionary(token, nullptr);
        }
        else
        {
            // TODO: push_back
            return array(token);
        }
    }
    case token_type::underscore:
    {
        return ast::make_node<ast::placeholder>(token.span);
    }
    default:
        parser_error(token.span, "Invalid start of primary expression");
        return nullptr;
    }
}

bool parser::is_hex_digit(char c) const noexcept
{
    return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')
        || (c >= '0' && c <= '9');
}

int parser::to_hex_value(char c) const noexcept
{
    if (c >= '0' && c <= '9') return c - '0';

    switch (c)
    {
    case 'a':
    case 'A': return 10;
    case 'b':
    case 'B': return 11;
    case 'c':
    case 'C': return 12;
    case 'd':
    case 'D': return 13;
    case 'e':
    case 'E': return 14;
    case 'f':
    case 'F': return 15;
    default: assert(0 && "not a hex digit");
    }
}

int parser::hex_string_to_int(const std::string& s, size_t& i) const noexcept
{
    int hex_value     = 0;
    size_t num_digits = 0;

    for (;;)
    {
        char hex_digit = s[i + 1];
        if (!hex_digit || !is_hex_digit(hex_digit)) break;
        num_digits += 1;
        i += 1;
    }

    for (size_t x = num_digits; x > 0; x--)
    {
        hex_value |= to_hex_value(s[i - x + 1]) << (4 * (x - 1));
    }

    return hex_value;
}

ast::expression_ptr parser::string(token token) noexcept
{
    auto unescape = [&](const std::string& s) {
        std::string result;

        for (size_t i = 0; i < s.size(); i++)
        {
            if (auto c = s[i]; c != '\\')
            {
                result.push_back(s[i]);
                continue;
            }

            switch (auto escape_character = s[++i]; escape_character)
            {
            case 'n': result.push_back('\n'); break;
            case '\\': result.push_back('\\'); break;
            case 'b': result.push_back('\b'); break;
            case 't': result.push_back('\t'); break;
            case '"': result.push_back('\"'); break;
            case '\'': result.push_back('\''); break;
            case 'x': result.push_back(hex_string_to_int(s, i)); break;
            default:
            {
                parser_error(token.span, "Invalid escape character");
            }
            }
        }

        return result;
    };

    return ast::make_node<ast::string>(
        token.span,
        unescape(token.span.to_string()));
}

ast::expression_ptr parser::number(token token) noexcept
{
    auto value = token.span.to_string();
    double num = 0;

    if (value.starts_with("0x"))
    {
        size_t i = 1;
        num      = hex_string_to_int(value, i);
    }
    else
    {
        num = std::stod(value);
    }

    return ast::make_node<ast::number>(token.span, num);
}

ast::expression_ptr parser::array(token lparen) noexcept
{
    std::vector<ast::expression_ptr> elems;

    for (;;)
    {
        auto expression_token = _lexer.next_token();
        if (!expression_token) break;

        if (match(expression_token, token_type::rparen))
        {
            _lexer.push_back(expression_token.value());
            break;
        }

        auto expr = expression(expression_token.value());
        if (!expr)
        {
            parser_error(
                lparen.span,
                "Expected an expression in array literal");
            return nullptr;
        }

        /*
         * If we see a '->' after the first expression, that means we are
         * parsing a dictionary literal.
         */
        if (match(token_type::thin_arrow))
        {
            return dictionary(lparen, expr);
        }

        elems.push_back(std::move(expr));

        if (!match(token_type::comma))
        {
            break;
        }
    }

    if (auto rparen = _lexer.next_token(); !match(rparen, token_type::rparen))
    {
        parser_error(
            lparen.span,
            fmt::format(
                "Expected a ')' after last expression in array literal, "
                "but "
                "got {}",
                rparen ? '\'' + rparen->span.to_string() + '\'' : "nothing"));
        return nullptr;
    }

    return ast::make_node<ast::array>(lparen.span, std::move(elems));
}

ast::expression_ptr
parser::dictionary(token lparen, ast::expression_ptr first_key) noexcept
{
    std::vector<ast::expression_ptr> keys;
    std::vector<ast::expression_ptr> values;

    if (first_key)
    {
        keys.push_back(std::move(first_key));
    }

    for (;;)
    {
        /* Parse the next value */

        auto expression_token = _lexer.next_token();
        if (!expression_token) break;
        if (match(expression_token, token_type::rparen))
        {
            _lexer.push_back(expression_token.value());
            break;
        }

        auto expr = expression(expression_token.value());
        if (!expr)
        {
            parser_error(
                lparen.span,
                "Expected an expression in dictionary literal");
            return nullptr;
        }

        values.push_back(std::move(expr));

        /* Parse an optional comma */

        if (match(token_type::comma))
        {
            /* Parse the next key */

            auto t = _lexer.next_token();
            if (!t)
            {
                parser_error(lparen.span, "Unterminated dictionary literal");
                return nullptr;
            }

            auto e = expression(t.value());
            if (!e)
            {
                parser_error(
                    lparen.span,
                    "Expected an expression after comma in dictionary "
                    "literal");
                return nullptr;
            }

            keys.push_back(std::move(e));

            /* Parse the arrow after the key */

            auto arrow = _lexer.next_token();
            if (!arrow)
            {
                parser_error(
                    lparen.span,
                    "Expected a '->' after key in dictionary literal");
                return nullptr;
            }
        }
        else
        {
            break;
        }
    }

    if (keys.size() != values.size())
    {
        parser_error(lparen.span, "Unmatched key(s) in dictionary literal");
        return nullptr;
    }

    if (auto rparen = _lexer.next_token(); !match(rparen, token_type::rparen))
    {
        parser_error(
            lparen.span,
            fmt::format(
                "Expected a ')' after last expression in dictionary "
                "literal, "
                "but got {}",
                rparen ? '\'' + rparen->span.to_string() + '\'' : "nothing"));
        return nullptr;
    }

    return ast::make_node<ast::dictionary>(
        lparen.span,
        std::move(keys),
        std::move(values));
}
}
