
%vec2 = type { i32, i32 }

define void @_ENM4main4vec24initE1x3int1y3int(%vec2* %this, i32 %x, i32 %y) {
  %x1 = getelementptr inbounds %vec2, %vec2* %this, i32 0, i32 0
  store i32 %x, i32* %x1
  %y2 = getelementptr inbounds %vec2, %vec2* %this, i32 0, i32 1
  store i32 %y, i32* %y2
  ret void
}

define i32 @_EN4main4vec2ixE5index3int(%vec2 %this, i32 %index) {
  ret i32 %index
}

define %vec2 @_EN4mainmlE1a4vec21b4vec2(%vec2 %a, %vec2 %b) {
  %1 = alloca %vec2
  %x = extractvalue %vec2 %a, 0
  %x1 = extractvalue %vec2 %b, 0
  %2 = mul i32 %x, %x1
  %y = extractvalue %vec2 %a, 1
  %y2 = extractvalue %vec2 %b, 1
  %3 = mul i32 %y, %y2
  call void @_ENM4main4vec24initE1x3int1y3int(%vec2* %1, i32 %2, i32 %3)
  %4 = load %vec2, %vec2* %1
  ret %vec2 %4
}

define i32 @main() {
  %v = alloca %vec2
  %1 = alloca %vec2
  %2 = alloca %vec2
  call void @_ENM4main4vec24initE1x3int1y3int(%vec2* %v, i32 3, i32 2)
  call void @_ENM4main4vec24initE1x3int1y3int(%vec2* %1, i32 2, i32 4)
  %3 = load %vec2, %vec2* %1
  %v1 = load %vec2, %vec2* %v
  %4 = call %vec2 @_EN4mainmlE1a4vec21b4vec2(%vec2 %3, %vec2 %v1)
  store %vec2 %4, %vec2* %v
  %v2 = load %vec2, %vec2* %v
  call void @_ENM4main4vec24initE1x3int1y3int(%vec2* %2, i32 -1, i32 3)
  %5 = load %vec2, %vec2* %2
  %6 = call i1 @_EN4maineqE1a4vec21b4vec2(%vec2 %v2, %vec2 %5)
  %v3 = load %vec2, %vec2* %v
  %x = getelementptr inbounds %vec2, %vec2* %v, i32 0, i32 0
  %7 = load i32, i32* %x
  %8 = call i32 @_EN4main4vec2ixE5index3int(%vec2 %v3, i32 %7)
  ret i32 0
}

define i1 @_EN4maineqE1a4vec21b4vec2(%vec2 %a, %vec2 %b) {
  %x = extractvalue %vec2 %a, 0
  %x1 = extractvalue %vec2 %b, 0
  %1 = icmp eq i32 %x, %x1
  ret i1 %1
}
