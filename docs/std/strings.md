# Strings

```
include "strings"
```

### `string.length`

Return the length of the string.

```
@param s <string> The string for which to calculate the length.
```

### `string.concat`

Return the concatenation of the second string onto the first one.

**NOTE:** This function mutates its first argument.

```
@param s1 <string> The first string.
@param s2 <string> The string to tack onto the end of the first one.
```

### `string.tonumber`

Convert a string to a number.

Unit is returned if the conversion failed.

```
@param s <string> The string to try to parse as a number.
```

### `string.index`

Return the index in the string where the provided pattern starts, or unit if
the pattern is not contained in the string.

```
@param haystack <string> The string in which to search for the pattern.
@param needle <string> The string to search for.
```

### `string.substring`

Return a substring of the given string.

If finish < start or start < 0 or finish > string.length(s), unit
is returned instead.

```
@param s <string> The string from which to extract the substring.
@param start <number> The start of the substring.
@param finish <number> The end of the substring.
```

### `string.startswith`

Return whether a string starts with the given pattern.

```
@param s <string> The string to test.
@param pattern <string> The pattern to search for in s.
```

### `string.endswith`

Return whether a string ends with the given pattern.

```
@param s <string> The string to test.
@param pattern <string> The pattern to search for in s.
```

### `string.trim`

Return a new string that is the result of trimming the leading and trailing
whitespace from the provided string.

```
@param s <string> The string to trim.
```

### `string.isempty`

Return whether the provided string is empty or not.

### `string.first`

Return the first character in a string, or unit if empty.

### `string.last`

Return the last character in a string, or unit if empty.

### `string.contains`

Return whether the provided pattern is contained in s.

```
@param s <string> The string to test for pattern.
@param pattern <string> The pattern to look for in s.
```

### `string.split`

Splits a string on the given pattern and returns a sequence over the parts.

```
@param s <string> The string to split.
@param pattern <string> The pattern to split the string on.
```

### `string.iswhitespace`

Return whether a string is a whitespace character.

```
@param s <string> The string to test.
```
