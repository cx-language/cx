
%B = type { i32 }
%A = type { %B, %B, i32 }
%C = type { %B, %B }

declare i1 @f()

define void @_ENM4main1B4initE(%B* %this) {
  %i = getelementptr inbounds %B, %B* %this, i32 0, i32 0
  store i32 0, i32* %i
  ret void
}

define void @_ENM4main1B6deinitE(%B* %this) {
  ret void
}

define void @_ENM4main1A4initE(%A* %this) {
  ret void
}

define void @_ENM4main1A6deinitE(%A* %this) {
  %b = getelementptr inbounds %A, %A* %this, i32 0, i32 0
  %bb = getelementptr inbounds %A, %A* %this, i32 0, i32 1
  %1 = call i1 @f()
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  call void @_ENM4main1B6deinitE(%B* %bb)
  call void @_ENM4main1B6deinitE(%B* %b)
  ret void

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else
  call void @_ENM4main1B6deinitE(%B* %bb)
  call void @_ENM4main1B6deinitE(%B* %b)
  ret void
}

define void @_ENM4main1C4initE(%C* %this) {
  ret void
}

define void @_ENM4main1C6deinitE(%C* %this) {
  %b = getelementptr inbounds %C, %C* %this, i32 0, i32 0
  %bb = getelementptr inbounds %C, %C* %this, i32 0, i32 1
  call void @_ENM4main1B6deinitE(%B* %bb)
  call void @_ENM4main1B6deinitE(%B* %b)
  ret void
}

define i32 @main() {
  %a = alloca %A
  %c = alloca %C
  call void @_ENM4main1A4initE(%A* %a)
  call void @_ENM4main1C4initE(%C* %c)
  call void @_ENM4main1C6deinitE(%C* %c)
  call void @_ENM4main1A6deinitE(%A* %a)
  ret i32 0
}
