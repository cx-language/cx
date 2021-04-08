
%S = type {}
%"Foo<int>" = type {}

define i32 @main() {
  %s = alloca %S, align 8
  %t = alloca %"Foo<int>", align 8
  call void @_EN4main1S4initE(%S* %s)
  call void @_EN4main1S3fooE(%S* %s)
  call void @_EN4main3FooI3intE4initE(%"Foo<int>"* %t)
  call void @_EN4main3FooI3intE3bazE(%"Foo<int>"* %t)
  call void @_EN4main3FooI3intE6deinitE(%"Foo<int>"* %t)
  ret i32 0
}

define void @_EN4main1S4initE(%S* %this) {
  ret void
}

define void @_EN4main1S3fooE(%S* %this) {
  call void @_EN4main1S3barE(%S* %this)
  ret void
}

define void @_EN4main3FooI3intE4initE(%"Foo<int>"* %this) {
  call void @_EN4main3FooI3intE3bazE(%"Foo<int>"* %this)
  ret void
}

define void @_EN4main3FooI3intE3bazE(%"Foo<int>"* %this) {
  call void @_EN4main3FooI3intE3quxE(%"Foo<int>"* %this)
  ret void
}

define void @_EN4main3FooI3intE6deinitE(%"Foo<int>"* %this) {
  call void @_EN4main3FooI3intE3bazE(%"Foo<int>"* %this)
  ret void
}

define void @_EN4main1S3barE(%S* %this) {
  ret void
}

define void @_EN4main3FooI3intE3quxE(%"Foo<int>"* %this) {
  ret void
}
