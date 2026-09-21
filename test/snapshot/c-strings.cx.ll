
%S = type { [1024 x i8] }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { ptr, i32 }
%StringBuffer = type { %"List<char>" }
%"List<char>" = type { ptr, i32, i32 }

@0 = private unnamed_addr constant [2 x i8] c"x\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"x\00", align 1

define i32 @main() {
  %s = alloca %S, align 8
  %__str = alloca %string, align 8
  %1 = alloca %StringBuffer, align 8
  %__str1 = alloca %string, align 8
  %a = getelementptr inbounds %S, ptr %s, i32 0, i32 0
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @0, i32 1)
  %__str.load = load %string, ptr %__str, align 8
  %2 = call i1 @_EN3stdeqEP4char6string(ptr %a, %string %__str.load)
  %a2 = getelementptr inbounds %S, ptr %s, i32 0, i32 0
  call void @_EN3std6string4initEP4char3int(ptr %__str1, ptr @1, i32 1)
  %__str.load3 = load %string, ptr %__str1, align 8
  call void @_EN3std12StringBuffer4initE6string(ptr %1, %string %__str.load3)
  %3 = getelementptr inbounds %StringBuffer, ptr %1, i32 0, i32 0
  %4 = getelementptr inbounds %"List<char>", ptr %3, i32 0, i32 0
  %.load = load ptr, ptr %4, align 8
  %5 = getelementptr inbounds %"List<char>", ptr %3, i32 0, i32 1
  %.load4 = load i32, ptr %5, align 4
  %6 = sub i32 %.load4, 1
  %7 = insertvalue %"ArrayRef<char>" undef, ptr %.load, 0
  %8 = insertvalue %"ArrayRef<char>" %7, i32 %6, 1
  %9 = insertvalue %string undef, %"ArrayRef<char>" %8, 0
  %10 = call i1 @_EN3stdeqEP4char6string(ptr %a2, %string %9)
  %11 = xor i1 %10, true
  ret i32 0
}

declare void @_EN3std6string4initEP4char3int(ptr, ptr, i32)

declare i1 @_EN3stdeqEP4char6string(ptr, %string)

declare void @_EN3std12StringBuffer4initE6string(ptr, %string)
