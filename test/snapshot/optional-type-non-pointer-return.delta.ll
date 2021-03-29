
%"Optional<int>" = type { i1, i32 }

define %"Optional<int>" @_EN4main1fE() {
  %i = alloca i32, align 4
  %1 = alloca %"Optional<int>", align 8
  store i32 1, i32* %i, align 4
  %i.load = load i32, i32* %i, align 4
  call void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %1, i32 %i.load)
  %.load = load %"Optional<int>", %"Optional<int>"* %1, align 4
  ret %"Optional<int>" %.load
}

define void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %this, i32 %value) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 0
  store i1 true, i1* %hasValue, align 1
  %value1 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 1
  store i32 %value, i32* %value1, align 4
  ret void
}
