
@x = private global i32 0

define i32 @main() {
  %foo = alloca i32
  store i32 2, i32* %foo
  %foo1 = load i32, i32* %foo
  %1 = icmp eq i32 %foo1, 1
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %foo2 = load i32, i32* %foo
  %2 = icmp ne i32 %foo2, 0
  br i1 %2, label %if.then3, label %if.else4

if.then3:                                         ; preds = %if.end
  br label %if.end5

if.else4:                                         ; preds = %if.end
  br label %if.end5

if.end5:                                          ; preds = %if.else4, %if.then3
  %foo6 = load i32, i32* %foo
  switch i32 %foo6, label %switch.default [
    i32 0, label %switch.case.0
    i32 1, label %switch.case.1
    i32 2, label %switch.case.2
  ]

switch.case.0:                                    ; preds = %if.end5
  ret i32 0

switch.case.1:                                    ; preds = %if.end5
  ret i32 0

switch.case.2:                                    ; preds = %if.end5
  ret i32 0

switch.default:                                   ; preds = %if.end5
  br label %switch.end

switch.end:                                       ; preds = %switch.default
  ret i32 0
}
