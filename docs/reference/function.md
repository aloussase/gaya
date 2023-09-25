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

If the last argument to a function call is itself a function, you can
put it outside the parenthesis (a so called _trailing function_):

```
(1, 2, 3, 4)
  |> seq.takewhile(_) { x => x < 4 }
  |> seq.dropwhile(_) { x => x < 2 }
  |> seq.toarray(_)
  |> assert(_ == (2, 3)).
```

**NOTE** To use the functions in this snippet, you need to `include
"sequences"`. More on include statements and sequences later.

Functions may also have default arguments. If present, default arguments should
go at the end of the parameter list.

```
greet :: { name = "Gaya" =>
  "Hello " <> name <> "!"
}
```

Finally, the parameters of a function can be an arbitrary pattern, not just
identifiers. You can use this feature, for example, to destructure an array
parameter:

```
(1 -> 2, 3 -> 3)
  |> seq.foreach(_) { (key, value) => io.println(tostring(key) <> ":" <> value) }
```

See the section on match expression for a more detailed introduction to
pattern matching in Gaya.
