
%StringRef = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [4 x i8] c"foo\00"

define i32 @main() {
  %s = alloca %StringRef
  %__str0 = alloca %StringRef
  call void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef* %__str0, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  %__str0.load = load %StringRef, %StringRef* %__str0
  store %StringRef %__str0.load, %StringRef* %s
  ret i32 0
}

declare void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef*, i8*, i32)
