
%"Optional<int>" = type { i1, i32 }

define i32 @main() {
  %a = alloca %"Optional<int>", align 8
  %1 = alloca %"Optional<int>", align 8
  %2 = alloca i32, align 4
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %1)
  %.load = load %"Optional<int>", %"Optional<int>"* %1, align 4
  store %"Optional<int>" %.load, %"Optional<int>"* %a, align 4
  store i32 1, i32* %2, align 4
  %3 = call i1 @_EN3stdeqI3intEEPO3intP3int(%"Optional<int>"* %a, i32* %2)
  ret i32 0
}

define void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %this) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 0
  store i1 false, i1* %hasValue, align 1
  ret void
}

define i1 @_EN3stdeqI3intEEPO3intP3int(%"Optional<int>"* %a, i32* %b) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %a, i32 0, i32 0
  %hasValue.load = load i1, i1* %hasValue, align 1
  %1 = xor i1 %hasValue.load, true
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %value = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %a, i32 0, i32 1
  %value.load = load i32, i32* %value, align 4
  %b.load = load i32, i32* %b, align 4
  %2 = icmp eq i32 %value.load, %b.load
  ret i1 %2
}
