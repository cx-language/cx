
%StringBuffer = type { %"List<char>" }
%"List<char>" = type { ptr, i32, i32 }

@my_global = external global i32
@0 = private unnamed_addr constant [5 x i8] c"%.*s\00", align 1

define i32 @main() {
  %my_global.load = load i32, ptr @my_global, align 4
  call void @_EN3std5printI3intEE3int(i32 %my_global.load)
  ret i32 0
}

define void @_EN3std5printI3intEE3int(i32 %value) {
  %value1 = alloca i32, align 4
  %s = alloca %StringBuffer, align 8
  store i32 %value, ptr %value1, align 4
  %1 = call %StringBuffer @_EN3std3int8toStringE(ptr %value1)
  store %StringBuffer %1, ptr %s, align 8
  %2 = call i32 @_EN3std12StringBuffer4sizeE(ptr %s)
  %3 = call ptr @_EN3std12StringBuffer4dataE(ptr %s)
  %4 = call i32 (ptr, ...) @printf(ptr @0, i32 %2, ptr %3)
  call void @_EN3std12StringBuffer6deinitE(ptr %s)
  ret void
}

declare %StringBuffer @_EN3std3int8toStringE(ptr)

declare i32 @_EN3std12StringBuffer4sizeE(ptr)

declare ptr @_EN3std12StringBuffer4dataE(ptr)

declare i32 @printf(ptr, ...)

declare void @_EN3std12StringBuffer6deinitE(ptr)
