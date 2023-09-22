# Truthiness

All values but the empty string, empty array, empty dictionary, unit and zero
are considered to be truthy.

```
test :: { x =>
  cases
    given x   => io.println("t")
    otherwise => io.println("f")
  end
}

test(42).    (* t *)
test("42").  (* t *)
test(unit).  (* f *)
test("").    (* f *)
test(0).     (* f *)
test(()).    (* f *)
```
