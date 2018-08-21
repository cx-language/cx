
%String = type { %"Array<char>" }
%"Array<char>" = type { i8*, i32, i32 }
%StringRef = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer

define i32 @main() {
  %a = alloca %String
  %__str0 = alloca %StringRef
  call void @_ENM3std6String4initE(%String* %a)
  %a1 = load %String, %String* %a
  call void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef* %__str0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0), i32 0)
  %__str02 = load %StringRef, %StringRef* %__str0
  %1 = call %String @_EN3stdplE1a6String1b9StringRef(%String %a1, %StringRef %__str02)
  store %String %1, %String* %a
  call void @_ENM3std6String6deinitE(%String* %a)
  ret i32 0
}

declare void @_ENM3std6String6deinitE(%String*)

declare void @_ENM3std6String4initE(%String*)

declare %String @_EN3stdplE1a6String1b9StringRef(%String, %StringRef)

declare void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef*, i8*, i32)
