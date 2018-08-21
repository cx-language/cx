
%X = type { i32 (i32)* }

define void @_ENM4main1X4initE1pF3int_3int(%X* %this, i32 (i32)* %p) {
  %p1 = getelementptr inbounds %X, %X* %this, i32 0, i32 0
  store i32 (i32)* %p, i32 (i32)** %p1
  ret void
}

define void @_EN4main1X1fE(%X* %this) {
  %p24 = alloca i32 (i32)*
  %p = getelementptr inbounds %X, %X* %this, i32 0, i32 0
  %p1 = load i32 (i32)*, i32 (i32)** %p
  %1 = call i32 %p1(i32 42)
  %p2 = getelementptr inbounds %X, %X* %this, i32 0, i32 0
  %p23 = load i32 (i32)*, i32 (i32)** %p2
  %2 = call i32 %p23(i32 42)
  %p5 = getelementptr inbounds %X, %X* %this, i32 0, i32 0
  %p6 = load i32 (i32)*, i32 (i32)** %p5
  store i32 (i32)* %p6, i32 (i32)** %p24
  %p247 = load i32 (i32)*, i32 (i32)** %p24
  %3 = call i32 %p247(i32 42)
  ret void
}

define i32 @_EN4main3fooE1a3int(i32 %a) {
  ret i32 %a
}

define i32 @main() {
  %x = alloca %X
  call void @_ENM4main1X4initE1pF3int_3int(%X* %x, i32 (i32)* @_EN4main3fooE1a3int)
  %p = getelementptr inbounds %X, %X* %x, i32 0, i32 0
  %p1 = load i32 (i32)*, i32 (i32)** %p
  %1 = call i32 %p1(i32 42)
  call void @_EN4main1X1fE(%X* %x)
  ret i32 0
}
