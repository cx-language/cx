
%"X<int>" = type { %"Y<int>"* }
%"Y<int>" = type { i32 }

@0 = private unnamed_addr constant [103 x i8] c"Assertion failed at /Users/emlai/src/delta/test/irgen/optional-type-unknown-identifier-bug.delta:11:18\00"

define i32 @main() {
  %x = alloca %"X<int>"
  call void @_ENM4main1XI3intE4initE(%"X<int>"* %x)
  call void @_ENM4main1XI3intE6deinitE(%"X<int>"* %x)
  ret i32 0
}

define void @_EN4main1YI3intE1fE(%"Y<int>"* %this) {
  ret void
}

define void @_ENM4main1XI3intE4initE(%"X<int>"* %this) {
  %y = getelementptr inbounds %"X<int>", %"X<int>"* %this, i32 0, i32 0
  store %"Y<int>"* null, %"Y<int>"** %y
  ret void
}

define void @_ENM4main1XI3intE6deinitE(%"X<int>"* %this) {
  %a = alloca i32
  %y = getelementptr inbounds %"X<int>", %"X<int>"* %this, i32 0, i32 0
  %y1 = load %"Y<int>"*, %"Y<int>"** %y
  %assert.condition = icmp eq %"Y<int>"* %y1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %1 = call i32 @puts(i8* getelementptr inbounds ([103 x i8], [103 x i8]* @0, i32 0, i32 0))
  call void @llvm.trap()
  unreachable

assert.success:                                   ; preds = %0
  %a2 = getelementptr inbounds %"Y<int>", %"Y<int>"* %y1, i32 0, i32 0
  %2 = load i32, i32* %a2
  store i32 %2, i32* %a
  ret void
}

declare i32 @puts(i8*)

; Function Attrs: noreturn nounwind
declare void @llvm.trap() #0

attributes #0 = { noreturn nounwind }
