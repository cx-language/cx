
%StringRef = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }
%String = type { %"Array<char>" }
%"Array<char>" = type { i8*, i32, i32 }

@0 = private unnamed_addr constant [4 x i8] c"foo\00"
@1 = private unnamed_addr constant [6 x i8] c"%.*s\0A\00"

define i32 @main() {
  %__str0 = alloca %StringRef
  call void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef* %__str0, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  call void @_EN3std5printI9StringRefEE5valueP9StringRef(%StringRef* %__str0)
  ret i32 0
}

define void @_EN3std5printI9StringRefEE5valueP9StringRef(%StringRef* %value) {
  %string = alloca %String
  %1 = load %StringRef, %StringRef* %value
  %2 = call %String @_EN3std9StringRef8toStringE(%StringRef %1)
  store %String %2, %String* %string
  %3 = call i32 @_EN3std6String4sizeE(%String* %string)
  %4 = call i8* @_EN3std6String4dataE(%String* %string)
  %5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i32 %3, i8* %4)
  call void @_ENM3std6String6deinitE(%String* %string)
  ret void
}

declare void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef*, i8*, i32)

declare void @_ENM3std6String6deinitE(%String*)

declare %String @_EN3std9StringRef8toStringE(%StringRef)

declare i32 @printf(i8*, ...)

declare i32 @_EN3std6String4sizeE(%String*)

declare i8* @_EN3std6String4dataE(%String*)
