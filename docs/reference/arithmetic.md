## Arithmetic

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
