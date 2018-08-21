
%"Range<int>" = type { i32, i32 }

define void @_EN4main1fE1r5RangeI3intE(%"Range<int>" %r) {
  ret void
}

define i32 @main() {
  %foo = alloca i32
  %bar = alloca i32
  %1 = alloca %"Range<int>"
  store i32 0, i32* %foo
  store i32 0, i32* %bar
  %foo1 = load i32, i32* %foo
  %bar2 = load i32, i32* %bar
  call void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 %foo1, i32 %bar2)
  %2 = load %"Range<int>", %"Range<int>"* %1
  call void @_EN4main1fE1r5RangeI3intE(%"Range<int>" %2)
  ret i32 0
}

declare void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"*, i32, i32)
