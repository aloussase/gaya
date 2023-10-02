# Structs

Structs are data structures similar to dictionaries with a pre-defined set of
fields.

```julia
struct Person
  name: String
  age:  Number
end
```

Here we are declaring a `Person` struct with two fields: name, which should be
a `String`, and age, which should be a `Number`. The types of the fields are
checked at runtime upon construction and assignment.

## Constructing Structs

To create an instance of a struct, you use a function of the same name as your
struct:

```
p :: Person("John", 42)
```

If an argument of the wrong type is provided to the constructor, a runtime
error will occur.

## Accessing Struct Fields

You can access the fields of a struct using the `@` operator:

```
assert(p@name == "John")
```

## Mutating Structs

Struct fields may be assigned new values using the assignment operator, similar
to how it works with dictionaries.

```
p@name <- "Jane"
```

Again, the type of the new value is checked at runtime.
