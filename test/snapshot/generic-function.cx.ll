
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { ptr, i32 }

@0 = private unnamed_addr constant [4 x i8] c"bar\00", align 1

define i32 @main() {
  %b = alloca %string, align 8
  %__str = alloca %string, align 8
  %five = alloca i32, align 4
  call void @_EN4main3fooI3intEE3int(i32 1)
  call void @_EN4main3fooI4boolEE4bool(i1 false)
  call void @_EN4main3fooI4boolEE4bool(i1 true)
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @0, i32 3)
  %__str.load = load %string, ptr %__str, align 8
  %1 = call %string @_EN4main3barI6stringEE6string(%string %__str.load)
  store %string %1, ptr %b, align 8
  %2 = call i32 @_EN4main3quxI3intEE3int(i32 -5)
  store i32 %2, ptr %five, align 4
  ret i32 0
}

define void @_EN4main3fooI3intEE3int(i32 %t) {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  ret void
}

define void @_EN4main3fooI4boolEE4bool(i1 %t) {
  %t1 = alloca i1, align 1
  store i1 %t, ptr %t1, align 1
  ret void
}

declare void @_EN3std6string4initEP4char3int(ptr, ptr, i32)

define %string @_EN4main3barI6stringEE6string(%string %t) {
  %t1 = alloca %string, align 8
  store %string %t, ptr %t1, align 8
  %t.load = load %string, ptr %t1, align 8
  ret %string %t.load
}

define i32 @_EN4main3quxI3intEE3int(i32 %t) {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  %t.load = load i32, ptr %t1, align 4
  %1 = icmp slt i32 %t.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %t.load2 = load i32, ptr %t1, align 4
  %2 = sub i32 0, %t.load2
  ret i32 %2

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %t.load3 = load i32, ptr %t1, align 4
  ret i32 %t.load3
}
