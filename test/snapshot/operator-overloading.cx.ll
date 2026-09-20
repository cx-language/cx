
%vec2 = type { i32, i32 }

define %vec2 @_EN4mainmlE4vec24vec2(%vec2 %a, %vec2 %b) {
  %a1 = alloca %vec2, align 8
  %b2 = alloca %vec2, align 8
  %1 = alloca %vec2, align 8
  store %vec2 %a, ptr %a1, align 4
  store %vec2 %b, ptr %b2, align 4
  %x = getelementptr inbounds %vec2, ptr %a1, i32 0, i32 0
  %x.load = load i32, ptr %x, align 4
  %x3 = getelementptr inbounds %vec2, ptr %b2, i32 0, i32 0
  %x.load4 = load i32, ptr %x3, align 4
  %2 = mul i32 %x.load, %x.load4
  %y = getelementptr inbounds %vec2, ptr %a1, i32 0, i32 1
  %y.load = load i32, ptr %y, align 4
  %y5 = getelementptr inbounds %vec2, ptr %b2, i32 0, i32 1
  %y.load6 = load i32, ptr %y5, align 4
  %3 = mul i32 %y.load, %y.load6
  call void @_EN4main4vec24initE3int3int(ptr %1, i32 %2, i32 %3)
  %.load = load %vec2, ptr %1, align 4
  ret %vec2 %.load
}

define void @_EN4main4vec24initE3int3int(ptr %this, i32 %x, i32 %y) {
  %x1 = alloca i32, align 4
  %y2 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  store i32 %y, ptr %y2, align 4
  %x3 = getelementptr inbounds %vec2, ptr %this, i32 0, i32 0
  %x.load = load i32, ptr %x1, align 4
  store i32 %x.load, ptr %x3, align 4
  %y4 = getelementptr inbounds %vec2, ptr %this, i32 0, i32 1
  %y.load = load i32, ptr %y2, align 4
  store i32 %y.load, ptr %y4, align 4
  ret void
}

define i32 @main() {
  %v = alloca %vec2, align 8
  %1 = alloca %vec2, align 8
  %2 = alloca %vec2, align 8
  call void @_EN4main4vec24initE3int3int(ptr %v, i32 3, i32 2)
  call void @_EN4main4vec24initE3int3int(ptr %1, i32 2, i32 4)
  %.load = load %vec2, ptr %1, align 4
  %v.load = load %vec2, ptr %v, align 4
  %3 = call %vec2 @_EN4mainmlE4vec24vec2(%vec2 %.load, %vec2 %v.load)
  store %vec2 %3, ptr %v, align 4
  %v.load1 = load %vec2, ptr %v, align 4
  call void @_EN4main4vec24initE3int3int(ptr %2, i32 -1, i32 3)
  %.load2 = load %vec2, ptr %2, align 4
  %4 = call i1 @_EN4maineqE4vec24vec2(%vec2 %v.load1, %vec2 %.load2)
  %x = getelementptr inbounds %vec2, ptr %v, i32 0, i32 0
  %x.load = load i32, ptr %x, align 4
  %5 = call i32 @_EN4main4vec2ixE3int(ptr %v, i32 %x.load)
  ret i32 0
}

define i1 @_EN4maineqE4vec24vec2(%vec2 %a, %vec2 %b) {
  %a1 = alloca %vec2, align 8
  %b2 = alloca %vec2, align 8
  store %vec2 %a, ptr %a1, align 4
  store %vec2 %b, ptr %b2, align 4
  %x = getelementptr inbounds %vec2, ptr %a1, i32 0, i32 0
  %x.load = load i32, ptr %x, align 4
  %x3 = getelementptr inbounds %vec2, ptr %b2, i32 0, i32 0
  %x.load4 = load i32, ptr %x3, align 4
  %1 = icmp eq i32 %x.load, %x.load4
  ret i1 %1
}

define i32 @_EN4main4vec2ixE3int(ptr %this, i32 %index) {
  %index1 = alloca i32, align 4
  store i32 %index, ptr %index1, align 4
  %index.load = load i32, ptr %index1, align 4
  ret i32 %index.load
}
