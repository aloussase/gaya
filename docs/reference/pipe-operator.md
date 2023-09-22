# Pipe Operator

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

(* Output: number *)
io.println(1 |> typeof(_)).

(* Output: 5 *)
io.println(5 |> let x = _ in x).
```

It is mandatory to use the placeholder at least once in the right hand side
expression of the pipe operator.
