
%"A<C>" = type {}
%C = type { i32 }

define i32 @main() {
  %a = alloca %"A<C>", align 8
  call void @_EN4main1AI1CE4initE(%"A<C>"* %a)
  ret i32 0
}

define void @_EN4main1AI1CE4initE(%"A<C>"* %this) {
  %t = alloca %C, align 8
  call void @_EN4main1C4initE(%C* %t)
  ret void
}

define void @_EN4main1C4initE(%C* %this) {
  %i = getelementptr inbounds %C, %C* %this, i32 0, i32 0
  store i32 0, i32* %i, align 4
  ret void
}
