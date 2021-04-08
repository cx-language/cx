
%"S<R>" = type { %"A<A<R>>" }
%"A<A<R>>" = type { %"A<R>" }
%"A<R>" = type { %R }
%R = type { i32 }

define i32 @main() {
  %s = alloca %"S<R>", align 8
  call void @_EN4main1SI1RE4initE(%"S<R>"* %s)
  call void @_EN4main1SI1RE1sE(%"S<R>"* %s)
  ret i32 0
}

define void @_EN4main1SI1RE4initE(%"S<R>"* %this) {
  ret void
}

define void @_EN4main1SI1RE1sE(%"S<R>"* %this) {
  %t = alloca %"A<R>", align 8
  %tt = alloca %R, align 8
  %a = getelementptr inbounds %"S<R>", %"S<R>"* %this, i32 0, i32 0
  %1 = call %"A<R>" @_EN4main1AI1AI1REEixE3int(%"A<A<R>>"* %a, i32 0)
  store %"A<R>" %1, %"A<R>"* %t, align 4
  %2 = call %R @_EN4main1AI1REixE3int(%"A<R>"* %t, i32 0)
  store %R %2, %R* %tt, align 4
  %a1 = getelementptr inbounds %"S<R>", %"S<R>"* %this, i32 0, i32 0
  %3 = call i32 @_EN4main1R1hE(%R* %tt)
  %4 = call %"A<R>" @_EN4main1AI1AI1REEixE3int(%"A<A<R>>"* %a1, i32 %3)
  ret void
}

define %"A<R>" @_EN4main1AI1AI1REEixE3int(%"A<A<R>>"* %this, i32 %i) {
  %t = getelementptr inbounds %"A<A<R>>", %"A<A<R>>"* %this, i32 0, i32 0
  %t.load = load %"A<R>", %"A<R>"* %t, align 4
  ret %"A<R>" %t.load
}

define %R @_EN4main1AI1REixE3int(%"A<R>"* %this, i32 %i) {
  %t = getelementptr inbounds %"A<R>", %"A<R>"* %this, i32 0, i32 0
  %t.load = load %R, %R* %t, align 4
  ret %R %t.load
}

define i32 @_EN4main1R1hE(%R* %this) {
  %i = getelementptr inbounds %R, %R* %this, i32 0, i32 0
  %i.load = load i32, i32* %i, align 4
  ret i32 %i.load
}
