
define i32 @main() {
  call void @_EN4main3fooI3intEEF3int3int_3int(ptr @_EN4main9__lambda0E3int3int)
  call void @_EN4main3fooI3intEEF3int3int_3int(ptr @_EN4main9__lambda1E3int3int)
  call void @_EN4main3fooI3intEEF3int3int_3int(ptr @_EN4main9__lambda2E3int3int)
  call void @_EN4main3fooI3intEEF3int3int_3int(ptr @_EN4main9__lambda3E3int3int)
  call void @_EN4main3fooI4voidEEF3int3int_4void(ptr @_EN4main9__lambda4E3int3int)
  ret i32 0
}

define void @_EN4main3fooI3intEEF3int3int_3int(ptr %a) {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  %1 = call i32 %a.load(i32 1, i32 2)
  ret void
}

define i32 @_EN4main9__lambda0E3int3int(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = add i32 %a.load, %b.load
  ret i32 %1
}

define i32 @_EN4main9__lambda1E3int3int(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = add i32 %a.load, %b.load
  ret i32 %1
}

define i32 @_EN4main9__lambda2E3int3int(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = icmp sgt i32 %a.load, %b.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %a.load3 = load i32, ptr %a1, align 4
  %b.load4 = load i32, ptr %b2, align 4
  %2 = add i32 %a.load3, %b.load4
  ret i32 %2

if.else:                                          ; preds = %0
  %a.load5 = load i32, ptr %a1, align 4
  %b.load6 = load i32, ptr %b2, align 4
  %3 = sub i32 %a.load5, %b.load6
  ret i32 %3
}

define i32 @_EN4main9__lambda3E3int3int(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = add i32 %a.load, %b.load
  store i32 %1, ptr %x, align 4
  %x.load = load i32, ptr %x, align 4
  ret i32 %x.load
}

define void @_EN4main3fooI4voidEEF3int3int_4void(ptr %a) {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  call void %a.load(i32 1, i32 2)
  ret void
}

define void @_EN4main9__lambda4E3int3int(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = icmp eq i32 %a.load, %b.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  ret void
}
