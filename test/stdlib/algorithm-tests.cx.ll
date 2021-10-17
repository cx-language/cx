
%"ClosedRange<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }
%StringBuffer = type { %"List<char>" }
%"List<char>" = type { i8*, i32, i32 }
%"ArrayRef<int>" = type { i32*, i32 }
%"List<int>" = type { i32*, i32, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"ClosedRangeIterator<int>" = type { i32, i32 }
%StringIterator = type { i8*, i8* }
%"ArrayIterator<int>" = type { i32*, i32* }
%never = type {}
%OutputFileStream = type { %FILE* }
%FILE = type {}

@0 = private unnamed_addr constant [45 x i8] c"Assertion failed at algorithm-tests.cx:12:5\0A\00", align 1
@1 = private unnamed_addr constant [45 x i8] c"Assertion failed at algorithm-tests.cx:13:5\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"abc\00", align 1
@3 = private unnamed_addr constant [45 x i8] c"Assertion failed at algorithm-tests.cx:14:5\0A\00", align 1
@4 = private unnamed_addr constant [4 x i8] c"abc\00", align 1
@5 = private unnamed_addr constant [45 x i8] c"Assertion failed at algorithm-tests.cx:15:5\0A\00", align 1
@6 = private unnamed_addr constant [45 x i8] c"Assertion failed at algorithm-tests.cx:16:5\0A\00", align 1
@7 = private unnamed_addr constant [45 x i8] c"Assertion failed at algorithm-tests.cx:17:5\0A\00", align 1
@8 = private unnamed_addr constant [45 x i8] c"Assertion failed at algorithm-tests.cx:33:5\0A\00", align 1
@9 = private unnamed_addr constant [45 x i8] c"Assertion failed at algorithm-tests.cx:51:5\0A\00", align 1
@10 = private unnamed_addr constant [45 x i8] c"Assertion failed at algorithm-tests.cx:55:5\0A\00", align 1
@11 = private unnamed_addr constant [11 x i8] c"operator[]\00", align 1
@12 = private unnamed_addr constant [10 x i8] c"ArrayRef.\00", align 1
@13 = private unnamed_addr constant [9 x i8] c": index \00", align 1
@14 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@15 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1
@16 = private unnamed_addr constant [12 x i8] c"List index \00", align 1
@17 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1

define i32 @main() {
  call void @_EN4main14testAllAnyNoneE()
  call void @_EN4main17testInsertionSortE()
  call void @_EN4main13testQuickSortE()
  call void @_EN4main7testMaxE()
  ret i32 0
}

define void @_EN4main14testAllAnyNoneE() {
  %1 = alloca %"ClosedRange<int>", align 8
  call void @_EN3std11ClosedRangeI3intE4initE3int3int(%"ClosedRange<int>"* %1, i32 0, i32 3)
  %2 = call i1 @_EN3std11ClosedRangeI3intE3allEF3int_4bool(%"ClosedRange<int>"* %1, i1 (i32)* @_EN4main9__lambda0E3int)
  %assert.condition = icmp eq i1 %2, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @0, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %3 = alloca %"Range<int>", align 8
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %3, i32 0, i32 3)
  %4 = call i1 @_EN3std5RangeI3intE3allEF3int_4bool(%"Range<int>"* %3, i1 (i32)* @_EN4main9__lambda1E3int)
  %5 = xor i1 %4, true
  %assert.condition1 = icmp eq i1 %5, false
  br i1 %assert.condition1, label %assert.fail2, label %assert.success3

assert.fail2:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @1, i32 0, i32 0))
  unreachable

assert.success3:                                  ; preds = %assert.success
  %__str = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @2, i32 0, i32 0), i32 3)
  %6 = call i1 @_EN3std6string3anyEF4char_4bool(%string* %__str, i1 (i8)* @_EN4main9__lambda2E4char)
  %assert.condition4 = icmp eq i1 %6, false
  br i1 %assert.condition4, label %assert.fail5, label %assert.success6

assert.fail5:                                     ; preds = %assert.success3
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @3, i32 0, i32 0))
  unreachable

assert.success6:                                  ; preds = %assert.success3
  %__str7 = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str7, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @4, i32 0, i32 0), i32 3)
  %__str.load = load %string, %string* %__str7, align 8
  %7 = alloca %StringBuffer, align 8
  call void @_EN3std12StringBuffer4initE6string(%StringBuffer* %7, %string %__str.load)
  %8 = call i1 @_EN3std12StringBuffer3anyEF4char_4bool(%StringBuffer* %7, i1 (i8)* @_EN4main9__lambda3E4char)
  %9 = xor i1 %8, true
  %assert.condition8 = icmp eq i1 %9, false
  br i1 %assert.condition8, label %assert.fail9, label %assert.success10

assert.fail9:                                     ; preds = %assert.success6
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @5, i32 0, i32 0))
  unreachable

assert.success10:                                 ; preds = %assert.success6
  %10 = alloca [4 x i32], align 4
  store [4 x i32] [i32 0, i32 1, i32 2, i32 3], [4 x i32]* %10, align 4
  %11 = getelementptr inbounds [4 x i32], [4 x i32]* %10, i32 0, i32 0
  %12 = insertvalue %"ArrayRef<int>" undef, i32* %11, 0
  %13 = insertvalue %"ArrayRef<int>" %12, i32 4, 1
  %14 = alloca %"List<int>", align 8
  call void @_EN3std4ListI3intE4initE8ArrayRefI3intE(%"List<int>"* %14, %"ArrayRef<int>" %13)
  %15 = call i1 @_EN3std4ListI3intE4noneEFP3int_4bool(%"List<int>"* %14, i1 (i32*)* @_EN4main9__lambda4EP3int)
  %assert.condition11 = icmp eq i1 %15, false
  br i1 %assert.condition11, label %assert.fail12, label %assert.success13

assert.fail12:                                    ; preds = %assert.success10
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @6, i32 0, i32 0))
  unreachable

assert.success13:                                 ; preds = %assert.success10
  %16 = alloca %"ArrayRef<int>", align 8
  call void @_EN3std8ArrayRefI3intE4initE(%"ArrayRef<int>"* %16)
  %17 = call i1 @_EN3std8ArrayRefI3intE4noneEFP3int_4bool(%"ArrayRef<int>"* %16, i1 (i32*)* @_EN4main9__lambda5EP3int)
  %assert.condition14 = icmp eq i1 %17, false
  br i1 %assert.condition14, label %assert.fail15, label %assert.success16

assert.fail15:                                    ; preds = %assert.success13
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @7, i32 0, i32 0))
  unreachable

assert.success16:                                 ; preds = %assert.success13
  ret void
}

define void @_EN4main17testInsertionSortE() {
  %a = alloca %"List<int>", align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %real = alloca %"List<int>", align 8
  %__iterator1 = alloca %"RangeIterator<int>", align 8
  %2 = alloca %"Range<int>", align 8
  %i2 = alloca i32, align 4
  %3 = alloca %"ArrayRef<int>", align 8
  %4 = alloca %"ArrayRef<int>", align 8
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %a)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 5)
  %5 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %5, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %6 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %6, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %7 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %7, i32* %i, align 4
  %i.load = load i32, i32* %i, align 4
  %8 = sub i32 5, %i.load
  call void @_EN3std4ListI3intE4pushE3int(%"List<int>"* %a, i32 %8)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %real)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %2, i32 0, i32 5)
  %9 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %2)
  store %"RangeIterator<int>" %9, %"RangeIterator<int>"* %__iterator1, align 4
  br label %loop.condition3

loop.condition3:                                  ; preds = %loop.increment6, %loop.end
  %10 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator1)
  br i1 %10, label %loop.body4, label %loop.end7

loop.body4:                                       ; preds = %loop.condition3
  %11 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator1)
  store i32 %11, i32* %i2, align 4
  %i.load5 = load i32, i32* %i2, align 4
  %12 = add i32 %i.load5, 1
  call void @_EN3std4ListI3intE4pushE3int(%"List<int>"* %real, i32 %12)
  br label %loop.increment6

loop.increment6:                                  ; preds = %loop.body4
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator1)
  br label %loop.condition3

loop.end7:                                        ; preds = %loop.condition3
  call void @_EN3std4sortI3intEEP4ListI3intE(%"List<int>"* %a)
  call void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(%"ArrayRef<int>"* %3, %"List<int>"* %a)
  %.load = load %"ArrayRef<int>", %"ArrayRef<int>"* %3, align 8
  call void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(%"ArrayRef<int>"* %4, %"List<int>"* %real)
  %.load8 = load %"ArrayRef<int>", %"ArrayRef<int>"* %4, align 8
  %13 = call i1 @_EN3stdeqI3intEE8ArrayRefI3intE8ArrayRefI3intE(%"ArrayRef<int>" %.load, %"ArrayRef<int>" %.load8)
  %assert.condition = icmp eq i1 %13, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %loop.end7
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @8, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.end7
  call void @_EN3std4ListI3intE6deinitE(%"List<int>"* %real)
  call void @_EN3std4ListI3intE6deinitE(%"List<int>"* %a)
  ret void
}

define void @_EN4main13testQuickSortE() {
  %a = alloca %"List<int>", align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %real = alloca %"List<int>", align 8
  %__iterator1 = alloca %"RangeIterator<int>", align 8
  %2 = alloca %"Range<int>", align 8
  %i2 = alloca i32, align 4
  %3 = alloca %"ArrayRef<int>", align 8
  %4 = alloca %"ArrayRef<int>", align 8
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %a)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 64)
  %5 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %5, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %6 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %6, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %7 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %7, i32* %i, align 4
  %i.load = load i32, i32* %i, align 4
  %8 = sub i32 64, %i.load
  call void @_EN3std4ListI3intE4pushE3int(%"List<int>"* %a, i32 %8)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %real)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %2, i32 0, i32 64)
  %9 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %2)
  store %"RangeIterator<int>" %9, %"RangeIterator<int>"* %__iterator1, align 4
  br label %loop.condition3

loop.condition3:                                  ; preds = %loop.increment6, %loop.end
  %10 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator1)
  br i1 %10, label %loop.body4, label %loop.end7

loop.body4:                                       ; preds = %loop.condition3
  %11 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator1)
  store i32 %11, i32* %i2, align 4
  %i.load5 = load i32, i32* %i2, align 4
  %12 = add i32 %i.load5, 1
  call void @_EN3std4ListI3intE4pushE3int(%"List<int>"* %real, i32 %12)
  br label %loop.increment6

loop.increment6:                                  ; preds = %loop.body4
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator1)
  br label %loop.condition3

loop.end7:                                        ; preds = %loop.condition3
  call void @_EN3std4sortI3intEEP4ListI3intE(%"List<int>"* %a)
  call void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(%"ArrayRef<int>"* %3, %"List<int>"* %a)
  %.load = load %"ArrayRef<int>", %"ArrayRef<int>"* %3, align 8
  call void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(%"ArrayRef<int>"* %4, %"List<int>"* %real)
  %.load8 = load %"ArrayRef<int>", %"ArrayRef<int>"* %4, align 8
  %13 = call i1 @_EN3stdeqI3intEE8ArrayRefI3intE8ArrayRefI3intE(%"ArrayRef<int>" %.load, %"ArrayRef<int>" %.load8)
  %assert.condition = icmp eq i1 %13, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %loop.end7
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @9, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.end7
  call void @_EN3std4ListI3intE6deinitE(%"List<int>"* %real)
  call void @_EN3std4ListI3intE6deinitE(%"List<int>"* %a)
  ret void
}

define void @_EN4main7testMaxE() {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 7, i32* %1, align 4
  store i32 9, i32* %2, align 4
  %3 = call i32* @_EN3std3maxI3intEEP3intP3int(i32* %1, i32* %2)
  %.load = load i32, i32* %3, align 4
  %4 = icmp eq i32 %.load, 9
  %assert.condition = icmp eq i1 %4, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @10, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  ret void
}

define void @_EN3std11ClosedRangeI3intE4initE3int3int(%"ClosedRange<int>"* %this, i32 %start, i32 %end) {
  %start1 = getelementptr inbounds %"ClosedRange<int>", %"ClosedRange<int>"* %this, i32 0, i32 0
  store i32 %start, i32* %start1, align 4
  %end2 = getelementptr inbounds %"ClosedRange<int>", %"ClosedRange<int>"* %this, i32 0, i32 1
  store i32 %end, i32* %end2, align 4
  ret void
}

define i1 @_EN3std11ClosedRangeI3intE3allEF3int_4bool(%"ClosedRange<int>"* %this, i1 (i32)* %predicate) {
  %1 = alloca %"ClosedRangeIterator<int>", align 8
  %2 = call %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(%"ClosedRange<int>"* %this)
  store %"ClosedRangeIterator<int>" %2, %"ClosedRangeIterator<int>"* %1, align 4
  %3 = call i1 @_EN3std3allI3int19ClosedRangeIteratorI3intEEEP19ClosedRangeIteratorI3intEF3int_4bool(%"ClosedRangeIterator<int>"* %1, i1 (i32)* %predicate)
  ret i1 %3
}

define i1 @_EN4main9__lambda0E3int(i32 %x) {
  %1 = icmp slt i32 %x, 4
  ret i1 %1
}

declare void @_EN3std10assertFailEP4char(i8*)

declare void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"*, i32, i32)

define i1 @_EN3std5RangeI3intE3allEF3int_4bool(%"Range<int>"* %this, i1 (i32)* %predicate) {
  %1 = alloca %"RangeIterator<int>", align 8
  %2 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %this)
  store %"RangeIterator<int>" %2, %"RangeIterator<int>"* %1, align 4
  %3 = call i1 @_EN3std3allI3int13RangeIteratorI3intEEEP13RangeIteratorI3intEF3int_4bool(%"RangeIterator<int>"* %1, i1 (i32)* %predicate)
  ret i1 %3
}

define i1 @_EN4main9__lambda1E3int(i32 %x) {
  %1 = icmp ne i32 %x, 2
  ret i1 %1
}

declare void @_EN3std6string4initEP4char3int(%string*, i8*, i32)

define i1 @_EN3std6string3anyEF4char_4bool(%string* %this, i1 (i8)* %predicate) {
  %1 = alloca %StringIterator, align 8
  %2 = call %StringIterator @_EN3std6string8iteratorE(%string* %this)
  store %StringIterator %2, %StringIterator* %1, align 8
  %3 = call i1 @_EN3std3anyI4char14StringIteratorEEP14StringIteratorF4char_4bool(%StringIterator* %1, i1 (i8)* %predicate)
  ret i1 %3
}

define i1 @_EN4main9__lambda2E4char(i8 %x) {
  %1 = icmp eq i8 %x, 99
  ret i1 %1
}

declare void @_EN3std12StringBuffer4initE6string(%StringBuffer*, %string)

define i1 @_EN3std12StringBuffer3anyEF4char_4bool(%StringBuffer* %this, i1 (i8)* %predicate) {
  %1 = alloca %StringIterator, align 8
  %2 = call %StringIterator @_EN3std12StringBuffer8iteratorE(%StringBuffer* %this)
  store %StringIterator %2, %StringIterator* %1, align 8
  %3 = call i1 @_EN3std3anyI4char14StringIteratorEEP14StringIteratorF4char_4bool(%StringIterator* %1, i1 (i8)* %predicate)
  ret i1 %3
}

define i1 @_EN4main9__lambda3E4char(i8 %x) {
  %1 = icmp eq i8 %x, 100
  ret i1 %1
}

define void @_EN3std4ListI3intE4initE8ArrayRefI3intE(%"List<int>"* %this, %"ArrayRef<int>" %elements) {
  %1 = alloca %"ArrayRef<int>", align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %2 = alloca %"Range<int>", align 8
  %3 = alloca %"ArrayRef<int>", align 8
  %index = alloca i32, align 4
  %4 = alloca %"ArrayRef<int>", align 8
  store %"ArrayRef<int>" %elements, %"ArrayRef<int>"* %1, align 8
  %5 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %1)
  call void @_EN3std4ListI3intE4initE8capacity3int(%"List<int>"* %this, i32 %5)
  store %"ArrayRef<int>" %elements, %"ArrayRef<int>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %3)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %2, i32 0, i32 %6)
  %7 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %2)
  store %"RangeIterator<int>" %7, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %8 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %8, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %9 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %9, i32* %index, align 4
  store %"ArrayRef<int>" %elements, %"ArrayRef<int>"* %4, align 8
  %index.load = load i32, i32* %index, align 4
  %10 = call i32* @_EN3std8ArrayRefI3intEixE3int(%"ArrayRef<int>"* %4, i32 %index.load)
  %.load = load i32, i32* %10, align 4
  call void @_EN3std4ListI3intE4pushE3int(%"List<int>"* %this, i32 %.load)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret void
}

define i1 @_EN3std4ListI3intE4noneEFP3int_4bool(%"List<int>"* %this, i1 (i32*)* %predicate) {
  %1 = alloca %"ArrayIterator<int>", align 8
  %2 = call %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(%"List<int>"* %this)
  store %"ArrayIterator<int>" %2, %"ArrayIterator<int>"* %1, align 8
  %3 = call i1 @_EN3std4noneIP3int13ArrayIteratorI3intEEEP13ArrayIteratorI3intEFP3int_4bool(%"ArrayIterator<int>"* %1, i1 (i32*)* %predicate)
  ret i1 %3
}

define i1 @_EN4main9__lambda4EP3int(i32* %x) {
  %x.load = load i32, i32* %x, align 4
  %1 = icmp sgt i32 %x.load, 3
  ret i1 %1
}

define void @_EN3std8ArrayRefI3intE4initE(%"ArrayRef<int>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  ret void
}

define i1 @_EN3std8ArrayRefI3intE4noneEFP3int_4bool(%"ArrayRef<int>"* %this, i1 (i32*)* %predicate) {
  %1 = alloca %"ArrayIterator<int>", align 8
  %2 = call %"ArrayIterator<int>" @_EN3std8ArrayRefI3intE8iteratorE(%"ArrayRef<int>"* %this)
  store %"ArrayIterator<int>" %2, %"ArrayIterator<int>"* %1, align 8
  %3 = call i1 @_EN3std4noneIP3int13ArrayIteratorI3intEEEP13ArrayIteratorI3intEFP3int_4bool(%"ArrayIterator<int>"* %1, i1 (i32*)* %predicate)
  ret i1 %3
}

define i1 @_EN4main9__lambda5EP3int(i32* %x) {
  ret i1 true
}

define void @_EN3std4ListI3intE4initE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"*)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"*)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"*)

define void @_EN3std4ListI3intE4pushE3int(%"List<int>"* %this, i32 %element) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE4growE(%"List<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer, align 8
  %size1 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load2 = load i32, i32* %size1, align 4
  %2 = getelementptr inbounds i32, i32* %buffer.load, i32 %size.load2
  store i32 %element, i32* %2, align 4
  %size3 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load4 = load i32, i32* %size3, align 4
  %3 = add i32 %size.load4, 1
  store i32 %3, i32* %size3, align 4
  ret void
}

declare void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

define void @_EN3std4sortI3intEEP4ListI3intE(%"List<int>"* %array) {
  %1 = call i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %array)
  %2 = icmp sle i32 %1, 32
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std13insertionSortI3intEEP4ListI3intE(%"List<int>"* %array)
  br label %if.end

if.else:                                          ; preds = %0
  %3 = call i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %array)
  %4 = sub i32 %3, 1
  call void @_EN3std9quickSortI3intEEP4ListI3intE3int3int(%"List<int>"* %array, i32 0, i32 %4)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(%"ArrayRef<int>"* %this, %"List<int>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  %1 = call i32* @_EN3std4ListI3intE4dataE(%"List<int>"* %list)
  store i32* %1, i32** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define i1 @_EN3stdeqI3intEE8ArrayRefI3intE8ArrayRefI3intE(%"ArrayRef<int>" %a, %"ArrayRef<int>" %b) {
  %1 = alloca %"ArrayRef<int>", align 8
  %2 = alloca %"ArrayRef<int>", align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %3 = alloca %"Range<int>", align 8
  %4 = alloca %"ArrayRef<int>", align 8
  %i = alloca i32, align 4
  %5 = alloca %"ArrayRef<int>", align 8
  %6 = alloca %"ArrayRef<int>", align 8
  store %"ArrayRef<int>" %a, %"ArrayRef<int>"* %1, align 8
  %7 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %1)
  store %"ArrayRef<int>" %b, %"ArrayRef<int>"* %2, align 8
  %8 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %2)
  %9 = icmp ne i32 %7, %8
  br i1 %9, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  store %"ArrayRef<int>" %a, %"ArrayRef<int>"* %4, align 8
  %10 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %4)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %3, i32 0, i32 %10)
  %11 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %3)
  store %"RangeIterator<int>" %11, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %if.end
  %12 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %12, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %13 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %13, i32* %i, align 4
  store %"ArrayRef<int>" %a, %"ArrayRef<int>"* %5, align 8
  %i.load = load i32, i32* %i, align 4
  %14 = call i32* @_EN3std8ArrayRefI3intEixE3int(%"ArrayRef<int>"* %5, i32 %i.load)
  %.load = load i32, i32* %14, align 4
  store %"ArrayRef<int>" %b, %"ArrayRef<int>"* %6, align 8
  %i.load1 = load i32, i32* %i, align 4
  %15 = call i32* @_EN3std8ArrayRefI3intEixE3int(%"ArrayRef<int>"* %6, i32 %i.load1)
  %.load2 = load i32, i32* %15, align 4
  %16 = icmp ne i32 %.load, %.load2
  br i1 %16, label %if.then3, label %if.else4

loop.increment:                                   ; preds = %if.end5
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i1 true

if.then3:                                         ; preds = %loop.body
  ret i1 false

if.else4:                                         ; preds = %loop.body
  br label %if.end5

if.end5:                                          ; preds = %if.else4
  br label %loop.increment
}

define void @_EN3std4ListI3intE6deinitE(%"List<int>"* %this) {
  %__iterator = alloca %"ArrayIterator<int>", align 8
  %element = alloca i32*, align 8
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(%"List<int>"* %this)
  store %"ArrayIterator<int>" %2, %"ArrayIterator<int>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>"* %__iterator)
  store i32* %4, i32** %element, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer, align 8
  call void @_EN3std10deallocateIAU_3intEEAU_3int(i32* %buffer.load)
  br label %if.end
}

define i32* @_EN3std3maxI3intEEP3intP3int(i32* %a, i32* %b) {
  %a.load = load i32, i32* %a, align 4
  %b.load = load i32, i32* %b, align 4
  %1 = icmp sge i32 %a.load, %b.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i32* %a

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  ret i32* %b
}

define %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(%"ClosedRange<int>"* %this) {
  %1 = alloca %"ClosedRangeIterator<int>", align 8
  %this.load = load %"ClosedRange<int>", %"ClosedRange<int>"* %this, align 4
  call void @_EN3std19ClosedRangeIteratorI3intE4initE11ClosedRangeI3intE(%"ClosedRangeIterator<int>"* %1, %"ClosedRange<int>" %this.load)
  %.load = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %1, align 4
  ret %"ClosedRangeIterator<int>" %.load
}

define i1 @_EN3std3allI3int19ClosedRangeIteratorI3intEEEP19ClosedRangeIteratorI3intEF3int_4bool(%"ClosedRangeIterator<int>"* %it, i1 (i32)* %predicate) {
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %1 = call i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(%"ClosedRangeIterator<int>"* %it)
  br i1 %1, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %2 = call i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(%"ClosedRangeIterator<int>"* %it)
  %3 = call i1 %predicate(i32 %2)
  %4 = xor i1 %3, true
  br i1 %4, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std19ClosedRangeIteratorI3intE9incrementE(%"ClosedRangeIterator<int>"* %it)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i1 true

if.then:                                          ; preds = %loop.body
  ret i1 false

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.increment
}

define i1 @_EN3std3allI3int13RangeIteratorI3intEEEP13RangeIteratorI3intEF3int_4bool(%"RangeIterator<int>"* %it, i1 (i32)* %predicate) {
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %1 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %it)
  br i1 %1, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %2 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %it)
  %3 = call i1 %predicate(i32 %2)
  %4 = xor i1 %3, true
  br i1 %4, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %it)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i1 true

if.then:                                          ; preds = %loop.body
  ret i1 false

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.increment
}

declare %StringIterator @_EN3std6string8iteratorE(%string*)

define i1 @_EN3std3anyI4char14StringIteratorEEP14StringIteratorF4char_4bool(%StringIterator* %it, i1 (i8)* %predicate) {
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %1 = call i1 @_EN3std14StringIterator8hasValueE(%StringIterator* %it)
  br i1 %1, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %2 = call i8 @_EN3std14StringIterator5valueE(%StringIterator* %it)
  %3 = call i1 %predicate(i8 %2)
  br i1 %3, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std14StringIterator9incrementE(%StringIterator* %it)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i1 false

if.then:                                          ; preds = %loop.body
  ret i1 true

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.increment
}

define %StringIterator @_EN3std12StringBuffer8iteratorE(%StringBuffer* %this) {
  %1 = alloca %StringIterator, align 8
  %2 = alloca %string, align 8
  call void @_EN3std6string4initEP12StringBuffer(%string* %2, %StringBuffer* %this)
  %.load = load %string, %string* %2, align 8
  call void @_EN3std14StringIterator4initE6string(%StringIterator* %1, %string %.load)
  %.load1 = load %StringIterator, %StringIterator* %1, align 8
  ret %StringIterator %.load1
}

define %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(%"List<int>"* %this) {
  %1 = alloca %"ArrayIterator<int>", align 8
  %2 = alloca %"ArrayRef<int>", align 8
  call void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(%"ArrayRef<int>"* %2, %"List<int>"* %this)
  %.load = load %"ArrayRef<int>", %"ArrayRef<int>"* %2, align 8
  call void @_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE(%"ArrayIterator<int>"* %1, %"ArrayRef<int>" %.load)
  %.load1 = load %"ArrayIterator<int>", %"ArrayIterator<int>"* %1, align 8
  ret %"ArrayIterator<int>" %.load1
}

define i1 @_EN3std4noneIP3int13ArrayIteratorI3intEEEP13ArrayIteratorI3intEFP3int_4bool(%"ArrayIterator<int>"* %it, i1 (i32*)* %predicate) {
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %1 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>"* %it)
  br i1 %1, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %2 = call i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>"* %it)
  %3 = call i1 %predicate(i32* %2)
  br i1 %3, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %it)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i1 true

if.then:                                          ; preds = %loop.body
  ret i1 false

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.increment
}

define i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std4ListI3intE4initE8capacity3int(%"List<int>"* %this, i32 %capacity) {
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %this)
  call void @_EN3std4ListI3intE7reserveE3int(%"List<int>"* %this, i32 %capacity)
  ret void
}

define i32* @_EN3std8ArrayRefI3intEixE3int(%"ArrayRef<int>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = icmp slt i32 %index, 0
  br i1 %1, label %or.end, label %or.rhs

or.rhs:                                           ; preds = %0
  %2 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %this)
  %3 = icmp sge i32 %index, %2
  br label %or.end

or.end:                                           ; preds = %or.rhs, %0
  %or = phi i1 [ %1, %0 ], [ %3, %or.rhs ]
  br i1 %or, label %if.then, label %if.else

if.then:                                          ; preds = %or.end
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @11, i32 0, i32 0), i32 10)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std8ArrayRefI3intE16indexOutOfBoundsE6string3int(%"ArrayRef<int>"* %this, %string %__str.load, i32 %index)
  br label %if.end

if.else:                                          ; preds = %or.end
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %data = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  %data.load = load i32*, i32** %data, align 8
  %4 = getelementptr inbounds i32, i32* %data.load, i32 %index
  ret i32* %4
}

define %"ArrayIterator<int>" @_EN3std8ArrayRefI3intE8iteratorE(%"ArrayRef<int>"* %this) {
  %1 = alloca %"ArrayIterator<int>", align 8
  %this.load = load %"ArrayRef<int>", %"ArrayRef<int>"* %this, align 8
  call void @_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE(%"ArrayIterator<int>"* %1, %"ArrayRef<int>" %this.load)
  %.load = load %"ArrayIterator<int>", %"ArrayIterator<int>"* %1, align 8
  ret %"ArrayIterator<int>" %.load
}

define i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32*, i32** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 1
  %end.load = load i32*, i32** %end, align 8
  %1 = icmp ne i32* %current.load, %end.load
  ret i1 %1
}

define i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32*, i32** %current, align 8
  ret i32* %current.load
}

define void @_EN3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32*, i32** %current, align 8
  %1 = getelementptr inbounds i32, i32* %current.load, i32 1
  store i32* %1, i32** %current, align 8
  ret void
}

define void @_EN3std10deallocateIAU_3intEEAU_3int(i32* %allocation) {
  %1 = bitcast i32* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define void @_EN3std4ListI3intE4growE(%"List<int>"* %this) {
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE7reserveE3int(%"List<int>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load2 = load i32, i32* %capacity1, align 4
  %2 = mul i32 %capacity.load2, 2
  call void @_EN3std4ListI3intE7reserveE3int(%"List<int>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std13insertionSortI3intEEP4ListI3intE(%"List<int>"* %arr) {
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 1, i32* %i, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.end7, %0
  %i.load = load i32, i32* %i, align 4
  %1 = call i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %arr)
  %2 = icmp slt i32 %i.load, %1
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %i.load1 = load i32, i32* %i, align 4
  store i32 %i.load1, i32* %j, align 4
  br label %loop.condition2

loop.end:                                         ; preds = %loop.condition
  ret void

loop.condition2:                                  ; preds = %loop.body3, %loop.body
  %j.load = load i32, i32* %j, align 4
  %3 = icmp sgt i32 %j.load, 0
  br i1 %3, label %and.rhs, label %and.end

loop.body3:                                       ; preds = %and.end
  %j.load4 = load i32, i32* %j, align 4
  %4 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %j.load4)
  %j.load5 = load i32, i32* %j, align 4
  %5 = sub i32 %j.load5, 1
  %6 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %5)
  call void @_EN3std4swapI3intEEP3intP3int(i32* %4, i32* %6)
  %j.load6 = load i32, i32* %j, align 4
  %7 = add i32 %j.load6, -1
  store i32 %7, i32* %j, align 4
  br label %loop.condition2

loop.end7:                                        ; preds = %and.end
  %i.load8 = load i32, i32* %i, align 4
  %8 = add i32 %i.load8, 1
  store i32 %8, i32* %i, align 4
  br label %loop.condition

and.rhs:                                          ; preds = %loop.condition2
  %j.load9 = load i32, i32* %j, align 4
  %9 = sub i32 %j.load9, 1
  %10 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %9)
  %.load = load i32, i32* %10, align 4
  %j.load10 = load i32, i32* %j, align 4
  %11 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %j.load10)
  %.load11 = load i32, i32* %11, align 4
  %12 = icmp sgt i32 %.load, %.load11
  br label %and.end

and.end:                                          ; preds = %and.rhs, %loop.condition2
  %and = phi i1 [ %3, %loop.condition2 ], [ %12, %and.rhs ]
  br i1 %and, label %loop.body3, label %loop.end7
}

define void @_EN3std9quickSortI3intEEP4ListI3intE3int3int(%"List<int>"* %arr, i32 %low, i32 %high) {
  %index = alloca i32, align 4
  %1 = icmp slt i32 %low, %high
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call i32 @_EN3std9partitionI3intEEP4ListI3intE3int3int(%"List<int>"* %arr, i32 %low, i32 %high)
  store i32 %2, i32* %index, align 4
  %index.load = load i32, i32* %index, align 4
  %3 = icmp slt i32 %low, %index.load
  br i1 %3, label %if.then1, label %if.else3

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.end9, %if.else
  ret void

if.then1:                                         ; preds = %if.then
  %index.load2 = load i32, i32* %index, align 4
  %4 = sub i32 %index.load2, 1
  call void @_EN3std9quickSortI3intEEP4ListI3intE3int3int(%"List<int>"* %arr, i32 %low, i32 %4)
  br label %if.end4

if.else3:                                         ; preds = %if.then
  br label %if.end4

if.end4:                                          ; preds = %if.else3, %if.then1
  %index.load5 = load i32, i32* %index, align 4
  %5 = icmp slt i32 %index.load5, %high
  br i1 %5, label %if.then6, label %if.else8

if.then6:                                         ; preds = %if.end4
  %index.load7 = load i32, i32* %index, align 4
  %6 = add i32 %index.load7, 1
  call void @_EN3std9quickSortI3intEEP4ListI3intE3int3int(%"List<int>"* %arr, i32 %6, i32 %high)
  br label %if.end9

if.else8:                                         ; preds = %if.end4
  br label %if.end9

if.end9:                                          ; preds = %if.else8, %if.then6
  br label %if.end
}

define i32* @_EN3std4ListI3intE4dataE(%"List<int>"* %this) {
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer, align 8
  ret i32* %buffer.load
}

define i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(%"ClosedRangeIterator<int>"* %this) {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32, i32* %current, align 4
  %end = getelementptr inbounds %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %this, i32 0, i32 1
  %end.load = load i32, i32* %end, align 4
  %1 = icmp sle i32 %current.load, %end.load
  ret i1 %1
}

define i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(%"ClosedRangeIterator<int>"* %this) {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32, i32* %current, align 4
  ret i32 %current.load
}

define void @_EN3std19ClosedRangeIteratorI3intE9incrementE(%"ClosedRangeIterator<int>"* %this) {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32, i32* %current, align 4
  %1 = add i32 %current.load, 1
  store i32 %1, i32* %current, align 4
  ret void
}

define void @_EN3std19ClosedRangeIteratorI3intE4initE11ClosedRangeI3intE(%"ClosedRangeIterator<int>"* %this, %"ClosedRange<int>" %range) {
  %1 = alloca %"ClosedRange<int>", align 8
  %2 = alloca %"ClosedRange<int>", align 8
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %this, i32 0, i32 0
  store %"ClosedRange<int>" %range, %"ClosedRange<int>"* %1, align 4
  %3 = call i32 @_EN3std11ClosedRangeI3intE5startE(%"ClosedRange<int>"* %1)
  store i32 %3, i32* %current, align 4
  %end = getelementptr inbounds %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %this, i32 0, i32 1
  store %"ClosedRange<int>" %range, %"ClosedRange<int>"* %2, align 4
  %4 = call i32 @_EN3std11ClosedRangeI3intE3endE(%"ClosedRange<int>"* %2)
  store i32 %4, i32* %end, align 4
  ret void
}

declare i1 @_EN3std14StringIterator8hasValueE(%StringIterator*)

declare i8 @_EN3std14StringIterator5valueE(%StringIterator*)

declare void @_EN3std14StringIterator9incrementE(%StringIterator*)

declare void @_EN3std6string4initEP12StringBuffer(%string*, %StringBuffer*)

declare void @_EN3std14StringIterator4initE6string(%StringIterator*, %string)

define void @_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE(%"ArrayIterator<int>"* %this, %"ArrayRef<int>" %array) {
  %1 = alloca %"ArrayRef<int>", align 8
  %2 = alloca %"ArrayRef<int>", align 8
  %3 = alloca %"ArrayRef<int>", align 8
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  store %"ArrayRef<int>" %array, %"ArrayRef<int>"* %1, align 8
  %4 = call i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>"* %1)
  store i32* %4, i32** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 1
  store %"ArrayRef<int>" %array, %"ArrayRef<int>"* %2, align 8
  %5 = call i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>"* %2)
  store %"ArrayRef<int>" %array, %"ArrayRef<int>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %3)
  %7 = getelementptr inbounds i32, i32* %5, i32 %6
  store i32* %7, i32** %end, align 8
  ret void
}

define void @_EN3std4ListI3intE7reserveE3int(%"List<int>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca i32*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca i32*, align 8
  %target = alloca i32*, align 8
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call i32* @_EN3std13allocateArrayI3intEE3int(i32 %minimumCapacity)
  store i32* %3, i32** %newBuffer, align 8
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 %size.load)
  %4 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %4, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.end8, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %5 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %5, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %6 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %6, i32* %index, align 4
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds i32, i32* %buffer.load, i32 %index.load
  store i32* %7, i32** %source, align 8
  %newBuffer.load = load i32*, i32** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds i32, i32* %newBuffer.load, i32 %index.load1
  store i32* %8, i32** %target, align 8
  %target.load = load i32*, i32** %target, align 8
  %source.load = load i32*, i32** %source, align 8
  %source.load.load = load i32, i32* %source.load, align 4
  store i32 %source.load.load, i32* %target.load, align 4
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load6 = load i32*, i32** %buffer5, align 8
  call void @_EN3std10deallocateIAU_3intEEAU_3int(i32* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %newBuffer.load10 = load i32*, i32** %newBuffer, align 8
  store i32* %newBuffer.load10, i32** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity11, align 4
  br label %if.end
}

define void @_EN3std8ArrayRefI3intE16indexOutOfBoundsE6string3int(%"ArrayRef<int>"* %this, %string %function, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  %__str2 = alloca %string, align 8
  %3 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @12, i32 0, i32 0), i32 9)
  store %string %function, %string* %1, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @13, i32 0, i32 0), i32 8)
  store i32 %index, i32* %2, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @14, i32 0, i32 0), i32 27)
  %4 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %this)
  store i32 %4, i32* %3, align 4
  %5 = call %never @_EN3std5abortI6string6string6string3int6string3intEEP6stringP6stringP6stringP3intP6stringP3int(%string* %__str, %string* %1, %string* %__str1, i32* %2, %string* %__str2, i32* %3)
  ret void
}

declare void @free(i8*)

define i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE16indexOutOfBoundsE3int(%"List<int>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer, align 8
  %2 = getelementptr inbounds i32, i32* %buffer.load, i32 %index
  ret i32* %2
}

define void @_EN3std4swapI3intEEP3intP3int(i32* %a, i32* %b) {
  %t = alloca i32, align 4
  %a.load = load i32, i32* %a, align 4
  store i32 %a.load, i32* %t, align 4
  %b.load = load i32, i32* %b, align 4
  store i32 %b.load, i32* %a, align 4
  %t.load = load i32, i32* %t, align 4
  store i32 %t.load, i32* %b, align 4
  ret void
}

define i32 @_EN3std9partitionI3intEEP4ListI3intE3int3int(%"List<int>"* %arr, i32 %low, i32 %high) {
  %pivot = alloca i32*, align 8
  %i = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %j = alloca i32, align 4
  %2 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %high)
  store i32* %2, i32** %pivot, align 8
  %3 = sub i32 %low, 1
  store i32 %3, i32* %i, align 4
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 %low, i32 %high)
  %4 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %4, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %5 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %5, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %6 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %6, i32* %j, align 4
  %j.load = load i32, i32* %j, align 4
  %7 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %j.load)
  %.load = load i32, i32* %7, align 4
  %pivot.load = load i32*, i32** %pivot, align 8
  %pivot.load.load = load i32, i32* %pivot.load, align 4
  %8 = icmp slt i32 %.load, %pivot.load.load
  br i1 %8, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %9 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %high)
  %.load1 = load i32, i32* %9, align 4
  %i.load = load i32, i32* %i, align 4
  %10 = add i32 %i.load, 1
  %11 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %10)
  %.load2 = load i32, i32* %11, align 4
  %12 = icmp slt i32 %.load1, %.load2
  br i1 %12, label %if.then6, label %if.else8

if.then:                                          ; preds = %loop.body
  %i.load3 = load i32, i32* %i, align 4
  %13 = add i32 %i.load3, 1
  store i32 %13, i32* %i, align 4
  %i.load4 = load i32, i32* %i, align 4
  %14 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %i.load4)
  %j.load5 = load i32, i32* %j, align 4
  %15 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %j.load5)
  call void @_EN3std4swapI3intEEP3intP3int(i32* %14, i32* %15)
  br label %if.end

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %loop.increment

if.then6:                                         ; preds = %loop.end
  %i.load7 = load i32, i32* %i, align 4
  %16 = add i32 %i.load7, 1
  %17 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %16)
  %18 = call i32* @_EN3std4ListI3intEixE3int(%"List<int>"* %arr, i32 %high)
  call void @_EN3std4swapI3intEEP3intP3int(i32* %17, i32* %18)
  br label %if.end9

if.else8:                                         ; preds = %loop.end
  br label %if.end9

if.end9:                                          ; preds = %if.else8, %if.then6
  %i.load10 = load i32, i32* %i, align 4
  %19 = add i32 %i.load10, 1
  ret i32 %19
}

define i32 @_EN3std11ClosedRangeI3intE5startE(%"ClosedRange<int>"* %this) {
  %start = getelementptr inbounds %"ClosedRange<int>", %"ClosedRange<int>"* %this, i32 0, i32 0
  %start.load = load i32, i32* %start, align 4
  ret i32 %start.load
}

define i32 @_EN3std11ClosedRangeI3intE3endE(%"ClosedRange<int>"* %this) {
  %end = getelementptr inbounds %"ClosedRange<int>", %"ClosedRange<int>"* %this, i32 0, i32 1
  %end.load = load i32, i32* %end, align 4
  ret i32 %end.load
}

define i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  %data.load = load i32*, i32** %data, align 8
  ret i32* %data.load
}

define i32* @_EN3std13allocateArrayI3intEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @15, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to i32*
  ret i32* %4
}

declare %never @_EN3std5abortI6string6string6string3int6string3intEEP6stringP6stringP6stringP3intP6stringP3int(%string*, %string*, %string*, i32*, %string*, i32*)

define void @_EN3std4ListI3intE16indexOutOfBoundsE3int(%"List<int>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @16, i32 0, i32 0), i32 11)
  store i32 %index, i32* %1, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @17, i32 0, i32 0), i32 27)
  %3 = call i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %this)
  store i32 %3, i32* %2, align 4
  %4 = call %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %__str, i32* %1, %string* %__str1, i32* %2)
  ret void
}

declare i8* @malloc(i64)

define %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %_0, i32* %_1, %string* %_2, i32* %_3) {
  %1 = alloca %OutputFileStream, align 8
  %2 = alloca %OutputFileStream, align 8
  %3 = alloca %OutputFileStream, align 8
  %4 = alloca %OutputFileStream, align 8
  %5 = alloca %OutputFileStream, align 8
  %6 = alloca i8, align 1
  %7 = call %OutputFileStream @_EN3std6stderrE()
  store %OutputFileStream %7, %OutputFileStream* %1, align 8
  call void @_EN3std16OutputFileStream5writeI6stringEEP6string(%OutputFileStream* %1, %string* %_0)
  %8 = call %OutputFileStream @_EN3std6stderrE()
  store %OutputFileStream %8, %OutputFileStream* %2, align 8
  call void @_EN3std16OutputFileStream5writeI3intEEP3int(%OutputFileStream* %2, i32* %_1)
  %9 = call %OutputFileStream @_EN3std6stderrE()
  store %OutputFileStream %9, %OutputFileStream* %3, align 8
  call void @_EN3std16OutputFileStream5writeI6stringEEP6string(%OutputFileStream* %3, %string* %_2)
  %10 = call %OutputFileStream @_EN3std6stderrE()
  store %OutputFileStream %10, %OutputFileStream* %4, align 8
  call void @_EN3std16OutputFileStream5writeI3intEEP3int(%OutputFileStream* %4, i32* %_3)
  %11 = call %OutputFileStream @_EN3std6stderrE()
  store %OutputFileStream %11, %OutputFileStream* %5, align 8
  store i8 10, i8* %6, align 1
  call void @_EN3std16OutputFileStream5writeI4charEEP4char(%OutputFileStream* %5, i8* %6)
  %12 = call %never @_EN3std12abortWrapperE()
  unreachable
}

declare %OutputFileStream @_EN3std6stderrE()

declare void @_EN3std16OutputFileStream5writeI6stringEEP6string(%OutputFileStream*, %string*)

declare void @_EN3std16OutputFileStream5writeI3intEEP3int(%OutputFileStream*, i32*)

declare void @_EN3std16OutputFileStream5writeI4charEEP4char(%OutputFileStream*, i8*)

declare %never @_EN3std12abortWrapperE()
