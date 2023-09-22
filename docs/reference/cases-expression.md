# Cases Expression

`cases` expressions are Gaya's take on if statements. Basically, the following
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

With the major difference that `cases` is an expression, so you can use it as
value. For example, a function that returns the greater of two numbers may be
written like this:

```ocaml
max :: { x, y =>
  cases
    given x > y => x
    otherwise   => y
  end
}
```
