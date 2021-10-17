
%"Optional<int>" = type { i1, i32 }

define i32 @main() {
  %a = alloca %"Optional<int>", align 8
  %1 = alloca %"Optional<int>", align 8
  %2 = alloca i32, align 4
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %1)
  %.load = load %"Optional<int>", %"Optional<int>"* %1, align 4
  store %"Optional<int>" %.load, %"Optional<int>"* %a, align 4
  store i32 1, i32* %2, align 4
  %3 = call i1 @_EN3stdeqI3intEERO3intR3int(%"Optional<int>"* %a, i32* %2)
  ret i32 0
}

define void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %this) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 0
  store i1 false, i1* %hasValue, align 1
  ret void
}

define i1 @_EN3stdeqI3intEERO3intR3int(%"Optional<int>"* %a, i32* %b) {
  %1 = alloca %"Optional<int>", align 8
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %1)
  %2 = call i1 @_EN3stdeqI3intEERO3intRO3int(%"Optional<int>"* %a, %"Optional<int>"* %1)
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %value = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %a, i32 0, i32 1
  %value.load = load i32, i32* %value, align 4
  %b.load = load i32, i32* %b, align 4
  %3 = icmp eq i32 %value.load, %b.load
  ret i1 %3
}

define i1 @_EN3stdeqI3intEERO3intRO3int(%"Optional<int>"* %a, %"Optional<int>"* %b) {
  %1 = alloca %"Optional<int>", align 8
  %2 = alloca %"Optional<int>", align 8
  %3 = alloca %"Optional<int>", align 8
  %4 = alloca %"Optional<int>", align 8
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %1)
  %5 = call i1 @_EN3stdeqI3intEERO3intRO3int(%"Optional<int>"* %a, %"Optional<int>"* %1)
  br i1 %5, label %and.rhs, label %and.end

and.rhs:                                          ; preds = %0
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %2)
  %6 = call i1 @_EN3stdeqI3intEERO3intRO3int(%"Optional<int>"* %b, %"Optional<int>"* %2)
  br label %and.end

and.end:                                          ; preds = %and.rhs, %0
  %and = phi i1 [ %5, %0 ], [ %6, %and.rhs ]
  br i1 %and, label %if.then, label %if.else

if.then:                                          ; preds = %and.end
  ret i1 true

if.else:                                          ; preds = %and.end
  br label %if.end

if.end:                                           ; preds = %if.else
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %3)
  %7 = call i1 @_EN3stdeqI3intEERO3intRO3int(%"Optional<int>"* %a, %"Optional<int>"* %3)
  br i1 %7, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.end
  ret i1 false

if.else2:                                         ; preds = %if.end
  br label %if.end3

if.end3:                                          ; preds = %if.else2
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %4)
  %8 = call i1 @_EN3stdeqI3intEERO3intRO3int(%"Optional<int>"* %b, %"Optional<int>"* %4)
  br i1 %8, label %if.then4, label %if.else5

if.then4:                                         ; preds = %if.end3
  ret i1 false

if.else5:                                         ; preds = %if.end3
  br label %if.end6

if.end6:                                          ; preds = %if.else5
  %value = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %a, i32 0, i32 1
  %value.load = load i32, i32* %value, align 4
  %value7 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %b, i32 0, i32 1
  %value.load8 = load i32, i32* %value7, align 4
  %9 = icmp eq i32 %value.load, %value.load8
  ret i1 %9
}
