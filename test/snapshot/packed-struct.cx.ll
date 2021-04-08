
%S = type <{ i8, i16, i8 }>

@0 = private unnamed_addr constant [43 x i8] c"Assertion failed at packed-struct.cx:11:5\0A\00", align 1

define i32 @main() {
  %s = alloca %S, align 8
  %a = getelementptr inbounds %S, %S* %s, i32 0, i32 0
  store i8 -85, i8* %a, align 1
  %b = getelementptr inbounds %S, %S* %s, i32 0, i32 1
  store i16 -12817, i16* %b, align 2
  %c = getelementptr inbounds %S, %S* %s, i32 0, i32 2
  store i8 0, i8* %c, align 1
  br i1 icmp ne (i64 ptrtoint (%S* getelementptr (%S, %S* null, i32 1) to i64), i64 4), label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @0, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  ret i32 0
}

declare void @_EN3std10assertFailEP4char(i8*)
