# Include Statements

`include` is used to evaluate the contents of a file in the interpreter's
environment. This is basically the poor man's imports, but it works good
enough.

If a file is included more than once, its contents will be evaluated only one.

For example, in A.gaya:

```
io.println("From A").
```

And in B.gaya:

```
include "A"
include "A"
```

Will only print "From A" once.

The argument to include is a string that represents a relative path to a file
in the same directory of the executing script, with or without the '.gaya'
extension, or the name of a bundled library, like "strings" or "sequences".
