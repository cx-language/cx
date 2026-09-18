# Enum types

C*'s enums are scoped and can optionally have associated values for each enum value.

```cs
enum JsonValue {
    String(StringBuffer value),
    Array(List<JsonValue> value),
    Object(Map<StringBuffer, JsonValue> value),
    Number(float64 value),
    Boolean(bool value),
    Null,
}

void main() {
    JsonValue a = JsonValue.Array(List([
        JsonValue.Boolean(false), 
        JsonValue.Number(42), 
        JsonValue.Null
    ]));

    switch (a) {
        case JsonValue.String: println("it's a string");
        case JsonValue.Array: println("it's an array");
        case JsonValue.Object: println("it's an object");
        case JsonValue.Number: println("it's a number");
        case JsonValue.Boolean: println("it's a boolean");
        case JsonValue.Null: println("it's null");
    }
}
```
