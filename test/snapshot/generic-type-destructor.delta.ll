
define i32 @main() {
  %i = alloca {}, align 8
  %b = alloca {}, align 8
  call void @_EN4main1CI3intE4initE({}* %i)
  call void @_EN4main1CI4boolE4initE({}* %b)
  call void @_EN4main1CI4boolE6deinitE({}* %b)
  call void @_EN4main1CI3intE6deinitE({}* %i)
  ret i32 0
}

define void @_EN4main1CI3intE4initE({}* %this) {
  ret void
}

define void @_EN4main1CI4boolE4initE({}* %this) {
  ret void
}

define void @_EN4main1CI4boolE6deinitE({}* %this) {
  ret void
}

define void @_EN4main1CI3intE6deinitE({}* %this) {
  ret void
}
