
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }
%StringBuffer = type { %"List<char>" }
%"List<char>" = type { i8*, i32, i32 }

@0 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"%.*s\0A\00", align 1

define i32 @main() {
  %__str0 = alloca %string
  call void @_EN3std6string4initE7pointerP4char6length3int(%string* %__str0, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  call void @_EN3std5printI6stringEE5valueP6string(%string* %__str0)
  ret i32 0
}

define void @_EN3std5printI6stringEE5valueP6string(%string* %value) {
  %string = alloca %StringBuffer
  %1 = call %StringBuffer @_EN3std6string8toStringE(%string* %value)
  store %StringBuffer %1, %StringBuffer* %string
  %2 = call i32 @_EN3std12StringBuffer4sizeE(%StringBuffer* %string)
  %3 = call i8* @_EN3std12StringBuffer4dataE(%StringBuffer* %string)
  %4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i32 %2, i8* %3)
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %string)
  ret void
}

declare void @_EN3std6string4initE7pointerP4char6length3int(%string*, i8*, i32)

declare void @_EN3std12StringBuffer6deinitE(%StringBuffer*)

declare %StringBuffer @_EN3std6string8toStringE(%string*)

declare i32 @printf(i8*, ...)

declare i32 @_EN3std12StringBuffer4sizeE(%StringBuffer*)

declare i8* @_EN3std12StringBuffer4dataE(%StringBuffer*)
