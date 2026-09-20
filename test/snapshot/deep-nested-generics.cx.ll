
%"S<R>" = type { %"A<A<R>>" }
%"A<A<R>>" = type { %"A<R>" }
%"A<R>" = type { %R }
%R = type { i32 }

define i32 @main() {
  %s = alloca %"S<R>", align 8
  call void @_EN4main1SI1RE4initE(ptr %s)
  call void @_EN4main1SI1RE1sE(ptr %s)
  ret i32 0
}

define void @_EN4main1SI1RE4initE(ptr %this) {
  ret void
}

define void @_EN4main1SI1RE1sE(ptr %this) {
  %t = alloca %"A<R>", align 8
  %tt = alloca %R, align 8
  %a = getelementptr inbounds %"S<R>", ptr %this, i32 0, i32 0
  %1 = call %"A<R>" @_EN4main1AI1AI1REEixE3int(ptr %a, i32 0)
  store %"A<R>" %1, ptr %t, align 4
  %2 = call %R @_EN4main1AI1REixE3int(ptr %t, i32 0)
  store %R %2, ptr %tt, align 4
  %a1 = getelementptr inbounds %"S<R>", ptr %this, i32 0, i32 0
  %3 = call i32 @_EN4main1R1hE(ptr %tt)
  %4 = call %"A<R>" @_EN4main1AI1AI1REEixE3int(ptr %a1, i32 %3)
  ret void
}

define %"A<R>" @_EN4main1AI1AI1REEixE3int(ptr %this, i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %t = getelementptr inbounds %"A<A<R>>", ptr %this, i32 0, i32 0
  %t.load = load %"A<R>", ptr %t, align 4
  ret %"A<R>" %t.load
}

define %R @_EN4main1AI1REixE3int(ptr %this, i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %t = getelementptr inbounds %"A<R>", ptr %this, i32 0, i32 0
  %t.load = load %R, ptr %t, align 4
  ret %R %t.load
}

define i32 @_EN4main1R1hE(ptr %this) {
  %i = getelementptr inbounds %R, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i, align 4
  ret i32 %i.load
}
