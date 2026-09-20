
%A = type { [2 x i1] }
%B = type { i8 }

@i = private global i32 42

define ptr @_EN4main3fooE() {
  %a = alloca ptr, align 8
  %1 = alloca %A, align 8
  %b = alloca ptr, align 8
  %2 = alloca %B, align 8
  call void @_EN4main1A4initEA2_4bool(ptr %1, [2 x i1] [i1 false, i1 true])
  %3 = call ptr @_EN4main1A2ffE(ptr %1)
  store ptr %3, ptr %a, align 8
  call void @_EN4main1B4initE4char(ptr %2, i8 97)
  %4 = call ptr @_EN4main1B2ggE(ptr %2)
  store ptr %4, ptr %b, align 8
  ret ptr @i
}

define void @_EN4main1A4initEA2_4bool(ptr %this, [2 x i1] %b) {
  %b1 = alloca [2 x i1], align 1
  store [2 x i1] %b, ptr %b1, align 1
  %b2 = getelementptr inbounds %A, ptr %this, i32 0, i32 0
  %b.load = load [2 x i1], ptr %b1, align 1
  store [2 x i1] %b.load, ptr %b2, align 1
  ret void
}

define ptr @_EN4main1A2ffE(ptr %this) {
  %b = getelementptr inbounds %A, ptr %this, i32 0, i32 0
  %1 = getelementptr inbounds [2 x i1], ptr %b, i32 0, i32 1
  ret ptr %1
}

define void @_EN4main1B4initE4char(ptr %this, i8 %a) {
  %a1 = alloca i8, align 1
  store i8 %a, ptr %a1, align 1
  %a2 = getelementptr inbounds %B, ptr %this, i32 0, i32 0
  %a.load = load i8, ptr %a1, align 1
  store i8 %a.load, ptr %a2, align 1
  ret void
}

define ptr @_EN4main1B2ggE(ptr %this) {
  %a = getelementptr inbounds %B, ptr %this, i32 0, i32 0
  ret ptr %a
}
