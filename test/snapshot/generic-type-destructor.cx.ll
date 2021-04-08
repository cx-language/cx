
%"C<int>" = type {}
%"C<bool>" = type {}

define i32 @main() {
  %i = alloca %"C<int>", align 8
  %b = alloca %"C<bool>", align 8
  call void @_EN4main1CI3intE4initE(%"C<int>"* %i)
  call void @_EN4main1CI4boolE4initE(%"C<bool>"* %b)
  call void @_EN4main1CI4boolE6deinitE(%"C<bool>"* %b)
  call void @_EN4main1CI3intE6deinitE(%"C<int>"* %i)
  ret i32 0
}

define void @_EN4main1CI3intE4initE(%"C<int>"* %this) {
  ret void
}

define void @_EN4main1CI4boolE4initE(%"C<bool>"* %this) {
  ret void
}

define void @_EN4main1CI4boolE6deinitE(%"C<bool>"* %this) {
  ret void
}

define void @_EN4main1CI3intE6deinitE(%"C<int>"* %this) {
  ret void
}
