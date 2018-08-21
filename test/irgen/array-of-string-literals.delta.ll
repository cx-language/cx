
%StringRef = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [4 x i8] c"foo\00"
@1 = private unnamed_addr constant [4 x i8] c"bar\00"

define i32 @main() {
  %a = alloca [2 x %StringRef]
  %__str0 = alloca %StringRef
  %__str1 = alloca %StringRef
  call void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef* %__str0, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  %1 = load %StringRef, %StringRef* %__str0
  %2 = insertvalue [2 x %StringRef] undef, %StringRef %1, 0
  call void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef* %__str1, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @1, i32 0, i32 0), i32 3)
  %3 = load %StringRef, %StringRef* %__str1
  %4 = insertvalue [2 x %StringRef] %2, %StringRef %3, 1
  store [2 x %StringRef] %4, [2 x %StringRef]* %a
  ret i32 0
}

declare void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef*, i8*, i32)
