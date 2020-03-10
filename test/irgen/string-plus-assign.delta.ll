
%StringBuffer = type { %"List<char>" }
%"List<char>" = type { i8*, i32, i32 }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define i32 @main() {
  %a = alloca %StringBuffer
  %__str0 = alloca %string
  call void @_EN3std12StringBuffer4initE(%StringBuffer* %a)
  %a.load = load %StringBuffer, %StringBuffer* %a
  call void @_EN3std6string4initE7pointerP4char6length3int(%string* %__str0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0), i32 0)
  %__str0.load = load %string, %string* %__str0
  %1 = call %StringBuffer @_EN3stdplE1a12StringBuffer1b6string(%StringBuffer %a.load, %string %__str0.load)
  store %StringBuffer %1, %StringBuffer* %a
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %a)
  ret i32 0
}

declare void @_EN3std12StringBuffer6deinitE(%StringBuffer*)

declare void @_EN3std12StringBuffer4initE(%StringBuffer*)

declare %StringBuffer @_EN3stdplE1a12StringBuffer1b6string(%StringBuffer, %string)

define void @_EN3std6string4initE7pointerP4char6length3int(%string* %this, i8* %pointer, i32 %length) {
  %1 = alloca %"ArrayRef<char>"
  %characters = getelementptr inbounds %string, %string* %this, i32 0, i32 0
  call void @_EN3std8ArrayRefI4charE4initE4dataP4char4size3int(%"ArrayRef<char>"* %1, i8* %pointer, i32 %length)
  %.load = load %"ArrayRef<char>", %"ArrayRef<char>"* %1
  store %"ArrayRef<char>" %.load, %"ArrayRef<char>"* %characters
  ret void
}

declare void @_EN3std8ArrayRefI4charE4initE4dataP4char4size3int(%"ArrayRef<char>"*, i8*, i32)
