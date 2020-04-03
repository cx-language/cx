
%"Optional<int>" = type { i32, i1 }

define i32 @main() {
  %a = alloca %"Optional<int>"
  %1 = alloca %"Optional<int>"
  %b = alloca %"Optional<int>"
  %c = alloca %"Optional<int>"
  %2 = alloca %"Optional<int>"
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %1)
  %.load = load %"Optional<int>", %"Optional<int>"* %1
  store %"Optional<int>" %.load, %"Optional<int>"* %a
  call void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %b, i32 2)
  call void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %2, i32 3)
  %.load1 = load %"Optional<int>", %"Optional<int>"* %2
  store %"Optional<int>" %.load1, %"Optional<int>"* %c
  ret i32 0
}

define void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %this) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 1
  store i1 false, i1* %hasValue
  ret void
}

define void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %this, i32 %value) {
  %value1 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 0
  store i32 %value, i32* %value1
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 1
  store i1 true, i1* %hasValue
  ret void
}
