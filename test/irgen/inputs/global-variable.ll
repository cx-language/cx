@foo = private global i32 42
@qux = private global i32 42

define void @a() {
  store i32 43, i32* @foo
  ret void
}

define void @c() {
  %d = alloca i32
  store i32 42, i32* %d
  ret void
}
