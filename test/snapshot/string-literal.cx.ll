
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [4 x i8] c"foo\00", align 1

define i32 @main() {
  %s = alloca %string, align 8
  %__str = alloca %string, align 8
  call void @_EN3std6string4initEAU_4char3int(%string* %__str, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  %__str.load = load %string, %string* %__str, align 8
  store %string %__str.load, %string* %s, align 8
  ret i32 0
}

declare void @_EN3std6string4initEAU_4char3int(%string*, i8*, i32)
