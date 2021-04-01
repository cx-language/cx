
define i32 @main() {
  call void @_EN4main3fooI3intEEF3int3int_3int(i32 (i32, i32)* @_EN4main9__lambda0E3int3int)
  call void @_EN4main3fooI3intEEF3int3int_3int(i32 (i32, i32)* @_EN4main9__lambda1E3int3int)
  call void @_EN4main3fooI3intEEF3int3int_3int(i32 (i32, i32)* @_EN4main9__lambda2E3int3int)
  call void @_EN4main3fooI3intEEF3int3int_3int(i32 (i32, i32)* @_EN4main9__lambda3E3int3int)
  call void @_EN4main3fooI4voidEEF3int3int_4void(void (i32, i32)* @_EN4main9__lambda4E3int3int)
  ret i32 0
}

define void @_EN4main3fooI3intEEF3int3int_3int(i32 (i32, i32)* %a) {
  %1 = call i32 %a(i32 1, i32 2)
  ret void
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
  %1 = icmp sgt i32 %a, %b
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = add i32 %a, %b
  ret i32 %2

if.else:                                          ; preds = %0
  %3 = sub i32 %a, %b
  ret i32 %3
}

define i32 @_EN4main9__lambda3E3int3int(i32 %a, i32 %b) {
  %x = alloca i32, align 4
  %1 = add i32 %a, %b
  store i32 %1, i32* %x, align 4
  %x.load = load i32, i32* %x, align 4
  ret i32 %x.load
}

define void @_EN4main3fooI4voidEEF3int3int_4void(void (i32, i32)* %a) {
  call void %a(i32 1, i32 2)
  ret void
}

define void @_EN4main9__lambda4E3int3int(i32 %a, i32 %b) {
  %1 = icmp eq i32 %a, %b
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  ret void
}
