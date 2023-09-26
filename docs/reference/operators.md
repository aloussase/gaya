# Operators

The operators described here follow roughly the same precedence as described
[here](https://en.cppreference.com/w/c/language/operator_precedence).

## String concatenation operator

The `<>` operator can be used to concatenate strings. It comes from the `<>`
operator for semigroups in Haskell.

```
greet :: { name => io.println("Hello " <> name <> "!")}
```

Its precedence is the same as the `+` arithmetic operator.

## Arithmetic operators

Arithmetic operations work as you would exepct in most programming languages:

```ocaml
42 + 27. (* 69 *)
70 - 1.  (* 69 *)
69 * 1.  (* 69 *)

(* Division by zero returns unit. *)
cases 69 / 0
  given unit => io.println("Oops")
  otherwise  => io.println(result)
end.

(* Associativy and precedence works as expected *)
42 + 14 * 2 - 1. (* 69 *)
```

## Comparison operators

It is undefined behaviour to use comparison operators other than `==` and `/=`
between objects of different types.

Note that "not equals" is `/=` instead of `!=`.

Otherwise, comparison operators work as you'd expect in most languages.

```ocaml
"Hello" < "Zap" (* t *)
42 >= 42        (* t *)
"str" == 42     (* f *)
```

## Logical operators

Gaya's logical operators are `and` and `or`, which are equivalent to `&&` and
`||` in other programming languages. One neat feature of these operators is
that you can use them like this:

```
"Hello" and "Gaya". (* "Gaya"  *)
"Hello" or "Gaya".  (* "Hello" *)
```

This is useful for example to compute default values or something.

## Bitwise operators

Gaya has all the bitwise operators from C. Their precedences are not the same
though, as I changed it so they are more convenient. If you have any problems
using them, open an issue in the [repo](https://github.com/aloussase/gaya).

## Pipe operator

Gaya implements [Hack style
pipes](https://docs.hhvm.com/hack/expressions-and-operators/pipe). You can pipe
any expression into another one and replace the result of the first one with
placeholders on the second one. Gaya uses underscores (`_`) as placeholders.
For example:

```ocaml
(* Output: H *)
io.println(0 |> "Hello"(_)).

(* Output: 69 *)
io.println(42 |> _ + 27).

(* Output: l *)
io.println(1 |> _ + 2 |> "Hello"(_)).

(* Output: Number *)
io.println(1 |> typeof(_)).

(* Output: 5 *)
io.println(5 |> let x = _ in x).
```

It is mandatory to use the placeholder at least once in the right hand side
expression of the pipe operator.
