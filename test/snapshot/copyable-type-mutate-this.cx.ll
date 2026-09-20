
%S = type { i32 }

define i32 @main() {
  %1 = alloca %S, align 8
  call void @_EN4main1S4initE3int(ptr %1, i32 1)
  call void @_EN4main1S1sE(ptr %1)
  ret i32 0
}

define void @_EN4main1S4initE3int(ptr %this, i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i1, align 4
  store i32 %i.load, ptr %i2, align 4
  ret void
}

define void @_EN4main1S1sE(ptr %this) {
  %i = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  store i32 2, ptr %i, align 4
  ret void
}
