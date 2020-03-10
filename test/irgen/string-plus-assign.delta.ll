
%StringBuffer = type { %"List<char>" }
%"List<char>" = type { i8*, i32, i32 }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define i32 @main() {
  %a = alloca %StringBuffer
  %__str4 = alloca %string
  call void @_EN3std12StringBuffer4initE(%StringBuffer* %a)
  %a.load = load %StringBuffer, %StringBuffer* %a
  call void @_EN3std6string4initE7pointerP4char6length3int(%string* %__str4, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0), i32 0)
  %__str4.load = load %string, %string* %__str4
  %1 = call %StringBuffer @_EN3stdplE1a12StringBuffer1b6string(%StringBuffer %a.load, %string %__str4.load)
  store %StringBuffer %1, %StringBuffer* %a
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %a)
  ret i32 0
}

declare void @_EN3std12StringBuffer6deinitE(%StringBuffer*)

declare void @_EN3std12StringBuffer4initE(%StringBuffer*)

declare %StringBuffer @_EN3stdplE1a12StringBuffer1b6string(%StringBuffer, %string)

declare void @_EN3std6string4initE7pointerP4char6length3int(%string*, i8*, i32)
