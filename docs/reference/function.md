# Functions

Gaya has first-class support for functions. Functions are defined between curly
braces:

```
{ name => io.println("Hello, " <> name) }

{ => io.println("Gaya is cool!") }(). (* Output: Gaya is cool *)
```

As you can see, the parameters go before the `=>` and the body should be a
single expression. If you need to put more, you can use a do-block. Note that
the `=>` is mandatory even if your function does not have any parameters.

```
(* Multiple statments *)
{ => do
    io.println("Gaya").
    io.println("is").
    io.println("cool!").
  end
}(). (* Output: Gaya\nis\ncool! *)
```

In this example we are immediatly invoking the function, but you can also store
it for later:

```
add :: { x, y => x + y }
add(1, 2). (* Output: 3 *)
```

`::` is used to declare variables at the top level. You could also store it in
a local binding using a `let` expression, since functions are first-class
citizens in Gaya.

## Trailing functions

If the last argument to a function call is itself a function, you can put it
outside the parenthesis (a so called _trailing function_):

```
(1, 2, 3, 4)
  |> seq.takewhile(_) { x => x < 4 }
  |> seq.dropwhile(_) { x => x < 2 }
  |> seq.toarray(_)
  |> assert(_ == (2, 3)).
```

**NOTE** To use the functions in this snippet, you need to `include
"sequences"`. More on include statements and sequences later.

## Default arguments

Functions may also have default arguments. If present, default arguments should
go at the end of the parameter list.

```
greet :: { name = "Gaya" =>
  "Hello " <> name <> "!"
}
```

## Typing parameters

Function parameters may be given type hints. These will be used to check at
runtime that the type of the argument is correct. If it isn't, a runtime error
will occur. This can be considered a nicer way of doing `assert(typeof(param)
== "SomeType")`. For example:

```
sum :: { x: Number, y: Number =>
  x + y
}
```

Possible types are:

- Any
- Array
- Dictionary
- Function
- Number
- Sequence
- String
- Unit

The type `Sequence` denotes any sequence-convertible type, such as `String` and
`Number`.

You can also define your custom types with a _type declaration_:

```
type NonNegative of Number with _ > 0
```

Here we are defining a type `NonNegative` that is a subset of `Number` that
also checks at runtime that the given number is greater than zero. The
constraint given after `with` can be an arbitrary expression. The value of the
argument will be available as `_`.

## Patterns as parameters

The parameters of a function can be an arbitrary pattern, not just identifiers.
You can use this feature, for example, to destructure an array parameter:

```
(1 -> 2, 3 -> 3)
  |> seq.foreach(_) { (key, value) => io.println(tostring(key) <> ":" <> value) }
```

See the section on match expression for a more detailed introduction to
pattern matching in Gaya.
