# Ranges

Ranges in Delta are created with the syntax `start..end` or `start...end`, the
latter being a closed range.

If a function takes a `Range` as input, one can use the short form to
initialize it without calling a constructor for the class. For example, the
[`substr`](https://github.com/delta-lang/delta/blob/863938ae9b7f3e472d9a0d85351824f379e4a0c9/stdlib/String.delta#L94)
function takes a `Range` as input, so we can use it like this:

```swift
var s = String("String");
print(s.substr(0..2));
// => St
```


