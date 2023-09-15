# Documentation of the Core Language

## Table of Contents

- [Comments](#comments)
- [Literals](#literals)
- [Arithmetic](#arithmetic)
- [`discard` and `perform`](#discard-and-perform)
- [Truthiness](#truthiness)
- [Comparison](#comparison)
- [Functions](#functions)
- [Let Expression](#let-expression)
- [Cases Expression](#cases-expression)
- [Do Expression](#do-expression)
- [While loops](#while-loops)
- [Assignment](#assignment)
- [Closures](#closures)
- [Pipe Operator](#pipe-operator)

## Comments <a name="comments" />

Comments follow OCaml style

```ocaml
(* This is a comment *)

(*
  Works across multiple lines as well.
*)
```

## Literals <a name="literals">

```ocaml
 (* string *)
"Hello world"

(* numbers are C++ doubles *)
42
42.69
-42

(* Booleans are C style ;) *)

(* Unit *)
unit
```

## Arithmetic <a name="arithmetic" />

Arithmetic operations work as you would exepct in most programming languages:

```ocaml
42 + 27 (* 69 *)
70 - 1  (* 69 *)
69 * 1  (* 69 *)

(* Division by zero returns unit. *)
let result = 69 / 0 in
cases
  given result == unit => io.println("Oops")
  otherwise            => io.println(result)
end

(* Associativy and precedence works as expected *)
42 + 14 * 2 - 1 (* 69 *)
```

## `discard` and `perform` <a name="discard-and-perform" />

`discard` is used to _discard_ the result of an expression, which is mandatory
if you are not using it.

On the other hand, `perform` allows you to _perform_ a single statement in an
expression context, returning `unit`.

This is useful in control structures that expect statements or expression, but
you need to do the other. For example, `let` expects what comes after `in` to be
an expression, but it might be useful to put a statement instead. This can
be done in two ways.

First, with a `do` block:

```julia
let x = 0 in do
  while x < 10
    x <- x + 1
  end
end
```

Second, with `perform`:

```julia
let x = 0 in perform
while x < 10
  x <- x + 1
end
```

Both are semantically equivalent.

## Truthiness <a name="truthiness" />

All values but the empty string, empty array, unit and zero are considered to
be truthy.

```ocaml
test :: { x =>
  cases
    given x   => io.println("t")
    otherwise => io.println("f")
  end
}

discard test(42)    (* Truthy *)
discard test("42")  (* Truthy *)
discard test(unit)  (* Falsey *)
discard test("")    (* Falsey *)
discard test(0)     (* Falsey *)
discard test(())    (* Falsey *)
```

## Comparison <a name="comparison" />

Objects of different types cannot be compared except with the `==` and `/=`
operators, in which case they are not considered equal. Otherwise, comparison
operators work as you'd expect in most languages.

```ocaml
"Hello" < "Zap" (* t *)
42 >= 42        (* t *)
"str" == 42     (* f *)
```

## Functions <a name="functions" />

Functions are defined between curly braces. Their syntax is as follows:

```
{ <params> => <body> }
```

Where `<params>` is a comma separated list of identifiers and `<body>` is a
single expression. For example:

```ocaml
discard { => io.println("Gaya is cool!") }() (* Output: Gaya is cool *)

(* Multiple statments *)
discard { => do
    discard io.println("Gaya")
    discard io.println("is")
    discard io.println("cool!")
  end
}() (* Output: Gaya\nis\ncool! *)
```

In this examples we are immediatly invoking the function, but you can also
store it for later:

```ocaml
add :: { x, y => x + y }
discard add(1, 2) (* Output: 3 *)
```

## Let Expression <a name="let-expression">

`let` expressions are used to introduce local bindings. Its syntax is:

```
let <bindings> in <body>
```

Where `<bindings>` is a comma separated list of `<identifier> = <expression>`
and `<body>` is a single expression. Each binding in the bindings list is available
to the next one, so you can do things like:

```ocaml
let x = 5, y = x + 1 in io.println(y) (* Output: 6 *)
```

## Cases Expression <a name="cases-expression" />

`cases` expressions is Gaya's take on if statements. Basically, the following
C code:

```cpp
int x = 2;
if (x == 0) /* ... */
else if (x == 1) /* ... */
else /* ... */
```

translates to the following Gaya:

```ocaml
let x = 2 in
cases
  given x == 0 => (* ... *)
  given x == 1 => (* ... *)
  otherwise    => (* ... *)
end
```

With the major difference that `cases` is an expression, so you can use its
value. For example, a function that return the greater of two numbers may be
written like this:

```ocaml
max :: { x, y =>
  cases
    given x > y => x
    otherwise   => y
  end
}
```

## Do Expression <a name="do-expression" />

A `do` expression is simply a way to group statements. All but the last element
in the body must statements. If the last element is an expression, its value is
return, else, if it's a statement, unit is returned.

```ocaml
(* Results in 42*)
do
  discard io.println("A statement")
  42
end

(* Results in unit *)
do
  discard let x = 0 in perform
  while x < 10
    x <- x + 1
  end
end
```

## While loops <a name="while-loops">

While loops work as you'd expect in other programming languages:

```ocaml
let x = 0 in perform
while x < 10
  discard io.println(x)
  x <- x + 1
end
```

An optional continuation statement may be put after the while condition like in
Zig. So the last example could be rewritten to:

```ocaml
let x = 0 in perform
while x < 10 : x <- x + 1
  discard io.println(x)
end
```

Note that the body of the while loop should consist entirely of statements.
This means that if you call a function, you need to discard its result
explicitely if you are not going to use it.

## Assignment <a name="assignment" />

The assignment operator `<-` can be used to mutate local variables (those
introduced with `let`). It cannot be used on global definitions of function
parameters.

```ocaml
discard let x = 0 in do
  x <- 10
  io.println(x)
end
```

It can only be used in _local statements_, meaning `do` blocks, the body of
`while` loops, and `perform`.

## Closures <a name="closures" />

Closure are functions that capture their environment. For a thorough
introduction to closures, check out
[this](https://en.wikipedia.org/wiki/Closure\_(computer_programming)) wikipedia
article.

An example from that article translated to Gaya:

```ocaml
f :: { x =>
  { y => x + y }
}

a :: f(1)

discard io.println(a(5)) (* Output: 6 *)
```

In this example `f` returns a closure that captures the parameter `x`.

The combination of closures and assignment allows us to simulate stateful
objects:

```ocaml
mkCounter :: { =>
  let count = 0 in { =>
    let previous = count in do
      count <- count + 1
      previous
    done
  }
}

counter :: mkCounter()

discard io.println(counter()) (* Output: 0 *)
discard io.println(counter()) (* Output: 1 *)
```

Really cool ;). See the `object.gaya` example in the `examples` directory of
the repository's root to see how we can simulate message passing using just
closures.

## Pipe Operator <a name="pipe-operator">

Gaya implements [Hack style
pipes](https://docs.hhvm.com/hack/expressions-and-operators/pipe). You can pipe
any expression into another one and replace the result of the first one with
placeholders on the second one. Gaya uses underscores (`_`) as placeholders.
For example:

```ocaml
(* Output: H *)
discard io.println(0 |> "Hello"(_))

(* Output: 69 *)
discard io.println(42 |> _ + 27)

(* Output: l *)
discard io.println(1 |> _ + 2 |> "Hello"(_))

(* Output: number *)
discard io.println(1 |> typeof(_))

(* Output: 5 *)
discard io.println(5 |> let x = _ in x)
```
