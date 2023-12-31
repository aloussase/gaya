# Match Expression

Match expressions are cases expression on steriods. They bring to Gaya a simple
form of pattern matching like the one you find in programming languages like
Rust or OCaml.

```
test :: { xs =>
  cases xs
    given x when x == 42   => "life is" <> x
    given (x, _, (42, 69)) => x
    given ((42, 69), y, _) => y
    otherwise              => "dunno"
  end
}
```

After the `cases` keyword goes the _target_ against which you want to match.
Then comes a series of match branches. Each branch consists of a pattern,
followed by an optional `when` clause, followed the branch value.

A branch is considered to match is the pattern matches and the conditon, if
given, is truthy.

An optional `otherwise` branch may be given as a fallback in case no other
branch matches. In the case that no branch matches and no `otherwise` branch is
given, the result of the match expression is `unit`.

## Patterns

A pattern can be one of the following:

- **A wildcard**: '\_' which matches everything.
- **An identifier**: for example 'x', which also matches everything but
  additionally binds the target to the name of the identifier.
- **An array pattern**: which is an arbitrarily nested series of
  comma-separated patterns surrounded by '(' and ')', like and array. This will
  match if the target is an array and every one of its elements matches the
  corresponding pattern.
- **An arbitrary expression**: which will be compared for equality with the
  target.

Sometimes you may want to match an array and also capture the whole array. You
can do this with _as patterns_:

```
cases someFunc()
  given xs@(x, y, z) => (* do something *)
end
```

You introduce an as-pattern by using an identifier followed by an `@` before
the pattern. The whole match will be available under the name of the
identifier.

Finally, patterns can be used in a number of places in Gaya, such as in let
expressions and function parameters. See the corresponding chapters for more on
that.
