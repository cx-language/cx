
%"Optional<ArrayRef<int8>>" = type { i1, %"ArrayRef<int8>" }
%"ArrayRef<int8>" = type { i8*, i32 }

define i32 @main() {
  %a = alloca %"Optional<ArrayRef<int8>>", align 8
  %1 = alloca %"Optional<ArrayRef<int8>>", align 8
  call void @_EN3std8OptionalI8ArrayRefI4int8EE4initE(%"Optional<ArrayRef<int8>>"* %1)
  %.load = load %"Optional<ArrayRef<int8>>", %"Optional<ArrayRef<int8>>"* %1, align 8
  store %"Optional<ArrayRef<int8>>" %.load, %"Optional<ArrayRef<int8>>"* %a, align 8
  ret i32 0
}

define void @_EN3std8OptionalI8ArrayRefI4int8EE4initE(%"Optional<ArrayRef<int8>>"* %this) {
  %hasValue = getelementptr inbounds %"Optional<ArrayRef<int8>>", %"Optional<ArrayRef<int8>>"* %this, i32 0, i32 0
  store i1 false, i1* %hasValue, align 1
  ret void
}
