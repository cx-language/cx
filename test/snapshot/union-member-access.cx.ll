
%U = type <{ i32 }>

@0 = private unnamed_addr constant [50 x i8] c"integer overflow at union-member-access.cx:13:19\0A\00", align 1

define i32 @_EN4main3fooE1U(%U %u) {
  %u1 = alloca %U, align 8
  store %U %u, ptr %u1, align 1
  %b.load = load i32, ptr %u1, align 4
  ret i32 %b.load
}

define i32 @main() {
  %u = alloca %U, align 8
  store i32 21, ptr %u, align 4
  %u.load = load %U, ptr %u, align 1
  %1 = call i32 @_EN4main3fooE1U(%U %u.load)
  %b.load = load i32, ptr %u, align 4
  %2 = sext i32 %1 to i128
  %3 = sext i32 %b.load to i128
  %4 = add i128 %2, %3
  %5 = trunc i128 %4 to i32
  %6 = sext i32 %5 to i128
  %7 = icmp ne i128 %4, %6
  %8 = xor i1 %7, true
  %overflow.condition = icmp eq i1 %8, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %0
  ret i32 %5
}

declare void @_EN3std10assertFailEP4char(ptr)
