
%R = type { i32 }
%"S<R>" = type { %"A<A<R>>" }
%"A<A<R>>" = type { %"A<R>" }
%"A<R>" = type { %R }

define i32 @_EN4main1R1hE(%R %this) {
  %i = extractvalue %R %this, 0
  ret i32 %i
}

define i32 @main() {
  %s = alloca %"S<R>"
  call void @_ENM4main1SI1RE4initE(%"S<R>"* %s)
  call void @_EN4main1SI1RE1sE(%"S<R>"* %s)
  ret i32 0
}

define void @_ENM4main1AI1RE4initE(%"A<R>"* %this) {
  ret void
}

define %R @_EN4main1AI1REixE1i3int(%"A<R>"* %this, i32 %i) {
  %t = getelementptr inbounds %"A<R>", %"A<R>"* %this, i32 0, i32 0
  %t.load = load %R, %R* %t
  ret %R %t.load
}

define void @_ENM4main1AI1AI1REE4initE(%"A<A<R>>"* %this) {
  ret void
}

define %"A<R>" @_EN4main1AI1AI1REEixE1i3int(%"A<A<R>>"* %this, i32 %i) {
  %t = getelementptr inbounds %"A<A<R>>", %"A<A<R>>"* %this, i32 0, i32 0
  %t.load = load %"A<R>", %"A<R>"* %t
  ret %"A<R>" %t.load
}

define void @_ENM4main1SI1RE4initE(%"S<R>"* %this) {
  ret void
}

define void @_EN4main1SI1RE1sE(%"S<R>"* %this) {
  %t = alloca %"A<R>"
  %tt = alloca %R
  %a = getelementptr inbounds %"S<R>", %"S<R>"* %this, i32 0, i32 0
  %1 = call %"A<R>" @_EN4main1AI1AI1REEixE1i3int(%"A<A<R>>"* %a, i32 0)
  store %"A<R>" %1, %"A<R>"* %t
  %2 = call %R @_EN4main1AI1REixE1i3int(%"A<R>"* %t, i32 0)
  store %R %2, %R* %tt
  %a1 = getelementptr inbounds %"S<R>", %"S<R>"* %this, i32 0, i32 0
  %tt.load = load %R, %R* %tt
  %3 = call i32 @_EN4main1R1hE(%R %tt.load)
  %4 = call %"A<R>" @_EN4main1AI1AI1REEixE1i3int(%"A<A<R>>"* %a1, i32 %3)
  ret void
}
