# Enums

Enums are C style enumerations.

```julia
enum Color
  red
  green
  blue
end

myColor :: Color@red

assert(myColor == Color@red).
```

They can be used as keys in dictionaries:

```
colors :: (->)

dict.set(colors, Color@red, 0xff0000).
dict.set(colors, Color@green, 0x00ff00).
dict.set(colors, Color@blue, 0x0000ff).
```
