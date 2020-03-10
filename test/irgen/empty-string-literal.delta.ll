
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define i32 @main() {
  %__str0 = alloca %string
  call void @_EN3std6string4initE7pointerP4char6length3int(%string* %__str0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0), i32 0)
  %1 = call i32 @_EN3std6string4sizeE(%string* %__str0)
  ret i32 %1
}

declare i32 @_EN3std6string4sizeE(%string*)

define void @_EN3std6string4initE7pointerP4char6length3int(%string* %this, i8* %pointer, i32 %length) {
  %1 = alloca %"ArrayRef<char>"
  %characters = getelementptr inbounds %string, %string* %this, i32 0, i32 0
  call void @_EN3std8ArrayRefI4charE4initE4dataP4char4size3int(%"ArrayRef<char>"* %1, i8* %pointer, i32 %length)
  %.load = load %"ArrayRef<char>", %"ArrayRef<char>"* %1
  store %"ArrayRef<char>" %.load, %"ArrayRef<char>"* %characters
  ret void
}

declare void @_EN3std8ArrayRefI4charE4initE4dataP4char4size3int(%"ArrayRef<char>"*, i8*, i32)
