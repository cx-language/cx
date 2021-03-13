
define void @_EN4main3fooEF3int3int_3int(i32 (i32, i32)* %a) {
  %1 = call i32 %a(i32 1, i32 2)
  ret void
}

define i32 @main() {
  call void @_EN4main3fooEF3int3int_3int(i32 (i32, i32)* @_EN4main9__lambda0E3int3int)
  call void @_EN4main3fooEF3int3int_3int(i32 (i32, i32)* @_EN4main9__lambda1E3int3int)
  call void @_EN4main3fooEF3int3int_3int(i32 (i32, i32)* @_EN4main9__lambda2E3int3int)
  call void @_EN4main3fooEF3int3int_3int(i32 (i32, i32)* @_EN4main9__lambda3E3int3int)
  ret i32 0
}

define i32 @_EN4main9__lambda0E3int3int(i32 %a, i32 %b) {
  %1 = add i32 %a, %b
  ret i32 %1
}

define i32 @_EN4main9__lambda1E3int3int(i32 %a, i32 %b) {
  %1 = add i32 %a, %b
  ret i32 %1
}

define i32 @_EN4main9__lambda2E3int3int(i32 %a, i32 %b) {
  %1 = add i32 %a, %b
  ret i32 %1
}

define i32 @_EN4main9__lambda3E3int3int(i32 %a, i32 %b) {
  %x = alloca i32, align 4
  %1 = add i32 %a, %b
  store i32 %1, i32* %x, align 4
  %x.load = load i32, i32* %x, align 4
  ret i32 %x.load
}
