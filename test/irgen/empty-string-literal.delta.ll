
%StringRef = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer

define i32 @main() {
  %__str0 = alloca %StringRef
  call void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef* %__str0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0), i32 0)
  %__str0.load = load %StringRef, %StringRef* %__str0
  %1 = call i32 @_EN3std9StringRef4sizeE(%StringRef %__str0.load)
  ret i32 %1
}

declare i32 @_EN3std9StringRef4sizeE(%StringRef)

declare void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef*, i8*, i32)
