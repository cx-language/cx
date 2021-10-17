
%"OrderedMap<int, int>" = type { %"AVLnode<int, int>"*, i32 }
%"AVLnode<int, int>" = type { %"MapEntry<int, int>", i32, %"AVLnode<int, int>"*, %"AVLnode<int, int>"*, %"AVLnode<int, int>"* }
%"MapEntry<int, int>" = type { i32, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }
%"OrderedMapIterator<int, int>" = type { %"AVLnode<int, int>"*, %"OrderedMap<int, int>"* }

@0 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:18:5\0A\00", align 1
@1 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:21:5\0A\00", align 1
@2 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:25:9\0A\00", align 1
@3 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:28:5\0A\00", align 1
@4 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:29:5\0A\00", align 1
@5 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:36:5\0A\00", align 1
@6 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:39:5\0A\00", align 1
@7 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:46:9\0A\00", align 1
@8 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:48:9\0A\00", align 1
@9 = private unnamed_addr constant [45 x i8] c"Unwrap failed at ordered-map-tests.cx:56:13\0A\00", align 1
@10 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:56:5\0A\00", align 1
@11 = private unnamed_addr constant [45 x i8] c"Unwrap failed at ordered-map-tests.cx:65:17\0A\00", align 1
@12 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:65:9\0A\00", align 1
@13 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:68:9\0A\00", align 1
@14 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:79:5\0A\00", align 1
@15 = private unnamed_addr constant [45 x i8] c"Unwrap failed at ordered-map-tests.cx:81:29\0A\00", align 1
@16 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:81:9\0A\00", align 1
@17 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:84:5\0A\00", align 1
@18 = private unnamed_addr constant [45 x i8] c"Unwrap failed at ordered-map-tests.cx:86:30\0A\00", align 1
@19 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:86:9\0A\00", align 1
@20 = private unnamed_addr constant [45 x i8] c"Unwrap failed at ordered-map-tests.cx:98:21\0A\00", align 1
@21 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:98:5\0A\00", align 1
@22 = private unnamed_addr constant [45 x i8] c"Unwrap failed at ordered-map-tests.cx:99:20\0A\00", align 1
@23 = private unnamed_addr constant [47 x i8] c"Assertion failed at ordered-map-tests.cx:99:5\0A\00", align 1
@24 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:105:5\0A\00", align 1
@25 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:106:5\0A\00", align 1
@26 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:107:5\0A\00", align 1
@27 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:108:5\0A\00", align 1
@28 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:154:9\0A\00", align 1
@29 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:121:9\0A\00", align 1
@30 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:122:9\0A\00", align 1
@31 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:126:5\0A\00", align 1
@32 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:137:9\0A\00", align 1
@33 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:138:9\0A\00", align 1
@34 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:142:5\0A\00", align 1
@35 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:167:9\0A\00", align 1
@36 = private unnamed_addr constant [48 x i8] c"Assertion failed at ordered-map-tests.cx:170:5\0A\00", align 1
@37 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:162:31\0A\00", align 1
@38 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:208:27\0A\00", align 1
@39 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:252:31\0A\00", align 1
@40 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:301:33\0A\00", align 1
@41 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:331:29\0A\00", align 1
@42 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:347:29\0A\00", align 1
@43 = private unnamed_addr constant [46 x i8] c"Unwrap failed at OrderedMapIterator.cx:15:23\0A\00", align 1
@44 = private unnamed_addr constant [46 x i8] c"Unwrap failed at OrderedMapIterator.cx:19:41\0A\00", align 1
@45 = private unnamed_addr constant [35 x i8] c"Unwrap failed at allocate.cx:9:54\0A\00", align 1
@46 = private unnamed_addr constant [38 x i8] c"Unwrap failed at OrderedMap.cx:32:30\0A\00", align 1
@47 = private unnamed_addr constant [38 x i8] c"Unwrap failed at OrderedMap.cx:38:31\0A\00", align 1
@48 = private unnamed_addr constant [38 x i8] c"Unwrap failed at OrderedMap.cx:46:25\0A\00", align 1
@49 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:280:38\0A\00", align 1
@50 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:288:19\0A\00", align 1
@51 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:313:38\0A\00", align 1
@52 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:321:19\0A\00", align 1
@53 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:271:19\0A\00", align 1
@54 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:263:19\0A\00", align 1
@55 = private unnamed_addr constant [38 x i8] c"Unwrap failed at OrderedMap.cx:80:23\0A\00", align 1
@56 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:105:35\0A\00", align 1
@57 = private unnamed_addr constant [38 x i8] c"Unwrap failed at OrderedMap.cx:54:24\0A\00", align 1
@58 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:110:38\0A\00", align 1
@59 = private unnamed_addr constant [39 x i8] c"Unwrap failed at OrderedMap.cx:339:29\0A\00", align 1

define i32 @main() {
  call void @_EN4main21testInsertAndContainsE()
  call void @_EN4main19testInsertAndDeleteE()
  call void @_EN4main18testInsertAndIndexE()
  call void @_EN4main18testLowerAndHigherE()
  call void @_EN4main13testFirstLastE()
  call void @_EN4main21testEmptyMapFunctionsE()
  call void @_EN4main20testEmptyMapIteratorE()
  call void @_EN4main17testIteratorOrderE()
  call void @_EN4main19testUnitMapIteratorE()
  ret i32 0
}

define void @_EN4main21testInsertAndContainsE() {
  %m = alloca %"OrderedMap<int, int>", align 8
  %1 = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int>", align 8
  %2 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %3 = alloca i32, align 4
  call void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %m)
  %4 = call i32 @_EN3std10OrderedMapI3int3intE4sizeE(%"OrderedMap<int, int>"* %m)
  %5 = icmp eq i32 %4, 0
  %assert.condition = icmp eq i1 %5, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @0, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %6 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 7, i32 8)
  store i32 7, i32* %1, align 4
  %7 = call i1 @_EN3std10OrderedMapI3int3intE8containsEP3int(%"OrderedMap<int, int>"* %m, i32* %1)
  %assert.condition1 = icmp eq i1 %7, false
  br i1 %assert.condition1, label %assert.fail2, label %assert.success3

assert.fail2:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @1, i32 0, i32 0))
  unreachable

assert.success3:                                  ; preds = %assert.success
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %2, i32 0, i32 10)
  %8 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %2)
  store %"RangeIterator<int>" %8, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %assert.success3
  %9 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %9, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %10 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %10, i32* %i, align 4
  %i.load = load i32, i32* %i, align 4
  %i.load4 = load i32, i32* %i, align 4
  %11 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 %i.load, i32 %i.load4)
  %12 = call i1 @_EN3std10OrderedMapI3int3intE8containsEP3int(%"OrderedMap<int, int>"* %m, i32* %i)
  %assert.condition5 = icmp eq i1 %12, false
  br i1 %assert.condition5, label %assert.fail7, label %assert.success8

loop.increment:                                   ; preds = %assert.success8
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  store i32 42, i32* %3, align 4
  %13 = call i1 @_EN3std10OrderedMapI3int3intE8containsEP3int(%"OrderedMap<int, int>"* %m, i32* %3)
  %14 = xor i1 %13, true
  %assert.condition6 = icmp eq i1 %14, false
  br i1 %assert.condition6, label %assert.fail9, label %assert.success10

assert.fail7:                                     ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @2, i32 0, i32 0))
  unreachable

assert.success8:                                  ; preds = %loop.body
  br label %loop.increment

assert.fail9:                                     ; preds = %loop.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @3, i32 0, i32 0))
  unreachable

assert.success10:                                 ; preds = %loop.end
  %15 = call i32 @_EN3std10OrderedMapI3int3intE4sizeE(%"OrderedMap<int, int>"* %m)
  %16 = icmp eq i32 %15, 10
  %assert.condition11 = icmp eq i1 %16, false
  br i1 %assert.condition11, label %assert.fail12, label %assert.success13

assert.fail12:                                    ; preds = %assert.success10
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @4, i32 0, i32 0))
  unreachable

assert.success13:                                 ; preds = %assert.success10
  call void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %m)
  ret void
}

define void @_EN4main19testInsertAndDeleteE() {
  %m = alloca %"OrderedMap<int, int>", align 8
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int>", align 8
  %4 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %__iterator1 = alloca %"RangeIterator<int>", align 8
  %5 = alloca %"Range<int>", align 8
  %i2 = alloca i32, align 4
  call void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %m)
  %6 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 42, i32 39)
  store i32 42, i32* %1, align 4
  %7 = call i1 @_EN3std10OrderedMapI3int3intE8containsEP3int(%"OrderedMap<int, int>"* %m, i32* %1)
  %assert.condition = icmp eq i1 %7, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @5, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  store i32 42, i32* %2, align 4
  call void @_EN3std10OrderedMapI3int3intE6removeEP3int(%"OrderedMap<int, int>"* %m, i32* %2)
  store i32 42, i32* %3, align 4
  %8 = call i1 @_EN3std10OrderedMapI3int3intE8containsEP3int(%"OrderedMap<int, int>"* %m, i32* %3)
  %9 = xor i1 %8, true
  %assert.condition3 = icmp eq i1 %9, false
  br i1 %assert.condition3, label %assert.fail4, label %assert.success5

assert.fail4:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @6, i32 0, i32 0))
  unreachable

assert.success5:                                  ; preds = %assert.success
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %4, i32 42, i32 79)
  %10 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %4)
  store %"RangeIterator<int>" %10, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %assert.success5
  %11 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %11, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %12 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %12, i32* %i, align 4
  %i.load = load i32, i32* %i, align 4
  %13 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 %i.load, i32 42)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %5, i32 42, i32 79)
  %14 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %5)
  store %"RangeIterator<int>" %14, %"RangeIterator<int>"* %__iterator1, align 4
  br label %loop.condition6

loop.condition6:                                  ; preds = %loop.increment9, %loop.end
  %15 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator1)
  br i1 %15, label %loop.body7, label %loop.end10

loop.body7:                                       ; preds = %loop.condition6
  %16 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator1)
  store i32 %16, i32* %i2, align 4
  %17 = call i1 @_EN3std10OrderedMapI3int3intE8containsEP3int(%"OrderedMap<int, int>"* %m, i32* %i2)
  %assert.condition8 = icmp eq i1 %17, false
  br i1 %assert.condition8, label %assert.fail11, label %assert.success12

loop.increment9:                                  ; preds = %assert.success15
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator1)
  br label %loop.condition6

loop.end10:                                       ; preds = %loop.condition6
  call void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %m)
  ret void

assert.fail11:                                    ; preds = %loop.body7
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @7, i32 0, i32 0))
  unreachable

assert.success12:                                 ; preds = %loop.body7
  call void @_EN3std10OrderedMapI3int3intE6removeEP3int(%"OrderedMap<int, int>"* %m, i32* %i2)
  %18 = call i1 @_EN3std10OrderedMapI3int3intE8containsEP3int(%"OrderedMap<int, int>"* %m, i32* %i2)
  %19 = xor i1 %18, true
  %assert.condition13 = icmp eq i1 %19, false
  br i1 %assert.condition13, label %assert.fail14, label %assert.success15

assert.fail14:                                    ; preds = %assert.success12
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @8, i32 0, i32 0))
  unreachable

assert.success15:                                 ; preds = %assert.success12
  br label %loop.increment9
}

define void @_EN4main18testInsertAndIndexE() {
  %m = alloca %"OrderedMap<int, int>", align 8
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int>", align 8
  %4 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %__iterator1 = alloca %"RangeIterator<int>", align 8
  %5 = alloca %"Range<int>", align 8
  %i2 = alloca i32, align 4
  %6 = alloca i32, align 4
  call void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %m)
  %7 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 42, i32 39)
  store i32 42, i32* %1, align 4
  %8 = call i32* @_EN3std10OrderedMapI3int3intEixEP3int(%"OrderedMap<int, int>"* %m, i32* %1)
  %__implicit_unwrap.condition = icmp eq i32* %8, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @9, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %0
  store i32 39, i32* %2, align 4
  %9 = call i1 @_EN3stdeqI3intEEP3intP3int(i32* %8, i32* %2)
  %assert.condition = icmp eq i1 %9, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %__implicit_unwrap.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @10, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %__implicit_unwrap.success
  store i32 42, i32* %3, align 4
  call void @_EN3std10OrderedMapI3int3intE6removeEP3int(%"OrderedMap<int, int>"* %m, i32* %3)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %4, i32 42, i32 79)
  %10 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %4)
  store %"RangeIterator<int>" %10, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %assert.success
  %11 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %11, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %12 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %12, i32* %i, align 4
  %i.load = load i32, i32* %i, align 4
  %i.load3 = load i32, i32* %i, align 4
  %13 = add i32 %i.load3, 7
  %14 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 %i.load, i32 %13)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %5, i32 42, i32 79)
  %15 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %5)
  store %"RangeIterator<int>" %15, %"RangeIterator<int>"* %__iterator1, align 4
  br label %loop.condition4

loop.condition4:                                  ; preds = %loop.increment7, %loop.end
  %16 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator1)
  br i1 %16, label %loop.body5, label %loop.end8

loop.body5:                                       ; preds = %loop.condition4
  %17 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator1)
  store i32 %17, i32* %i2, align 4
  %18 = call i32* @_EN3std10OrderedMapI3int3intEixEP3int(%"OrderedMap<int, int>"* %m, i32* %i2)
  %__implicit_unwrap.condition6 = icmp eq i32* %18, null
  br i1 %__implicit_unwrap.condition6, label %__implicit_unwrap.fail9, label %__implicit_unwrap.success10

loop.increment7:                                  ; preds = %assert.success17
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator1)
  br label %loop.condition4

loop.end8:                                        ; preds = %loop.condition4
  call void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %m)
  ret void

__implicit_unwrap.fail9:                          ; preds = %loop.body5
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @11, i32 0, i32 0))
  unreachable

__implicit_unwrap.success10:                      ; preds = %loop.body5
  %i.load11 = load i32, i32* %i2, align 4
  %19 = add i32 %i.load11, 7
  store i32 %19, i32* %6, align 4
  %20 = call i1 @_EN3stdeqI3intEEP3intP3int(i32* %18, i32* %6)
  %assert.condition12 = icmp eq i1 %20, false
  br i1 %assert.condition12, label %assert.fail13, label %assert.success14

assert.fail13:                                    ; preds = %__implicit_unwrap.success10
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @12, i32 0, i32 0))
  unreachable

assert.success14:                                 ; preds = %__implicit_unwrap.success10
  call void @_EN3std10OrderedMapI3int3intE6removeEP3int(%"OrderedMap<int, int>"* %m, i32* %i2)
  %21 = call i32* @_EN3std10OrderedMapI3int3intEixEP3int(%"OrderedMap<int, int>"* %m, i32* %i2)
  %22 = icmp eq i32* %21, null
  %assert.condition15 = icmp eq i1 %22, false
  br i1 %assert.condition15, label %assert.fail16, label %assert.success17

assert.fail16:                                    ; preds = %assert.success14
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @13, i32 0, i32 0))
  unreachable

assert.success17:                                 ; preds = %assert.success14
  br label %loop.increment7
}

define void @_EN4main18testLowerAndHigherE() {
  %m = alloca %"OrderedMap<int, int>", align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %2 = alloca i32, align 4
  %__iterator1 = alloca %"RangeIterator<int>", align 8
  %3 = alloca %"Range<int>", align 8
  %i2 = alloca i32, align 4
  %4 = alloca i32, align 4
  %__iterator3 = alloca %"RangeIterator<int>", align 8
  %5 = alloca %"Range<int>", align 8
  %i4 = alloca i32, align 4
  call void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %m)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 100)
  %6 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %6, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %7 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %7, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %8 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %8, i32* %i, align 4
  %i.load = load i32, i32* %i, align 4
  %i.load5 = load i32, i32* %i, align 4
  %9 = add i32 %i.load5, 6
  %10 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 %i.load, i32 %9)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  store i32 0, i32* %2, align 4
  %11 = call i32* @_EN3std10OrderedMapI3int3intE8lowerKeyEP3int(%"OrderedMap<int, int>"* %m, i32* %2)
  %12 = icmp eq i32* %11, null
  %assert.condition = icmp eq i1 %12, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %loop.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @14, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.end
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %3, i32 1, i32 100)
  %13 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %3)
  store %"RangeIterator<int>" %13, %"RangeIterator<int>"* %__iterator1, align 4
  br label %loop.condition6

loop.condition6:                                  ; preds = %loop.increment9, %assert.success
  %14 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator1)
  br i1 %14, label %loop.body7, label %loop.end10

loop.body7:                                       ; preds = %loop.condition6
  %15 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator1)
  store i32 %15, i32* %i2, align 4
  %16 = call i32* @_EN3std10OrderedMapI3int3intE8lowerKeyEP3int(%"OrderedMap<int, int>"* %m, i32* %i2)
  %assert.condition8 = icmp eq i32* %16, null
  br i1 %assert.condition8, label %assert.fail12, label %assert.success13

loop.increment9:                                  ; preds = %assert.success17
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator1)
  br label %loop.condition6

loop.end10:                                       ; preds = %loop.condition6
  store i32 99, i32* %4, align 4
  %17 = call i32* @_EN3std10OrderedMapI3int3intE9higherKeyEP3int(%"OrderedMap<int, int>"* %m, i32* %4)
  %18 = icmp eq i32* %17, null
  %assert.condition11 = icmp eq i1 %18, false
  br i1 %assert.condition11, label %assert.fail18, label %assert.success19

assert.fail12:                                    ; preds = %loop.body7
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @15, i32 0, i32 0))
  unreachable

assert.success13:                                 ; preds = %loop.body7
  %i.load14 = load i32, i32* %i2, align 4
  %19 = sub i32 %i.load14, 1
  %.load = load i32, i32* %16, align 4
  %20 = icmp eq i32 %.load, %19
  %assert.condition15 = icmp eq i1 %20, false
  br i1 %assert.condition15, label %assert.fail16, label %assert.success17

assert.fail16:                                    ; preds = %assert.success13
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @16, i32 0, i32 0))
  unreachable

assert.success17:                                 ; preds = %assert.success13
  br label %loop.increment9

assert.fail18:                                    ; preds = %loop.end10
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @17, i32 0, i32 0))
  unreachable

assert.success19:                                 ; preds = %loop.end10
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %5, i32 0, i32 99)
  %21 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %5)
  store %"RangeIterator<int>" %21, %"RangeIterator<int>"* %__iterator3, align 4
  br label %loop.condition20

loop.condition20:                                 ; preds = %loop.increment23, %assert.success19
  %22 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator3)
  br i1 %22, label %loop.body21, label %loop.end24

loop.body21:                                      ; preds = %loop.condition20
  %23 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator3)
  store i32 %23, i32* %i4, align 4
  %24 = call i32* @_EN3std10OrderedMapI3int3intE9higherKeyEP3int(%"OrderedMap<int, int>"* %m, i32* %i4)
  %assert.condition22 = icmp eq i32* %24, null
  br i1 %assert.condition22, label %assert.fail25, label %assert.success26

loop.increment23:                                 ; preds = %assert.success31
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator3)
  br label %loop.condition20

loop.end24:                                       ; preds = %loop.condition20
  call void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %m)
  ret void

assert.fail25:                                    ; preds = %loop.body21
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @18, i32 0, i32 0))
  unreachable

assert.success26:                                 ; preds = %loop.body21
  %i.load27 = load i32, i32* %i4, align 4
  %25 = add i32 %i.load27, 1
  %.load28 = load i32, i32* %24, align 4
  %26 = icmp eq i32 %.load28, %25
  %assert.condition29 = icmp eq i1 %26, false
  br i1 %assert.condition29, label %assert.fail30, label %assert.success31

assert.fail30:                                    ; preds = %assert.success26
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @19, i32 0, i32 0))
  unreachable

assert.success31:                                 ; preds = %assert.success26
  br label %loop.increment23
}

define void @_EN4main13testFirstLastE() {
  %m = alloca %"OrderedMap<int, int>", align 8
  call void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %m)
  %1 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 63, i32 3)
  %2 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 69, i32 3)
  %3 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 1, i32 3)
  %4 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %m, i32 53, i32 3)
  %5 = call i32* @_EN3std10OrderedMapI3int3intE5firstE(%"OrderedMap<int, int>"* %m)
  %assert.condition = icmp eq i32* %5, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @20, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %.load = load i32, i32* %5, align 4
  %6 = icmp eq i32 %.load, 1
  %assert.condition1 = icmp eq i1 %6, false
  br i1 %assert.condition1, label %assert.fail2, label %assert.success3

assert.fail2:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @21, i32 0, i32 0))
  unreachable

assert.success3:                                  ; preds = %assert.success
  %7 = call i32* @_EN3std10OrderedMapI3int3intE4lastE(%"OrderedMap<int, int>"* %m)
  %assert.condition4 = icmp eq i32* %7, null
  br i1 %assert.condition4, label %assert.fail5, label %assert.success6

assert.fail5:                                     ; preds = %assert.success3
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @22, i32 0, i32 0))
  unreachable

assert.success6:                                  ; preds = %assert.success3
  %.load7 = load i32, i32* %7, align 4
  %8 = icmp eq i32 %.load7, 69
  %assert.condition8 = icmp eq i1 %8, false
  br i1 %assert.condition8, label %assert.fail9, label %assert.success10

assert.fail9:                                     ; preds = %assert.success6
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([47 x i8], [47 x i8]* @23, i32 0, i32 0))
  unreachable

assert.success10:                                 ; preds = %assert.success6
  call void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %m)
  ret void
}

define void @_EN4main21testEmptyMapFunctionsE() {
  %m = alloca %"OrderedMap<int, int>", align 8
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  call void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %m)
  %3 = call i32* @_EN3std10OrderedMapI3int3intE5firstE(%"OrderedMap<int, int>"* %m)
  %4 = icmp eq i32* %3, null
  %assert.condition = icmp eq i1 %4, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @24, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %5 = call i32* @_EN3std10OrderedMapI3int3intE4lastE(%"OrderedMap<int, int>"* %m)
  %6 = icmp eq i32* %5, null
  %assert.condition1 = icmp eq i1 %6, false
  br i1 %assert.condition1, label %assert.fail2, label %assert.success3

assert.fail2:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @25, i32 0, i32 0))
  unreachable

assert.success3:                                  ; preds = %assert.success
  store i32 48, i32* %1, align 4
  %7 = call i32* @_EN3std10OrderedMapI3int3intE9higherKeyEP3int(%"OrderedMap<int, int>"* %m, i32* %1)
  %8 = icmp eq i32* %7, null
  %assert.condition4 = icmp eq i1 %8, false
  br i1 %assert.condition4, label %assert.fail5, label %assert.success6

assert.fail5:                                     ; preds = %assert.success3
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @26, i32 0, i32 0))
  unreachable

assert.success6:                                  ; preds = %assert.success3
  store i32 42, i32* %2, align 4
  %9 = call i32* @_EN3std10OrderedMapI3int3intE8lowerKeyEP3int(%"OrderedMap<int, int>"* %m, i32* %2)
  %10 = icmp eq i32* %9, null
  %assert.condition7 = icmp eq i1 %10, false
  br i1 %assert.condition7, label %assert.fail8, label %assert.success9

assert.fail8:                                     ; preds = %assert.success6
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @27, i32 0, i32 0))
  unreachable

assert.success9:                                  ; preds = %assert.success6
  call void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %m)
  ret void
}

define void @_EN4main20testEmptyMapIteratorE() {
  %map = alloca %"OrderedMap<int, int>", align 8
  %1 = alloca i32, align 4
  %__iterator = alloca %"OrderedMapIterator<int, int>", align 8
  %e = alloca %"MapEntry<int, int>"*, align 8
  call void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %map)
  %2 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %map, i32 1, i32 2)
  %3 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %map, i32 1, i32 2)
  store i32 1, i32* %1, align 4
  call void @_EN3std10OrderedMapI3int3intE6removeEP3int(%"OrderedMap<int, int>"* %map, i32* %1)
  %4 = call %"OrderedMapIterator<int, int>" @_EN3std10OrderedMapI3int3intE8iteratorE(%"OrderedMap<int, int>"* %map)
  store %"OrderedMapIterator<int, int>" %4, %"OrderedMapIterator<int, int>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %5 = call i1 @_EN3std18OrderedMapIteratorI3int3intE8hasValueE(%"OrderedMapIterator<int, int>"* %__iterator)
  br i1 %5, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %6 = call %"MapEntry<int, int>"* @_EN3std18OrderedMapIteratorI3int3intE5valueE(%"OrderedMapIterator<int, int>"* %__iterator)
  store %"MapEntry<int, int>"* %6, %"MapEntry<int, int>"** %e, align 8
  br i1 true, label %assert.fail, label %assert.success

loop.increment:                                   ; preds = %assert.success
  call void @_EN3std18OrderedMapIteratorI3int3intE9incrementE(%"OrderedMapIterator<int, int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %map)
  ret void

assert.fail:                                      ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @28, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.body
  br label %loop.increment
}

define void @_EN4main17testIteratorOrderE() {
  %map = alloca %"OrderedMap<int, int>", align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %called = alloca i32, align 4
  %__iterator1 = alloca %"OrderedMapIterator<int, int>", align 8
  %e = alloca %"MapEntry<int, int>"*, align 8
  %2 = alloca %"OrderedMap<int, int>", align 8
  %__iterator2 = alloca %"RangeIterator<int>", align 8
  %3 = alloca %"Range<int>", align 8
  %i3 = alloca i32, align 4
  %__iterator4 = alloca %"OrderedMapIterator<int, int>", align 8
  %e5 = alloca %"MapEntry<int, int>"*, align 8
  call void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %map)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 100)
  %4 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %4, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %5 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %5, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %6 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %6, i32* %i, align 4
  %i.load = load i32, i32* %i, align 4
  %i.load6 = load i32, i32* %i, align 4
  %7 = sub i32 %i.load6, 12
  %8 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %map, i32 %i.load, i32 %7)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  store i32 0, i32* %called, align 4
  %9 = call %"OrderedMapIterator<int, int>" @_EN3std10OrderedMapI3int3intE8iteratorE(%"OrderedMap<int, int>"* %map)
  store %"OrderedMapIterator<int, int>" %9, %"OrderedMapIterator<int, int>"* %__iterator1, align 8
  br label %loop.condition7

loop.condition7:                                  ; preds = %loop.increment9, %loop.end
  %10 = call i1 @_EN3std18OrderedMapIteratorI3int3intE8hasValueE(%"OrderedMapIterator<int, int>"* %__iterator1)
  br i1 %10, label %loop.body8, label %loop.end10

loop.body8:                                       ; preds = %loop.condition7
  %11 = call %"MapEntry<int, int>"* @_EN3std18OrderedMapIteratorI3int3intE5valueE(%"OrderedMapIterator<int, int>"* %__iterator1)
  store %"MapEntry<int, int>"* %11, %"MapEntry<int, int>"** %e, align 8
  %e.load = load %"MapEntry<int, int>"*, %"MapEntry<int, int>"** %e, align 8
  %key = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %e.load, i32 0, i32 0
  %key.load = load i32, i32* %key, align 4
  %called.load = load i32, i32* %called, align 4
  %12 = icmp eq i32 %key.load, %called.load
  %assert.condition = icmp eq i1 %12, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

loop.increment9:                                  ; preds = %assert.success17
  call void @_EN3std18OrderedMapIteratorI3int3intE9incrementE(%"OrderedMapIterator<int, int>"* %__iterator1)
  br label %loop.condition7

loop.end10:                                       ; preds = %loop.condition7
  %called.load11 = load i32, i32* %called, align 4
  %13 = icmp eq i32 %called.load11, 100
  %assert.condition12 = icmp eq i1 %13, false
  br i1 %assert.condition12, label %assert.fail19, label %assert.success20

assert.fail:                                      ; preds = %loop.body8
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @29, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.body8
  %e.load13 = load %"MapEntry<int, int>"*, %"MapEntry<int, int>"** %e, align 8
  %value = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %e.load13, i32 0, i32 1
  %value.load = load i32, i32* %value, align 4
  %called.load14 = load i32, i32* %called, align 4
  %14 = sub i32 %called.load14, 12
  %15 = icmp eq i32 %value.load, %14
  %assert.condition15 = icmp eq i1 %15, false
  br i1 %assert.condition15, label %assert.fail16, label %assert.success17

assert.fail16:                                    ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @30, i32 0, i32 0))
  unreachable

assert.success17:                                 ; preds = %assert.success
  %called.load18 = load i32, i32* %called, align 4
  %16 = add i32 %called.load18, 1
  store i32 %16, i32* %called, align 4
  br label %loop.increment9

assert.fail19:                                    ; preds = %loop.end10
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @31, i32 0, i32 0))
  unreachable

assert.success20:                                 ; preds = %loop.end10
  call void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %map)
  call void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %2)
  %.load = load %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %2, align 8
  store %"OrderedMap<int, int>" %.load, %"OrderedMap<int, int>"* %map, align 8
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %3, i32 0, i32 100)
  %17 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %3)
  store %"RangeIterator<int>" %17, %"RangeIterator<int>"* %__iterator2, align 4
  br label %loop.condition21

loop.condition21:                                 ; preds = %loop.increment25, %assert.success20
  %18 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator2)
  br i1 %18, label %loop.body22, label %loop.end26

loop.body22:                                      ; preds = %loop.condition21
  %19 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator2)
  store i32 %19, i32* %i3, align 4
  %i.load23 = load i32, i32* %i3, align 4
  %20 = sub i32 99, %i.load23
  %i.load24 = load i32, i32* %i3, align 4
  %21 = sub i32 %i.load24, 12
  %22 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %map, i32 %20, i32 %21)
  br label %loop.increment25

loop.increment25:                                 ; preds = %loop.body22
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator2)
  br label %loop.condition21

loop.end26:                                       ; preds = %loop.condition21
  store i32 0, i32* %called, align 4
  %23 = call %"OrderedMapIterator<int, int>" @_EN3std10OrderedMapI3int3intE8iteratorE(%"OrderedMap<int, int>"* %map)
  store %"OrderedMapIterator<int, int>" %23, %"OrderedMapIterator<int, int>"* %__iterator4, align 8
  br label %loop.condition27

loop.condition27:                                 ; preds = %loop.increment34, %loop.end26
  %24 = call i1 @_EN3std18OrderedMapIteratorI3int3intE8hasValueE(%"OrderedMapIterator<int, int>"* %__iterator4)
  br i1 %24, label %loop.body28, label %loop.end35

loop.body28:                                      ; preds = %loop.condition27
  %25 = call %"MapEntry<int, int>"* @_EN3std18OrderedMapIteratorI3int3intE5valueE(%"OrderedMapIterator<int, int>"* %__iterator4)
  store %"MapEntry<int, int>"* %25, %"MapEntry<int, int>"** %e5, align 8
  %e.load29 = load %"MapEntry<int, int>"*, %"MapEntry<int, int>"** %e5, align 8
  %key30 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %e.load29, i32 0, i32 0
  %key.load31 = load i32, i32* %key30, align 4
  %called.load32 = load i32, i32* %called, align 4
  %26 = icmp eq i32 %key.load31, %called.load32
  %assert.condition33 = icmp eq i1 %26, false
  br i1 %assert.condition33, label %assert.fail38, label %assert.success39

loop.increment34:                                 ; preds = %assert.success46
  call void @_EN3std18OrderedMapIteratorI3int3intE9incrementE(%"OrderedMapIterator<int, int>"* %__iterator4)
  br label %loop.condition27

loop.end35:                                       ; preds = %loop.condition27
  %called.load36 = load i32, i32* %called, align 4
  %27 = icmp eq i32 %called.load36, 100
  %assert.condition37 = icmp eq i1 %27, false
  br i1 %assert.condition37, label %assert.fail48, label %assert.success49

assert.fail38:                                    ; preds = %loop.body28
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @32, i32 0, i32 0))
  unreachable

assert.success39:                                 ; preds = %loop.body28
  %e.load40 = load %"MapEntry<int, int>"*, %"MapEntry<int, int>"** %e5, align 8
  %value41 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %e.load40, i32 0, i32 1
  %value.load42 = load i32, i32* %value41, align 4
  %called.load43 = load i32, i32* %called, align 4
  %28 = sub i32 99, %called.load43
  %29 = sub i32 %28, 12
  %30 = icmp eq i32 %value.load42, %29
  %assert.condition44 = icmp eq i1 %30, false
  br i1 %assert.condition44, label %assert.fail45, label %assert.success46

assert.fail45:                                    ; preds = %assert.success39
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @33, i32 0, i32 0))
  unreachable

assert.success46:                                 ; preds = %assert.success39
  %called.load47 = load i32, i32* %called, align 4
  %31 = add i32 %called.load47, 1
  store i32 %31, i32* %called, align 4
  br label %loop.increment34

assert.fail48:                                    ; preds = %loop.end35
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @34, i32 0, i32 0))
  unreachable

assert.success49:                                 ; preds = %loop.end35
  call void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %map)
  ret void
}

define void @_EN4main19testUnitMapIteratorE() {
  %map = alloca %"OrderedMap<int, int>", align 8
  %count = alloca i32, align 4
  %__iterator = alloca %"OrderedMapIterator<int, int>", align 8
  %e = alloca %"MapEntry<int, int>"*, align 8
  call void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %map)
  %1 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %map, i32 1, i32 2)
  %2 = call i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %map, i32 1, i32 4)
  store i32 0, i32* %count, align 4
  %3 = call %"OrderedMapIterator<int, int>" @_EN3std10OrderedMapI3int3intE8iteratorE(%"OrderedMap<int, int>"* %map)
  store %"OrderedMapIterator<int, int>" %3, %"OrderedMapIterator<int, int>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %4 = call i1 @_EN3std18OrderedMapIteratorI3int3intE8hasValueE(%"OrderedMapIterator<int, int>"* %__iterator)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %5 = call %"MapEntry<int, int>"* @_EN3std18OrderedMapIteratorI3int3intE5valueE(%"OrderedMapIterator<int, int>"* %__iterator)
  store %"MapEntry<int, int>"* %5, %"MapEntry<int, int>"** %e, align 8
  %count.load = load i32, i32* %count, align 4
  %6 = add i32 %count.load, 1
  store i32 %6, i32* %count, align 4
  %e.load = load %"MapEntry<int, int>"*, %"MapEntry<int, int>"** %e, align 8
  %key = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %e.load, i32 0, i32 0
  %key.load = load i32, i32* %key, align 4
  %7 = icmp eq i32 %key.load, 1
  %assert.condition = icmp eq i1 %7, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

loop.increment:                                   ; preds = %assert.success
  call void @_EN3std18OrderedMapIteratorI3int3intE9incrementE(%"OrderedMapIterator<int, int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %count.load1 = load i32, i32* %count, align 4
  %8 = icmp eq i32 %count.load1, 1
  %assert.condition2 = icmp eq i1 %8, false
  br i1 %assert.condition2, label %assert.fail3, label %assert.success4

assert.fail:                                      ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @35, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.body
  br label %loop.increment

assert.fail3:                                     ; preds = %loop.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @36, i32 0, i32 0))
  unreachable

assert.success4:                                  ; preds = %loop.end
  call void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %map)
  ret void
}

define void @_EN3std10OrderedMapI3int3intE4initE(%"OrderedMap<int, int>"* %this) {
  %root = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  store %"AVLnode<int, int>"* null, %"AVLnode<int, int>"** %root, align 8
  %size = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  ret void
}

define i32 @_EN3std10OrderedMapI3int3intE4sizeE(%"OrderedMap<int, int>"* %this) {
  %size = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

declare void @_EN3std10assertFailEP4char(i8*)

define i1 @_EN3std10OrderedMapI3int3intE6insertE3int3int(%"OrderedMap<int, int>"* %this, i32 %key, i32 %value) {
  %1 = alloca %"AVLnode<int, int>", align 8
  %n = alloca %"AVLnode<int, int>"*, align 8
  %parent = alloca %"AVLnode<int, int>"*, align 8
  %goLeft = alloca i1, align 1
  %newNode = alloca %"AVLnode<int, int>"*, align 8
  %2 = alloca %"AVLnode<int, int>", align 8
  %3 = alloca %"AVLnode<int, int>", align 8
  %root = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root, align 8
  %4 = icmp eq %"AVLnode<int, int>"* %root.load, null
  br i1 %4, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %root1 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  call void @_EN3std7AVLnodeI3int3intE4initE3intOP7AVLnodeI3int3intE3int(%"AVLnode<int, int>"* %1, i32 %key, %"AVLnode<int, int>"* null, i32 %value)
  %.load = load %"AVLnode<int, int>", %"AVLnode<int, int>"* %1, align 8
  %5 = call %"AVLnode<int, int>"* @_EN3std8allocateI7AVLnodeI3int3intEEE7AVLnodeI3int3intE(%"AVLnode<int, int>" %.load)
  store %"AVLnode<int, int>"* %5, %"AVLnode<int, int>"** %root1, align 8
  br label %if.end

if.else:                                          ; preds = %0
  %root2 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load3 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root2, align 8
  store %"AVLnode<int, int>"* %root.load3, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition

if.end:                                           ; preds = %loop.end, %if.then
  %size = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %6 = add i32 %size.load, 1
  store i32 %6, i32* %size, align 4
  ret i1 true

loop.condition:                                   ; preds = %if.end21, %if.else
  br i1 true, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %n.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %entry = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load, i32 0, i32 0
  %key4 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry, i32 0, i32 0
  %key.load = load i32, i32* %key4, align 4
  %7 = icmp eq i32 %key.load, %key
  br i1 %7, label %if.then5, label %if.else6

loop.end:                                         ; preds = %__implicit_unwrap.success, %loop.condition
  br label %if.end

if.then5:                                         ; preds = %loop.body
  ret i1 false

if.else6:                                         ; preds = %loop.body
  br label %if.end7

if.end7:                                          ; preds = %if.else6
  %n.load8 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  store %"AVLnode<int, int>"* %n.load8, %"AVLnode<int, int>"** %parent, align 8
  %n.load9 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %entry10 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load9, i32 0, i32 0
  %key11 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry10, i32 0, i32 0
  %key.load12 = load i32, i32* %key11, align 4
  %8 = icmp sgt i32 %key.load12, %key
  store i1 %8, i1* %goLeft, align 1
  %goLeft.load = load i1, i1* %goLeft, align 1
  br i1 %goLeft.load, label %if.then13, label %if.else15

if.then13:                                        ; preds = %if.end7
  %n.load14 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load14, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  br label %if.end17

if.else15:                                        ; preds = %if.end7
  %n.load16 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load16, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  br label %if.end17

if.end17:                                         ; preds = %if.else15, %if.then13
  %if.result = phi %"AVLnode<int, int>"* [ %left.load, %if.then13 ], [ %right.load, %if.else15 ]
  store %"AVLnode<int, int>"* %if.result, %"AVLnode<int, int>"** %newNode, align 8
  %newNode.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %newNode, align 8
  %9 = icmp eq %"AVLnode<int, int>"* %newNode.load, null
  br i1 %9, label %if.then18, label %if.else20

if.then18:                                        ; preds = %if.end17
  %goLeft.load19 = load i1, i1* %goLeft, align 1
  br i1 %goLeft.load19, label %if.then23, label %if.else27

if.else20:                                        ; preds = %if.end17
  br label %if.end21

if.end21:                                         ; preds = %if.else20
  %newNode.load22 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %newNode, align 8
  store %"AVLnode<int, int>"* %newNode.load22, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition

if.then23:                                        ; preds = %if.then18
  %parent.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  %left24 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load, i32 0, i32 3
  %parent.load25 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  call void @_EN3std7AVLnodeI3int3intE4initE3intOP7AVLnodeI3int3intE3int(%"AVLnode<int, int>"* %2, i32 %key, %"AVLnode<int, int>"* %parent.load25, i32 %value)
  %.load26 = load %"AVLnode<int, int>", %"AVLnode<int, int>"* %2, align 8
  %10 = call %"AVLnode<int, int>"* @_EN3std8allocateI7AVLnodeI3int3intEEE7AVLnodeI3int3intE(%"AVLnode<int, int>" %.load26)
  store %"AVLnode<int, int>"* %10, %"AVLnode<int, int>"** %left24, align 8
  br label %if.end32

if.else27:                                        ; preds = %if.then18
  %parent.load28 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  %right29 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load28, i32 0, i32 4
  %parent.load30 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  call void @_EN3std7AVLnodeI3int3intE4initE3intOP7AVLnodeI3int3intE3int(%"AVLnode<int, int>"* %3, i32 %key, %"AVLnode<int, int>"* %parent.load30, i32 %value)
  %.load31 = load %"AVLnode<int, int>", %"AVLnode<int, int>"* %3, align 8
  %11 = call %"AVLnode<int, int>"* @_EN3std8allocateI7AVLnodeI3int3intEEE7AVLnodeI3int3intE(%"AVLnode<int, int>" %.load31)
  store %"AVLnode<int, int>"* %11, %"AVLnode<int, int>"** %right29, align 8
  br label %if.end32

if.end32:                                         ; preds = %if.else27, %if.then23
  %parent.load33 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %parent.load33, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %if.end32
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @37, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end32
  call void @_EN3std10OrderedMapI3int3intE9rebalanceEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %parent.load33)
  br label %loop.end
}

define i1 @_EN3std10OrderedMapI3int3intE8containsEP3int(%"OrderedMap<int, int>"* %this, i32* %key) {
  %1 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE8findNodeEP3int(%"OrderedMap<int, int>"* %this, i32* %key)
  %2 = icmp ne %"AVLnode<int, int>"* %1, null
  ret i1 %2
}

declare void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"*, i32, i32)

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"*)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"*)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"*)

declare void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

define void @_EN3std10OrderedMapI3int3intE6deinitE(%"OrderedMap<int, int>"* %this) {
  %root = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root, align 8
  %1 = icmp ne %"AVLnode<int, int>"* %root.load, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %root1 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load2 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root1, align 8
  call void @_EN3std7AVLnodeI3int3intE10deallocateE(%"AVLnode<int, int>"* %root.load2)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std10OrderedMapI3int3intE6removeEP3int(%"OrderedMap<int, int>"* %this, i32* %key) {
  %n = alloca %"AVLnode<int, int>"*, align 8
  %parent = alloca %"AVLnode<int, int>"*, align 8
  %delNode = alloca %"AVLnode<int, int>"*, align 8
  %child = alloca %"AVLnode<int, int>"*, align 8
  %root = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root, align 8
  %1 = icmp eq %"AVLnode<int, int>"* %root.load, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %root1 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load2 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root1, align 8
  store %"AVLnode<int, int>"* %root.load2, %"AVLnode<int, int>"** %n, align 8
  %root3 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load4 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root3, align 8
  store %"AVLnode<int, int>"* %root.load4, %"AVLnode<int, int>"** %parent, align 8
  store %"AVLnode<int, int>"* null, %"AVLnode<int, int>"** %delNode, align 8
  %root5 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load6 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root5, align 8
  store %"AVLnode<int, int>"* %root.load6, %"AVLnode<int, int>"** %child, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %if.end24, %if.end
  %child.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %child, align 8
  %2 = icmp ne %"AVLnode<int, int>"* %child.load, null
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %n.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  store %"AVLnode<int, int>"* %n.load, %"AVLnode<int, int>"** %parent, align 8
  %child.load7 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %child, align 8
  store %"AVLnode<int, int>"* %child.load7, %"AVLnode<int, int>"** %n, align 8
  %key.load = load i32, i32* %key, align 4
  %n.load8 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %entry = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load8, i32 0, i32 0
  %key9 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry, i32 0, i32 0
  %key.load10 = load i32, i32* %key9, align 4
  %3 = icmp sge i32 %key.load, %key.load10
  br i1 %3, label %if.then11, label %if.else13

loop.end:                                         ; preds = %loop.condition
  %delNode.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %delNode, align 8
  %4 = icmp ne %"AVLnode<int, int>"* %delNode.load, null
  br i1 %4, label %if.then25, label %if.else41

if.then11:                                        ; preds = %loop.body
  %n.load12 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load12, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  br label %if.end15

if.else13:                                        ; preds = %loop.body
  %n.load14 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load14, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  br label %if.end15

if.end15:                                         ; preds = %if.else13, %if.then11
  %if.result = phi %"AVLnode<int, int>"* [ %right.load, %if.then11 ], [ %left.load, %if.else13 ]
  store %"AVLnode<int, int>"* %if.result, %"AVLnode<int, int>"** %child, align 8
  %key.load16 = load i32, i32* %key, align 4
  %n.load17 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %entry18 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load17, i32 0, i32 0
  %key19 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry18, i32 0, i32 0
  %key.load20 = load i32, i32* %key19, align 4
  %5 = icmp eq i32 %key.load16, %key.load20
  br i1 %5, label %if.then21, label %if.else23

if.then21:                                        ; preds = %if.end15
  %n.load22 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  store %"AVLnode<int, int>"* %n.load22, %"AVLnode<int, int>"** %delNode, align 8
  br label %if.end24

if.else23:                                        ; preds = %if.end15
  br label %if.end24

if.end24:                                         ; preds = %if.else23, %if.then21
  br label %loop.condition

if.then25:                                        ; preds = %loop.end
  %delNode.load26 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %delNode, align 8
  %entry27 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %delNode.load26, i32 0, i32 0
  %key28 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry27, i32 0, i32 0
  %n.load29 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %entry30 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load29, i32 0, i32 0
  %key31 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry30, i32 0, i32 0
  %key.load32 = load i32, i32* %key31, align 4
  store i32 %key.load32, i32* %key28, align 4
  %delNode.load33 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %delNode, align 8
  %entry34 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %delNode.load33, i32 0, i32 0
  %value = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry34, i32 0, i32 1
  %n.load35 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %entry36 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load35, i32 0, i32 0
  %value37 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry36, i32 0, i32 1
  %value.load = load i32, i32* %value37, align 4
  store i32 %value.load, i32* %value, align 4
  %n.load38 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %left39 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load38, i32 0, i32 3
  %left.load40 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left39, align 8
  %6 = icmp ne %"AVLnode<int, int>"* %left.load40, null
  br i1 %6, label %if.then43, label %if.else47

if.else41:                                        ; preds = %loop.end
  br label %if.end42

if.end42:                                         ; preds = %if.end66, %if.else41
  %size = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %7 = add i32 %size.load, -1
  store i32 %7, i32* %size, align 4
  ret void

if.then43:                                        ; preds = %if.then25
  %n.load44 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %left45 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load44, i32 0, i32 3
  %left.load46 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left45, align 8
  br label %if.end51

if.else47:                                        ; preds = %if.then25
  %n.load48 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %right49 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load48, i32 0, i32 4
  %right.load50 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right49, align 8
  br label %if.end51

if.end51:                                         ; preds = %if.else47, %if.then43
  %if.result52 = phi %"AVLnode<int, int>"* [ %left.load46, %if.then43 ], [ %right.load50, %if.else47 ]
  store %"AVLnode<int, int>"* %if.result52, %"AVLnode<int, int>"** %child, align 8
  %root53 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load54 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root53, align 8
  %entry55 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %root.load54, i32 0, i32 0
  %key56 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry55, i32 0, i32 0
  %key.load57 = load i32, i32* %key56, align 4
  %key.load58 = load i32, i32* %key, align 4
  %8 = icmp eq i32 %key.load57, %key.load58
  br i1 %8, label %if.then59, label %if.else62

if.then59:                                        ; preds = %if.end51
  %root60 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %child.load61 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %child, align 8
  store %"AVLnode<int, int>"* %child.load61, %"AVLnode<int, int>"** %root60, align 8
  br label %if.end66

if.else62:                                        ; preds = %if.end51
  %parent.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  %left63 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load, i32 0, i32 3
  %left.load64 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left63, align 8
  %n.load65 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %9 = icmp eq %"AVLnode<int, int>"* %left.load64, %n.load65
  br i1 %9, label %if.then68, label %if.else72

if.end66:                                         ; preds = %__implicit_unwrap.success, %if.then59
  %n.load67 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  call void @_EN3std10deallocateIOP7AVLnodeI3int3intEEEOP7AVLnodeI3int3intE(%"AVLnode<int, int>"* %n.load67)
  br label %if.end42

if.then68:                                        ; preds = %if.else62
  %parent.load69 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  %left70 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load69, i32 0, i32 3
  %child.load71 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %child, align 8
  store %"AVLnode<int, int>"* %child.load71, %"AVLnode<int, int>"** %left70, align 8
  br label %if.end76

if.else72:                                        ; preds = %if.else62
  %parent.load73 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  %right74 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load73, i32 0, i32 4
  %child.load75 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %child, align 8
  store %"AVLnode<int, int>"* %child.load75, %"AVLnode<int, int>"** %right74, align 8
  br label %if.end76

if.end76:                                         ; preds = %if.else72, %if.then68
  %parent.load77 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %parent.load77, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %if.end76
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @38, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end76
  call void @_EN3std10OrderedMapI3int3intE9rebalanceEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %parent.load77)
  br label %if.end66
}

define i32* @_EN3std10OrderedMapI3int3intEixEP3int(%"OrderedMap<int, int>"* %this, i32* %key) {
  %found = alloca %"AVLnode<int, int>"*, align 8
  %1 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE8findNodeEP3int(%"OrderedMap<int, int>"* %this, i32* %key)
  store %"AVLnode<int, int>"* %1, %"AVLnode<int, int>"** %found, align 8
  %found.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %found, align 8
  %2 = icmp ne %"AVLnode<int, int>"* %found.load, null
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %found.load1 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %found, align 8
  %entry = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %found.load1, i32 0, i32 0
  %value = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry, i32 0, i32 1
  ret i32* %value

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  ret i32* null
}

define i1 @_EN3stdeqI3intEEP3intP3int(i32* %a, i32* %b) {
  %1 = call i32 @_EN3std3int7compareEP3int(i32* %a, i32* %b)
  %2 = icmp eq i32 %1, 1
  ret i1 %2
}

define i32* @_EN3std10OrderedMapI3int3intE8lowerKeyEP3int(%"OrderedMap<int, int>"* %this, i32* %key) {
  %found = alloca %"AVLnode<int, int>"*, align 8
  %lower = alloca %"AVLnode<int, int>"*, align 8
  %1 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE8findNodeEP3int(%"OrderedMap<int, int>"* %this, i32* %key)
  store %"AVLnode<int, int>"* %1, %"AVLnode<int, int>"** %found, align 8
  %found.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %found, align 8
  %2 = icmp eq %"AVLnode<int, int>"* %found.load, null
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i32* null

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %found.load1 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %found, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %found.load1, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %if.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @39, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end
  %3 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE9lowerNodeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %found.load1)
  store %"AVLnode<int, int>"* %3, %"AVLnode<int, int>"** %lower, align 8
  %lower.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %lower, align 8
  %4 = icmp eq %"AVLnode<int, int>"* %lower.load, null
  br i1 %4, label %if.then2, label %if.else3

if.then2:                                         ; preds = %__implicit_unwrap.success
  ret i32* null

if.else3:                                         ; preds = %__implicit_unwrap.success
  br label %if.end4

if.end4:                                          ; preds = %if.else3
  %lower.load5 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %lower, align 8
  %entry = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %lower.load5, i32 0, i32 0
  %key6 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry, i32 0, i32 0
  ret i32* %key6
}

define i32* @_EN3std10OrderedMapI3int3intE9higherKeyEP3int(%"OrderedMap<int, int>"* %this, i32* %key) {
  %found = alloca %"AVLnode<int, int>"*, align 8
  %higher = alloca %"AVLnode<int, int>"*, align 8
  %1 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE8findNodeEP3int(%"OrderedMap<int, int>"* %this, i32* %key)
  store %"AVLnode<int, int>"* %1, %"AVLnode<int, int>"** %found, align 8
  %found.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %found, align 8
  %2 = icmp eq %"AVLnode<int, int>"* %found.load, null
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i32* null

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %found.load1 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %found, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %found.load1, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %if.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @40, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end
  %3 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE10higherNodeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %found.load1)
  store %"AVLnode<int, int>"* %3, %"AVLnode<int, int>"** %higher, align 8
  %higher.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %higher, align 8
  %4 = icmp eq %"AVLnode<int, int>"* %higher.load, null
  br i1 %4, label %if.then2, label %if.else3

if.then2:                                         ; preds = %__implicit_unwrap.success
  ret i32* null

if.else3:                                         ; preds = %__implicit_unwrap.success
  br label %if.end4

if.end4:                                          ; preds = %if.else3
  %higher.load5 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %higher, align 8
  %entry = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %higher.load5, i32 0, i32 0
  %key6 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry, i32 0, i32 0
  ret i32* %key6
}

define i32* @_EN3std10OrderedMapI3int3intE5firstE(%"OrderedMap<int, int>"* %this) {
  %root = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root, align 8
  %1 = icmp eq %"AVLnode<int, int>"* %root.load, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i32* null

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %root1 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load2 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root1, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %root.load2, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %if.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @41, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end
  %2 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE12minInSubtreeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %root.load2)
  %entry = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %2, i32 0, i32 0
  %key = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry, i32 0, i32 0
  ret i32* %key
}

define i32* @_EN3std10OrderedMapI3int3intE4lastE(%"OrderedMap<int, int>"* %this) {
  %root = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root, align 8
  %1 = icmp eq %"AVLnode<int, int>"* %root.load, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i32* null

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %root1 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load2 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root1, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %root.load2, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %if.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @42, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end
  %2 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE12maxInSubtreeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %root.load2)
  %entry = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %2, i32 0, i32 0
  %key = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry, i32 0, i32 0
  ret i32* %key
}

define %"OrderedMapIterator<int, int>" @_EN3std10OrderedMapI3int3intE8iteratorE(%"OrderedMap<int, int>"* %this) {
  %1 = alloca %"OrderedMapIterator<int, int>", align 8
  call void @_EN3std18OrderedMapIteratorI3int3intE4initEP10OrderedMapI3int3intE(%"OrderedMapIterator<int, int>"* %1, %"OrderedMap<int, int>"* %this)
  %.load = load %"OrderedMapIterator<int, int>", %"OrderedMapIterator<int, int>"* %1, align 8
  ret %"OrderedMapIterator<int, int>" %.load
}

define i1 @_EN3std18OrderedMapIteratorI3int3intE8hasValueE(%"OrderedMapIterator<int, int>"* %this) {
  %current = getelementptr inbounds %"OrderedMapIterator<int, int>", %"OrderedMapIterator<int, int>"* %this, i32 0, i32 0
  %current.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %current, align 8
  %1 = icmp ne %"AVLnode<int, int>"* %current.load, null
  ret i1 %1
}

define %"MapEntry<int, int>"* @_EN3std18OrderedMapIteratorI3int3intE5valueE(%"OrderedMapIterator<int, int>"* %this) {
  %current = getelementptr inbounds %"OrderedMapIterator<int, int>", %"OrderedMapIterator<int, int>"* %this, i32 0, i32 0
  %current.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %current, align 8
  %assert.condition = icmp eq %"AVLnode<int, int>"* %current.load, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([46 x i8], [46 x i8]* @43, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %entry = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %current.load, i32 0, i32 0
  ret %"MapEntry<int, int>"* %entry
}

define void @_EN3std18OrderedMapIteratorI3int3intE9incrementE(%"OrderedMapIterator<int, int>"* %this) {
  %current = getelementptr inbounds %"OrderedMapIterator<int, int>", %"OrderedMapIterator<int, int>"* %this, i32 0, i32 0
  %map = getelementptr inbounds %"OrderedMapIterator<int, int>", %"OrderedMapIterator<int, int>"* %this, i32 0, i32 1
  %map.load = load %"OrderedMap<int, int>"*, %"OrderedMap<int, int>"** %map, align 8
  %current1 = getelementptr inbounds %"OrderedMapIterator<int, int>", %"OrderedMapIterator<int, int>"* %this, i32 0, i32 0
  %current.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %current1, align 8
  %assert.condition = icmp eq %"AVLnode<int, int>"* %current.load, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([46 x i8], [46 x i8]* @44, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %1 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE10higherNodeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %map.load, %"AVLnode<int, int>"* %current.load)
  store %"AVLnode<int, int>"* %1, %"AVLnode<int, int>"** %current, align 8
  ret void
}

define void @_EN3std7AVLnodeI3int3intE10deallocateE(%"AVLnode<int, int>"* %this) {
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %this, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  %1 = icmp ne %"AVLnode<int, int>"* %left.load, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %left1 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %this, i32 0, i32 3
  %left.load2 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left1, align 8
  call void @_EN3std7AVLnodeI3int3intE10deallocateE(%"AVLnode<int, int>"* %left.load2)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %this, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  %2 = icmp ne %"AVLnode<int, int>"* %right.load, null
  br i1 %2, label %if.then3, label %if.else6

if.then3:                                         ; preds = %if.end
  %right4 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %this, i32 0, i32 4
  %right.load5 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right4, align 8
  call void @_EN3std7AVLnodeI3int3intE10deallocateE(%"AVLnode<int, int>"* %right.load5)
  br label %if.end7

if.else6:                                         ; preds = %if.end
  br label %if.end7

if.end7:                                          ; preds = %if.else6, %if.then3
  call void @_EN3std10deallocateIP7AVLnodeI3int3intEEEP7AVLnodeI3int3intE(%"AVLnode<int, int>"* %this)
  ret void
}

define void @_EN3std7AVLnodeI3int3intE4initE3intOP7AVLnodeI3int3intE3int(%"AVLnode<int, int>"* %this, i32 %key, %"AVLnode<int, int>"* %parent, i32 %value) {
  %1 = alloca %"MapEntry<int, int>", align 8
  %entry = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %this, i32 0, i32 0
  call void @_EN3std8MapEntryI3int3intE4initE3int3int(%"MapEntry<int, int>"* %1, i32 %key, i32 %value)
  %.load = load %"MapEntry<int, int>", %"MapEntry<int, int>"* %1, align 4
  store %"MapEntry<int, int>" %.load, %"MapEntry<int, int>"* %entry, align 4
  %parent1 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %this, i32 0, i32 2
  store %"AVLnode<int, int>"* %parent, %"AVLnode<int, int>"** %parent1, align 8
  %balance = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %this, i32 0, i32 1
  store i32 0, i32* %balance, align 4
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %this, i32 0, i32 3
  store %"AVLnode<int, int>"* null, %"AVLnode<int, int>"** %left, align 8
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %this, i32 0, i32 4
  store %"AVLnode<int, int>"* null, %"AVLnode<int, int>"** %right, align 8
  ret void
}

define %"AVLnode<int, int>"* @_EN3std8allocateI7AVLnodeI3int3intEEE7AVLnodeI3int3intE(%"AVLnode<int, int>" %value) {
  %allocation = alloca %"AVLnode<int, int>"*, align 8
  %1 = call i8* @malloc(i64 ptrtoint (%"AVLnode<int, int>"* getelementptr (%"AVLnode<int, int>", %"AVLnode<int, int>"* null, i32 1) to i64))
  %assert.condition = icmp eq i8* %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([35 x i8], [35 x i8]* @45, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %2 = bitcast i8* %1 to %"AVLnode<int, int>"*
  store %"AVLnode<int, int>"* %2, %"AVLnode<int, int>"** %allocation, align 8
  %allocation.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %allocation, align 8
  store %"AVLnode<int, int>" %value, %"AVLnode<int, int>"* %allocation.load, align 8
  %allocation.load1 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %allocation, align 8
  ret %"AVLnode<int, int>"* %allocation.load1
}

define void @_EN3std10OrderedMapI3int3intE9rebalanceEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %node) {
  %n = alloca %"AVLnode<int, int>"*, align 8
  store %"AVLnode<int, int>"* %node, %"AVLnode<int, int>"** %n, align 8
  %n.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  call void @_EN3std10OrderedMapI3int3intE10setBalanceEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n.load)
  %n.load1 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %balance = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load1, i32 0, i32 1
  %balance.load = load i32, i32* %balance, align 4
  %1 = icmp eq i32 %balance.load, -2
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %n.load2 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load2, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  %assert.condition = icmp eq %"AVLnode<int, int>"* %left.load, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

if.else:                                          ; preds = %0
  %n.load3 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %balance4 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load3, i32 0, i32 1
  %balance.load5 = load i32, i32* %balance4, align 4
  %2 = icmp eq i32 %balance.load5, 2
  br i1 %2, label %if.then17, label %if.else22

if.end:                                           ; preds = %if.end23, %if.end16
  %n.load6 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %parent = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load6, i32 0, i32 2
  %parent.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  %3 = icmp ne %"AVLnode<int, int>"* %parent.load, null
  br i1 %3, label %if.then38, label %if.else42

assert.fail:                                      ; preds = %if.then
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @46, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %if.then
  %left7 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %left.load, i32 0, i32 3
  %left.load8 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left7, align 8
  %4 = call i32 @_EN3std10OrderedMapI3int3intE6heightEOP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %left.load8)
  %n.load9 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %left10 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load9, i32 0, i32 3
  %left.load11 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left10, align 8
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %left.load11, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  %5 = call i32 @_EN3std10OrderedMapI3int3intE6heightEOP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %right.load)
  %6 = icmp sge i32 %4, %5
  br i1 %6, label %if.then12, label %if.else14

if.then12:                                        ; preds = %assert.success
  %n.load13 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %7 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE11rotateRightEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n.load13)
  store %"AVLnode<int, int>"* %7, %"AVLnode<int, int>"** %n, align 8
  br label %if.end16

if.else14:                                        ; preds = %assert.success
  %n.load15 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %8 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE19rotateLeftThenRightEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n.load15)
  store %"AVLnode<int, int>"* %8, %"AVLnode<int, int>"** %n, align 8
  br label %if.end16

if.end16:                                         ; preds = %if.else14, %if.then12
  br label %if.end

if.then17:                                        ; preds = %if.else
  %n.load18 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %right19 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load18, i32 0, i32 4
  %right.load20 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right19, align 8
  %assert.condition21 = icmp eq %"AVLnode<int, int>"* %right.load20, null
  br i1 %assert.condition21, label %assert.fail24, label %assert.success25

if.else22:                                        ; preds = %if.else
  br label %if.end23

if.end23:                                         ; preds = %if.end37, %if.else22
  br label %if.end

assert.fail24:                                    ; preds = %if.then17
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @47, i32 0, i32 0))
  unreachable

assert.success25:                                 ; preds = %if.then17
  %right26 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %right.load20, i32 0, i32 4
  %right.load27 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right26, align 8
  %9 = call i32 @_EN3std10OrderedMapI3int3intE6heightEOP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %right.load27)
  %n.load28 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %right29 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load28, i32 0, i32 4
  %right.load30 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right29, align 8
  %left31 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %right.load30, i32 0, i32 3
  %left.load32 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left31, align 8
  %10 = call i32 @_EN3std10OrderedMapI3int3intE6heightEOP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %left.load32)
  %11 = icmp sge i32 %9, %10
  br i1 %11, label %if.then33, label %if.else35

if.then33:                                        ; preds = %assert.success25
  %n.load34 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %12 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE10rotateLeftEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n.load34)
  store %"AVLnode<int, int>"* %12, %"AVLnode<int, int>"** %n, align 8
  br label %if.end37

if.else35:                                        ; preds = %assert.success25
  %n.load36 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %13 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE19rotateRightThenLeftEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n.load36)
  store %"AVLnode<int, int>"* %13, %"AVLnode<int, int>"** %n, align 8
  br label %if.end37

if.end37:                                         ; preds = %if.else35, %if.then33
  br label %if.end23

if.then38:                                        ; preds = %if.end
  %n.load39 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %parent40 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load39, i32 0, i32 2
  %parent.load41 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent40, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %parent.load41, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

if.else42:                                        ; preds = %if.end
  %root = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %n.load43 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  store %"AVLnode<int, int>"* %n.load43, %"AVLnode<int, int>"** %root, align 8
  br label %if.end44

if.end44:                                         ; preds = %__implicit_unwrap.success, %if.else42
  ret void

__implicit_unwrap.fail:                           ; preds = %if.then38
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @48, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.then38
  call void @_EN3std10OrderedMapI3int3intE9rebalanceEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %parent.load41)
  br label %if.end44
}

define %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE8findNodeEP3int(%"OrderedMap<int, int>"* %this, i32* %key) {
  %n = alloca %"AVLnode<int, int>"*, align 8
  %goLeft = alloca i1, align 1
  %root = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root, align 8
  store %"AVLnode<int, int>"* %root.load, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %if.end17, %0
  br i1 true, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %n.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %1 = icmp eq %"AVLnode<int, int>"* %n.load, null
  br i1 %1, label %if.then, label %if.else

loop.end:                                         ; preds = %loop.condition
  ret %"AVLnode<int, int>"* null

if.then:                                          ; preds = %loop.body
  ret %"AVLnode<int, int>"* null

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  %n.load1 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %entry = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load1, i32 0, i32 0
  %key2 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry, i32 0, i32 0
  %key.load = load i32, i32* %key2, align 4
  %key.load3 = load i32, i32* %key, align 4
  %2 = icmp eq i32 %key.load, %key.load3
  br i1 %2, label %if.then4, label %if.else6

if.then4:                                         ; preds = %if.end
  %n.load5 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  ret %"AVLnode<int, int>"* %n.load5

if.else6:                                         ; preds = %if.end
  br label %if.end7

if.end7:                                          ; preds = %if.else6
  %n.load8 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %entry9 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load8, i32 0, i32 0
  %key10 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %entry9, i32 0, i32 0
  %key.load11 = load i32, i32* %key10, align 4
  %key.load12 = load i32, i32* %key, align 4
  %3 = icmp sgt i32 %key.load11, %key.load12
  store i1 %3, i1* %goLeft, align 1
  %goLeft.load = load i1, i1* %goLeft, align 1
  br i1 %goLeft.load, label %if.then13, label %if.else15

if.then13:                                        ; preds = %if.end7
  %n.load14 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load14, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  br label %if.end17

if.else15:                                        ; preds = %if.end7
  %n.load16 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load16, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  br label %if.end17

if.end17:                                         ; preds = %if.else15, %if.then13
  %if.result = phi %"AVLnode<int, int>"* [ %left.load, %if.then13 ], [ %right.load, %if.else15 ]
  store %"AVLnode<int, int>"* %if.result, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition
}

define void @_EN3std10deallocateIOP7AVLnodeI3int3intEEEOP7AVLnodeI3int3intE(%"AVLnode<int, int>"* %allocation) {
  %1 = bitcast %"AVLnode<int, int>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define i32 @_EN3std3int7compareEP3int(i32* %this, i32* %other) {
  %this.load = load i32, i32* %this, align 4
  %other.load = load i32, i32* %other, align 4
  %1 = icmp slt i32 %this.load, %other.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end5

if.else:                                          ; preds = %0
  %this.load1 = load i32, i32* %this, align 4
  %other.load2 = load i32, i32* %other, align 4
  %2 = icmp sgt i32 %this.load1, %other.load2
  br i1 %2, label %if.then3, label %if.else4

if.then3:                                         ; preds = %if.else
  br label %if.end

if.else4:                                         ; preds = %if.else
  br label %if.end

if.end:                                           ; preds = %if.else4, %if.then3
  %if.result = phi i32 [ 2, %if.then3 ], [ 1, %if.else4 ]
  br label %if.end5

if.end5:                                          ; preds = %if.end, %if.then
  %if.result6 = phi i32 [ 0, %if.then ], [ %if.result, %if.end ]
  ret i32 %if.result6
}

define %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE9lowerNodeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %node) {
  %n = alloca %"AVLnode<int, int>"*, align 8
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %node, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  %1 = icmp ne %"AVLnode<int, int>"* %left.load, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %left1 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %node, i32 0, i32 3
  %left.load2 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left1, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %left.load2, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  store %"AVLnode<int, int>"* %node, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition

__implicit_unwrap.fail:                           ; preds = %if.then
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @49, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.then
  %2 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE12maxInSubtreeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %left.load2)
  ret %"AVLnode<int, int>"* %2

loop.condition:                                   ; preds = %__implicit_unwrap.success18, %if.end
  %n.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %parent = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load, i32 0, i32 2
  %parent.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  %3 = icmp ne %"AVLnode<int, int>"* %parent.load, null
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %n.load3 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %n.load4 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %parent5 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load4, i32 0, i32 2
  %parent.load6 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent5, align 8
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load6, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  %4 = icmp eq %"AVLnode<int, int>"* %n.load3, %right.load
  br i1 %4, label %if.then7, label %if.else11

loop.end:                                         ; preds = %loop.condition
  ret %"AVLnode<int, int>"* null

if.then7:                                         ; preds = %loop.body
  %n.load8 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %parent9 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load8, i32 0, i32 2
  %parent.load10 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent9, align 8
  ret %"AVLnode<int, int>"* %parent.load10

if.else11:                                        ; preds = %loop.body
  br label %if.end12

if.end12:                                         ; preds = %if.else11
  %n.load13 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %parent14 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load13, i32 0, i32 2
  %parent.load15 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent14, align 8
  %__implicit_unwrap.condition16 = icmp eq %"AVLnode<int, int>"* %parent.load15, null
  br i1 %__implicit_unwrap.condition16, label %__implicit_unwrap.fail17, label %__implicit_unwrap.success18

__implicit_unwrap.fail17:                         ; preds = %if.end12
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @50, i32 0, i32 0))
  unreachable

__implicit_unwrap.success18:                      ; preds = %if.end12
  store %"AVLnode<int, int>"* %parent.load15, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition
}

define %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE10higherNodeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %node) {
  %n = alloca %"AVLnode<int, int>"*, align 8
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %node, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  %1 = icmp ne %"AVLnode<int, int>"* %right.load, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %right1 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %node, i32 0, i32 4
  %right.load2 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right1, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %right.load2, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  store %"AVLnode<int, int>"* %node, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition

__implicit_unwrap.fail:                           ; preds = %if.then
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @51, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.then
  %2 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE12minInSubtreeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %right.load2)
  ret %"AVLnode<int, int>"* %2

loop.condition:                                   ; preds = %__implicit_unwrap.success18, %if.end
  %n.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %parent = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load, i32 0, i32 2
  %parent.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent, align 8
  %3 = icmp ne %"AVLnode<int, int>"* %parent.load, null
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %n.load3 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %n.load4 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %parent5 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load4, i32 0, i32 2
  %parent.load6 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent5, align 8
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load6, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  %4 = icmp eq %"AVLnode<int, int>"* %n.load3, %left.load
  br i1 %4, label %if.then7, label %if.else11

loop.end:                                         ; preds = %loop.condition
  ret %"AVLnode<int, int>"* null

if.then7:                                         ; preds = %loop.body
  %n.load8 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %parent9 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load8, i32 0, i32 2
  %parent.load10 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent9, align 8
  ret %"AVLnode<int, int>"* %parent.load10

if.else11:                                        ; preds = %loop.body
  br label %if.end12

if.end12:                                         ; preds = %if.else11
  %n.load13 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %parent14 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load13, i32 0, i32 2
  %parent.load15 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent14, align 8
  %__implicit_unwrap.condition16 = icmp eq %"AVLnode<int, int>"* %parent.load15, null
  br i1 %__implicit_unwrap.condition16, label %__implicit_unwrap.fail17, label %__implicit_unwrap.success18

__implicit_unwrap.fail17:                         ; preds = %if.end12
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @52, i32 0, i32 0))
  unreachable

__implicit_unwrap.success18:                      ; preds = %if.end12
  store %"AVLnode<int, int>"* %parent.load15, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition
}

define %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE12minInSubtreeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %node) {
  %n = alloca %"AVLnode<int, int>"*, align 8
  store %"AVLnode<int, int>"* %node, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %__implicit_unwrap.success, %0
  %n.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  %1 = icmp ne %"AVLnode<int, int>"* %left.load, null
  br i1 %1, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %n.load1 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %left2 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load1, i32 0, i32 3
  %left.load3 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left2, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %left.load3, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

loop.end:                                         ; preds = %loop.condition
  %n.load4 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  ret %"AVLnode<int, int>"* %n.load4

__implicit_unwrap.fail:                           ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @53, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %loop.body
  store %"AVLnode<int, int>"* %left.load3, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition
}

define %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE12maxInSubtreeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %node) {
  %n = alloca %"AVLnode<int, int>"*, align 8
  store %"AVLnode<int, int>"* %node, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %__implicit_unwrap.success, %0
  %n.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  %1 = icmp ne %"AVLnode<int, int>"* %right.load, null
  br i1 %1, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %n.load1 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  %right2 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n.load1, i32 0, i32 4
  %right.load3 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right2, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %right.load3, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

loop.end:                                         ; preds = %loop.condition
  %n.load4 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %n, align 8
  ret %"AVLnode<int, int>"* %n.load4

__implicit_unwrap.fail:                           ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @54, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %loop.body
  store %"AVLnode<int, int>"* %right.load3, %"AVLnode<int, int>"** %n, align 8
  br label %loop.condition
}

define void @_EN3std18OrderedMapIteratorI3int3intE4initEP10OrderedMapI3int3intE(%"OrderedMapIterator<int, int>"* %this, %"OrderedMap<int, int>"* %map) {
  %current = getelementptr inbounds %"OrderedMapIterator<int, int>", %"OrderedMapIterator<int, int>"* %this, i32 0, i32 0
  %1 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE9firstNodeE(%"OrderedMap<int, int>"* %map)
  store %"AVLnode<int, int>"* %1, %"AVLnode<int, int>"** %current, align 8
  %map1 = getelementptr inbounds %"OrderedMapIterator<int, int>", %"OrderedMapIterator<int, int>"* %this, i32 0, i32 1
  store %"OrderedMap<int, int>"* %map, %"OrderedMap<int, int>"** %map1, align 8
  ret void
}

define void @_EN3std10deallocateIP7AVLnodeI3int3intEEEP7AVLnodeI3int3intE(%"AVLnode<int, int>"* %allocation) {
  %1 = bitcast %"AVLnode<int, int>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

declare i8* @malloc(i64)

define void @_EN3std8MapEntryI3int3intE4initE3int3int(%"MapEntry<int, int>"* %this, i32 %key, i32 %value) {
  %key1 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %this, i32 0, i32 0
  store i32 %key, i32* %key1, align 4
  %value2 = getelementptr inbounds %"MapEntry<int, int>", %"MapEntry<int, int>"* %this, i32 0, i32 1
  store i32 %value, i32* %value2, align 4
  ret void
}

define void @_EN3std10OrderedMapI3int3intE10setBalanceEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n) {
  %balance = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n, i32 0, i32 1
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  %1 = call i32 @_EN3std10OrderedMapI3int3intE6heightEOP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %right.load)
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  %2 = call i32 @_EN3std10OrderedMapI3int3intE6heightEOP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %left.load)
  %3 = sub i32 %1, %2
  store i32 %3, i32* %balance, align 4
  ret void
}

define i32 @_EN3std10OrderedMapI3int3intE6heightEOP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n) {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = icmp eq %"AVLnode<int, int>"* %n, null
  br i1 %3, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i32 -1

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  %4 = call i32 @_EN3std10OrderedMapI3int3intE6heightEOP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %left.load)
  store i32 %4, i32* %1, align 4
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  %5 = call i32 @_EN3std10OrderedMapI3int3intE6heightEOP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %right.load)
  store i32 %5, i32* %2, align 4
  %6 = call i32* @_EN3std3maxI3intEEP3intP3int(i32* %1, i32* %2)
  %.load = load i32, i32* %6, align 4
  %7 = add i32 1, %.load
  ret i32 %7
}

define %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE11rotateRightEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %node) {
  %a = alloca %"AVLnode<int, int>"*, align 8
  %b = alloca %"AVLnode<int, int>"*, align 8
  store %"AVLnode<int, int>"* %node, %"AVLnode<int, int>"** %a, align 8
  %a.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  %assert.condition = icmp eq %"AVLnode<int, int>"* %left.load, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @55, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  store %"AVLnode<int, int>"* %left.load, %"AVLnode<int, int>"** %b, align 8
  %b.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %parent = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load, i32 0, i32 2
  %a.load1 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %parent2 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load1, i32 0, i32 2
  %parent.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent2, align 8
  store %"AVLnode<int, int>"* %parent.load, %"AVLnode<int, int>"** %parent, align 8
  %a.load3 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %left4 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load3, i32 0, i32 3
  %b.load5 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load5, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  store %"AVLnode<int, int>"* %right.load, %"AVLnode<int, int>"** %left4, align 8
  %a.load6 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %left7 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load6, i32 0, i32 3
  %left.load8 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left7, align 8
  %1 = icmp ne %"AVLnode<int, int>"* %left.load8, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %assert.success
  %a.load9 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %left10 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load9, i32 0, i32 3
  %left.load11 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left10, align 8
  %parent12 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %left.load11, i32 0, i32 2
  %a.load13 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  store %"AVLnode<int, int>"* %a.load13, %"AVLnode<int, int>"** %parent12, align 8
  br label %if.end

if.else:                                          ; preds = %assert.success
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %b.load14 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %right15 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load14, i32 0, i32 4
  %a.load16 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  store %"AVLnode<int, int>"* %a.load16, %"AVLnode<int, int>"** %right15, align 8
  %a.load17 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %parent18 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load17, i32 0, i32 2
  %b.load19 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  store %"AVLnode<int, int>"* %b.load19, %"AVLnode<int, int>"** %parent18, align 8
  %b.load20 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %parent21 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load20, i32 0, i32 2
  %parent.load22 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent21, align 8
  %2 = icmp ne %"AVLnode<int, int>"* %parent.load22, null
  br i1 %2, label %if.then23, label %if.else30

if.then23:                                        ; preds = %if.end
  %b.load24 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %parent25 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load24, i32 0, i32 2
  %parent.load26 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent25, align 8
  %right27 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load26, i32 0, i32 4
  %right.load28 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right27, align 8
  %a.load29 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %3 = icmp eq %"AVLnode<int, int>"* %right.load28, %a.load29
  br i1 %3, label %if.then35, label %if.else41

if.else30:                                        ; preds = %if.end
  br label %if.end31

if.end31:                                         ; preds = %if.end47, %if.else30
  %a.load32 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  call void @_EN3std10OrderedMapI3int3intE10setBalanceEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %a.load32)
  %b.load33 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  call void @_EN3std10OrderedMapI3int3intE10setBalanceEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %b.load33)
  %b.load34 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  ret %"AVLnode<int, int>"* %b.load34

if.then35:                                        ; preds = %if.then23
  %b.load36 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %parent37 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load36, i32 0, i32 2
  %parent.load38 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent37, align 8
  %right39 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load38, i32 0, i32 4
  %b.load40 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  store %"AVLnode<int, int>"* %b.load40, %"AVLnode<int, int>"** %right39, align 8
  br label %if.end47

if.else41:                                        ; preds = %if.then23
  %b.load42 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %parent43 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load42, i32 0, i32 2
  %parent.load44 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent43, align 8
  %left45 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load44, i32 0, i32 3
  %b.load46 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  store %"AVLnode<int, int>"* %b.load46, %"AVLnode<int, int>"** %left45, align 8
  br label %if.end47

if.end47:                                         ; preds = %if.else41, %if.then35
  br label %if.end31
}

define %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE19rotateLeftThenRightEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n) {
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n, i32 0, i32 3
  %left1 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left1, align 8
  %assert.condition = icmp eq %"AVLnode<int, int>"* %left.load, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @56, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %1 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE10rotateLeftEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %left.load)
  store %"AVLnode<int, int>"* %1, %"AVLnode<int, int>"** %left, align 8
  %2 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE11rotateRightEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n)
  ret %"AVLnode<int, int>"* %2
}

define %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE10rotateLeftEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %node) {
  %a = alloca %"AVLnode<int, int>"*, align 8
  %b = alloca %"AVLnode<int, int>"*, align 8
  store %"AVLnode<int, int>"* %node, %"AVLnode<int, int>"** %a, align 8
  %a.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right, align 8
  %assert.condition = icmp eq %"AVLnode<int, int>"* %right.load, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @57, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  store %"AVLnode<int, int>"* %right.load, %"AVLnode<int, int>"** %b, align 8
  %b.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %parent = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load, i32 0, i32 2
  %a.load1 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %parent2 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load1, i32 0, i32 2
  %parent.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent2, align 8
  store %"AVLnode<int, int>"* %parent.load, %"AVLnode<int, int>"** %parent, align 8
  %a.load3 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %right4 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load3, i32 0, i32 4
  %b.load5 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %left = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load5, i32 0, i32 3
  %left.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %left, align 8
  store %"AVLnode<int, int>"* %left.load, %"AVLnode<int, int>"** %right4, align 8
  %a.load6 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %right7 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load6, i32 0, i32 4
  %right.load8 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right7, align 8
  %1 = icmp ne %"AVLnode<int, int>"* %right.load8, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %assert.success
  %a.load9 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %right10 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load9, i32 0, i32 4
  %right.load11 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right10, align 8
  %parent12 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %right.load11, i32 0, i32 2
  %a.load13 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  store %"AVLnode<int, int>"* %a.load13, %"AVLnode<int, int>"** %parent12, align 8
  br label %if.end

if.else:                                          ; preds = %assert.success
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %b.load14 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %left15 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load14, i32 0, i32 3
  %a.load16 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  store %"AVLnode<int, int>"* %a.load16, %"AVLnode<int, int>"** %left15, align 8
  %a.load17 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %parent18 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %a.load17, i32 0, i32 2
  %b.load19 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  store %"AVLnode<int, int>"* %b.load19, %"AVLnode<int, int>"** %parent18, align 8
  %b.load20 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %parent21 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load20, i32 0, i32 2
  %parent.load22 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent21, align 8
  %2 = icmp ne %"AVLnode<int, int>"* %parent.load22, null
  br i1 %2, label %if.then23, label %if.else30

if.then23:                                        ; preds = %if.end
  %b.load24 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %parent25 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load24, i32 0, i32 2
  %parent.load26 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent25, align 8
  %right27 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load26, i32 0, i32 4
  %right.load28 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right27, align 8
  %a.load29 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  %3 = icmp eq %"AVLnode<int, int>"* %right.load28, %a.load29
  br i1 %3, label %if.then35, label %if.else41

if.else30:                                        ; preds = %if.end
  br label %if.end31

if.end31:                                         ; preds = %if.end47, %if.else30
  %a.load32 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %a, align 8
  call void @_EN3std10OrderedMapI3int3intE10setBalanceEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %a.load32)
  %b.load33 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  call void @_EN3std10OrderedMapI3int3intE10setBalanceEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %b.load33)
  %b.load34 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  ret %"AVLnode<int, int>"* %b.load34

if.then35:                                        ; preds = %if.then23
  %b.load36 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %parent37 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load36, i32 0, i32 2
  %parent.load38 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent37, align 8
  %right39 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load38, i32 0, i32 4
  %b.load40 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  store %"AVLnode<int, int>"* %b.load40, %"AVLnode<int, int>"** %right39, align 8
  br label %if.end47

if.else41:                                        ; preds = %if.then23
  %b.load42 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  %parent43 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %b.load42, i32 0, i32 2
  %parent.load44 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %parent43, align 8
  %left45 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %parent.load44, i32 0, i32 3
  %b.load46 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %b, align 8
  store %"AVLnode<int, int>"* %b.load46, %"AVLnode<int, int>"** %left45, align 8
  br label %if.end47

if.end47:                                         ; preds = %if.else41, %if.then35
  br label %if.end31
}

define %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE19rotateRightThenLeftEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n) {
  %right = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n, i32 0, i32 4
  %right1 = getelementptr inbounds %"AVLnode<int, int>", %"AVLnode<int, int>"* %n, i32 0, i32 4
  %right.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %right1, align 8
  %assert.condition = icmp eq %"AVLnode<int, int>"* %right.load, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @58, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %1 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE11rotateRightEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %right.load)
  store %"AVLnode<int, int>"* %1, %"AVLnode<int, int>"** %right, align 8
  %2 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE10rotateLeftEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %n)
  ret %"AVLnode<int, int>"* %2
}

declare void @free(i8*)

define %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE9firstNodeE(%"OrderedMap<int, int>"* %this) {
  %root = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root, align 8
  %1 = icmp eq %"AVLnode<int, int>"* %root.load, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret %"AVLnode<int, int>"* null

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %root1 = getelementptr inbounds %"OrderedMap<int, int>", %"OrderedMap<int, int>"* %this, i32 0, i32 0
  %root.load2 = load %"AVLnode<int, int>"*, %"AVLnode<int, int>"** %root1, align 8
  %__implicit_unwrap.condition = icmp eq %"AVLnode<int, int>"* %root.load2, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %if.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @59, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end
  %2 = call %"AVLnode<int, int>"* @_EN3std10OrderedMapI3int3intE12minInSubtreeEP7AVLnodeI3int3intE(%"OrderedMap<int, int>"* %this, %"AVLnode<int, int>"* %root.load2)
  ret %"AVLnode<int, int>"* %2
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
