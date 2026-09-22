
%S = type { { i32 } }

define i32 @main() {
  %s = alloca %S, align 8
  %f = alloca ptr, align 8
  %g = alloca ptr, align 8
  call void @_EN4main1S4initET3int_(ptr %s, { i32 } { i32 1 })
  %1 = call ptr @_EN4main1S1fE(ptr %s)
  store ptr %1, ptr %f, align 8
  %2 = call ptr @_EN4main1S1gE(ptr %s)
  store ptr %2, ptr %g, align 8
  ret i32 0
}

define void @_EN4main1S4initET3int_(ptr %this, { i32 } %t) {
  %t1 = alloca { i32 }, align 8
  store { i32 } %t, ptr %t1, align 4
  %t2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %t.load = load { i32 }, ptr %t1, align 4
  store { i32 } %t.load, ptr %t2, align 4
  ret void
}

define ptr @_EN4main1S1fE(ptr %this) {
  %t = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  ret ptr %t
}

define ptr @_EN4main1S1gE(ptr %this) {
  %t = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  ret ptr %t
}
