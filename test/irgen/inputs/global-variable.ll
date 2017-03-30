@foo = private global i32 42
@baz = private constant i32 42
@qux = private global i32 42
@bar = private constant i32 42

define void @a() {
  %b = alloca i32
  %foo = load i32, i32* @foo
  store i32 %foo, i32* %b
  ret void
}

define void @c() {
  %d = alloca i32
  %bar = load i32, i32* @bar
  store i32 %bar, i32* %d
  ret void
}
