
%StringRef = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [3 x i8] c"\5Cn\00"

define i32 @main() {
  %__str0 = alloca %StringRef
  call void @_ENM3std9StringRef4initE7pointerP4char6length3int(%StringRef* %__str0, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i32 2)
  ret i32 0
}

declare void @_ENM3std9StringRef4initE7pointerP4char6length3int(%StringRef*, i8*, i32)
