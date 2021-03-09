
%S = type { i32 }

@i = private global i32 0

define i32 @main() {
  %i = alloca i32, align 4
  %s = alloca %S, align 8
  store i32 0, i32* %i, align 4
  call void @_EN4main1S4initE(%S* %s)
  call void @_EN4main1S1fE(%S* %s)
  ret i32 0
}

define void @_EN4main1S4initE(%S* %this) {
  %i = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store i32 0, i32* %i, align 4
  ret void
}

define void @_EN4main1S1fE(%S* %this) {
  %i = alloca i32, align 4
  store i32 0, i32* %i, align 4
  ret void
}
