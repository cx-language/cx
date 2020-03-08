
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [4 x i8] c"foo\00", align 1

define i32 @main() {
  %s = alloca %string
  %__str0 = alloca %string
  call void @_EN3std6string4initE7pointerP4char6length3int(%string* %__str0, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  %__str0.load = load %string, %string* %__str0
  store %string %__str0.load, %string* %s
  ret i32 0
}

declare void @_EN3std6string4initE7pointerP4char6length3int(%string*, i8*, i32)
