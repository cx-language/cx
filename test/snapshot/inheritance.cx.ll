
%A = type { i32, i32, i1 }
%B = type { i32, i32 }
%O = type { i32, i32 }

define i32 @main() {
  %a = alloca %A, align 8
  %b = alloca %B, align 8
  call void @_EN4main1A4initE4bool3int(ptr %a, i1 false, i32 0)
  call void @_EN4main1B4initE3int(ptr %b, i32 4)
  ret i32 0
}

define void @_EN4main1A4initE4bool3int(ptr %this, i1 %b, i32 %p) {
  %b1 = alloca i1, align 1
  %p2 = alloca i32, align 4
  store i1 %b, ptr %b1, align 1
  store i32 %p, ptr %p2, align 4
  %p.load = load i32, ptr %p2, align 4
  call void @_EN4main1O4initE3int(ptr %this, i32 %p.load)
  %b3 = getelementptr inbounds %A, ptr %this, i32 0, i32 2
  %b.load = load i1, ptr %b1, align 1
  store i1 %b.load, ptr %b3, align 1
  ret void
}

define void @_EN4main1B4initE3int(ptr %this, i32 %p) {
  %p1 = alloca i32, align 4
  store i32 %p, ptr %p1, align 4
  %p2 = getelementptr inbounds %B, ptr %this, i32 0, i32 0
  %p.load = load i32, ptr %p1, align 4
  store i32 %p.load, ptr %p2, align 4
  %r = getelementptr inbounds %B, ptr %this, i32 0, i32 1
  %p.load3 = load i32, ptr %p1, align 4
  store i32 %p.load3, ptr %r, align 4
  ret void
}

define void @_EN4main1O4initE3int(ptr %this, i32 %p) {
  %p1 = alloca i32, align 4
  store i32 %p, ptr %p1, align 4
  %p2 = getelementptr inbounds %O, ptr %this, i32 0, i32 0
  %p.load = load i32, ptr %p1, align 4
  store i32 %p.load, ptr %p2, align 4
  %r = getelementptr inbounds %O, ptr %this, i32 0, i32 1
  %p.load3 = load i32, ptr %p1, align 4
  store i32 %p.load3, ptr %r, align 4
  ret void
}
