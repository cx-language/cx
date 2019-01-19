
%StringRef = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [4 x i8] c"bar\00"

define i32 @main() {
  %b = alloca %StringRef
  %__str0 = alloca %StringRef
  %five = alloca i32
  call void @_EN4main3fooI3intEE1t3int(i32 1)
  call void @_EN4main3fooI4boolEE1t4bool(i1 false)
  call void @_EN4main3fooI4boolEE1t4bool(i1 true)
  call void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef* %__str0, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  %__str0.load = load %StringRef, %StringRef* %__str0
  %1 = call %StringRef @_EN4main3barI9StringRefEE1t9StringRef(%StringRef %__str0.load)
  store %StringRef %1, %StringRef* %b
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

define %StringRef @_EN4main3barI9StringRefEE1t9StringRef(%StringRef %t) {
  ret %StringRef %t
}

declare void @_ENM3std9StringRef4initE7pointerP4char6length4uint(%StringRef*, i8*, i32)

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
