
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [4 x i8] c"bar\00", align 1

define i32 @main() {
  %b = alloca %string
  %__str0 = alloca %string
  %five = alloca i32
  call void @_EN4main3fooI3intEE1t3int(i32 1)
  call void @_EN4main3fooI4boolEE1t4bool(i1 false)
  call void @_EN4main3fooI4boolEE1t4bool(i1 true)
  call void @_EN3std6string4initE7pointerP4char6length3int(%string* %__str0, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  %__str0.load = load %string, %string* %__str0
  %1 = call %string @_EN4main3barI6stringEE1t6string(%string %__str0.load)
  store %string %1, %string* %b
  %2 = call i32 @_EN4main3quxI3intEE1t3int(i32 -5)
  store i32 %2, i32* %five
  ret i32 0
}

define void @_EN4main3fooI3intEE1t3int(i32 %t) {
  ret void
}

define void @_EN4main3fooI4boolEE1t4bool(i1 %t) {
  ret void
}

define %string @_EN4main3barI6stringEE1t6string(%string %t) {
  ret %string %t
}

define void @_EN3std6string4initE7pointerP4char6length3int(%string* %this, i8* %pointer, i32 %length) {
  %1 = alloca %"ArrayRef<char>"
  %characters = getelementptr inbounds %string, %string* %this, i32 0, i32 0
  call void @_EN3std8ArrayRefI4charE4initE4dataP4char4size3int(%"ArrayRef<char>"* %1, i8* %pointer, i32 %length)
  %.load = load %"ArrayRef<char>", %"ArrayRef<char>"* %1
  store %"ArrayRef<char>" %.load, %"ArrayRef<char>"* %characters
  ret void
}

define i32 @_EN4main3quxI3intEE1t3int(i32 %t) {
  %1 = icmp slt i32 %t, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = sub i32 0, %t
  ret i32 %2

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  ret i32 %t
}

declare void @_EN3std8ArrayRefI4charE4initE4dataP4char4size3int(%"ArrayRef<char>"*, i8*, i32)
