
%X = type { i32, i32* }
%Empty = type {}
%"Generic<float>" = type { float }
%"Generic<Empty>" = type { %Empty }

define i32 @main() {
  %b = alloca i32, align 4
  %x = alloca %X, align 8
  %y = alloca %X, align 8
  %e = alloca %Empty, align 8
  %g = alloca %"Generic<float>", align 8
  %h = alloca %"Generic<Empty>", align 8
  store i32 2, i32* %b, align 4
  call void @_EN4main1X4initE3intP3int(%X* %x, i32 4, i32* %b)
  call void @_EN4main1X4initE3intP3int(%X* %y, i32 4, i32* %b)
  call void @_EN4main5Empty4initE(%Empty* %e)
  call void @_EN4main7GenericI5floatE4initE5float(%"Generic<float>"* %g, float 4.500000e+00)
  %e.load = load %Empty, %Empty* %e, align 1
  call void @_EN4main7GenericI5EmptyE4initE5Empty(%"Generic<Empty>"* %h, %Empty %e.load)
  call void @_EN4main7GenericI5EmptyE6deinitE(%"Generic<Empty>"* %h)
  ret i32 0
}

define void @_EN4main1X4initE3intP3int(%X* %this, i32 %a, i32* %b) {
  %a1 = getelementptr inbounds %X, %X* %this, i32 0, i32 0
  store i32 %a, i32* %a1, align 4
  %b2 = getelementptr inbounds %X, %X* %this, i32 0, i32 1
  store i32* %b, i32** %b2, align 8
  ret void
}

define void @_EN4main5Empty4initE(%Empty* %this) {
  ret void
}

define void @_EN4main7GenericI5floatE4initE5float(%"Generic<float>"* %this, float %i) {
  %i1 = getelementptr inbounds %"Generic<float>", %"Generic<float>"* %this, i32 0, i32 0
  store float %i, float* %i1, align 4
  ret void
}

define void @_EN4main7GenericI5EmptyE4initE5Empty(%"Generic<Empty>"* %this, %Empty %i) {
  %i1 = getelementptr inbounds %"Generic<Empty>", %"Generic<Empty>"* %this, i32 0, i32 0
  store %Empty %i, %Empty* %i1, align 1
  ret void
}

define void @_EN4main7GenericI5EmptyE6deinitE(%"Generic<Empty>"* %this) {
  %i = getelementptr inbounds %"Generic<Empty>", %"Generic<Empty>"* %this, i32 0, i32 0
  call void @_EN4main5Empty6deinitE(%Empty* %i)
  ret void
}

define void @_EN4main5Empty6deinitE(%Empty* %this) {
  ret void
}
