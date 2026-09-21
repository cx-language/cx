
%0 = type <{ { i32 } }>
%"Optional<int>" = type { i32, %0 }

@0 = private unnamed_addr constant [36 x i8] c"Unwrap failed at Optional.cx:17:16\0A\00", align 1

define i32 @main() {
  %a = alloca %"Optional<int>", align 8
  %enum = alloca %"Optional<int>", align 8
  %1 = alloca i32, align 4
  %tag = getelementptr inbounds %"Optional<int>", ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %enum.load = load %"Optional<int>", ptr %enum, align 4
  store %"Optional<int>" %enum.load, ptr %a, align 4
  store i32 1, ptr %1, align 4
  %2 = call i1 @_EN3stdeqI3intEERO3intR3int(ptr %a, ptr %1)
  ret i32 0
}

define i1 @_EN3stdeqI3intEERO3intR3int(ptr %a, ptr %b) {
  %a1 = alloca ptr, align 8
  %b2 = alloca ptr, align 8
  %1 = alloca %"Optional<int>", align 8
  store ptr %a, ptr %a1, align 8
  store ptr %b, ptr %b2, align 8
  %a.load = load ptr, ptr %a1, align 8
  %a.load.load = load %"Optional<int>", ptr %a.load, align 4
  %2 = extractvalue %"Optional<int>" %a.load.load, 0
  %3 = icmp eq i32 %2, 1
  %4 = xor i1 %3, true
  br i1 %4, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %a.load3 = load ptr, ptr %a1, align 8
  %a.load.load4 = load %"Optional<int>", ptr %a.load3, align 4
  %5 = extractvalue %"Optional<int>" %a.load.load4, 0
  %6 = icmp eq i32 %5, 1
  %assert.condition = icmp eq i1 %6, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %if.end
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

assert.success:                                   ; preds = %if.end
  store %"Optional<int>" %a.load.load4, ptr %1, align 4
  %7 = getelementptr inbounds %"Optional<int>", ptr %1, i32 0, i32 1
  %.load = load i32, ptr %7, align 4
  %b.load = load ptr, ptr %b2, align 8
  %b.load.load = load i32, ptr %b.load, align 4
  %8 = icmp eq i32 %.load, %b.load.load
  ret i1 %8
}

declare void @_EN3std10assertFailEP4char(ptr)
