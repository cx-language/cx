# Enum types

C*'s enums are scoped and can optionally have associated values for each enum value.

```cs
enum JSONValue {
    String(StringBuffer value),
    Array(List<JSONValue> value),
    Object(Map<StringBuffer, JSONValue> value),
    Number(float64 value),
    Boolean(bool value),
    Null,
}

void main() {
    JSONValue a = JSONValue.Array(List([
        JSONValue.Boolean(false), 
        JSONValue.Number(42), 
        JSONValue.Null
    ]));

    switch (a) {
        case JSONValue.String: println("it's a string");
        case JSONValue.Array: println("it's an array");
        case JSONValue.Object: println("it's an object");
        case JSONValue.Number: println("it's a number");
        case JSONValue.Boolean: println("it's a boolean");
        case JSONValue.Null: println("it's null");
    }
}
```
