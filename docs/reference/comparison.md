# Comparison

It is undefined behaviour to use comparison operators other than `==` and `/=`
between objects of different types.

Note that "not equals" is `/=` instead of `!=`.

Otherwise, comparison operators work as you'd expect in most languages.

```ocaml
"Hello" < "Zap" (* t *)
42 >= 42        (* t *)
"str" == 42     (* f *)
```
