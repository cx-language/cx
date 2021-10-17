
%"Set<string>" = type { %"Map<string, bool>" }
%"Map<string, bool>" = type { %"List<List<MapEntry<string, bool>>>", i32 }
%"List<List<MapEntry<string, bool>>>" = type { %"List<MapEntry<string, bool>>"*, i32, i32 }
%"List<MapEntry<string, bool>>" = type { %"MapEntry<string, bool>"*, i32, i32 }
%"MapEntry<string, bool>" = type { %string, i1 }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }
%"SetIterator<string>" = type { %"MapIterator<string, bool>" }
%"MapIterator<string, bool>" = type { %"ArrayIterator<List<MapEntry<string, bool>>>", %"MapEntry<string, bool>"* }
%"ArrayIterator<List<MapEntry<string, bool>>>" = type { %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"* }
%"ArrayIterator<MapEntry<string, bool>>" = type { %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"* }
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }
%"ArrayRef<MapEntry<string, bool>>" = type { %"MapEntry<string, bool>"*, i32 }
%never = type {}
%"ArrayRef<List<MapEntry<string, bool>>>" = type { %"List<MapEntry<string, bool>>"*, i32 }
%OutputFileStream = type { %FILE* }
%FILE = type {}

@0 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@2 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:18:5\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"lol\00", align 1
@4 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:19:5\0A\00", align 1
@5 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@6 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@7 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:27:5\0A\00", align 1
@8 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@9 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@10 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:30:5\0A\00", align 1
@11 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@12 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@13 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@14 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:38:5\0A\00", align 1
@15 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@16 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@17 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:41:5\0A\00", align 1
@18 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@19 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@20 = private unnamed_addr constant [3 x i8] c"bb\00", align 1
@21 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@22 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@23 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@24 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@25 = private unnamed_addr constant [3 x i8] c"bb\00", align 1
@26 = private unnamed_addr constant [40 x i8] c"Assertion failed at set-tests.cx:60:13\0A\00", align 1
@27 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@28 = private unnamed_addr constant [40 x i8] c"Assertion failed at set-tests.cx:63:13\0A\00", align 1
@29 = private unnamed_addr constant [40 x i8] c"Assertion failed at set-tests.cx:66:13\0A\00", align 1
@30 = private unnamed_addr constant [40 x i8] c"Assertion failed at set-tests.cx:69:13\0A\00", align 1
@31 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:73:5\0A\00", align 1
@32 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:74:5\0A\00", align 1
@33 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:75:5\0A\00", align 1
@34 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@35 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@36 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@37 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:85:9\0A\00", align 1
@38 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@39 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@40 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@41 = private unnamed_addr constant [39 x i8] c"Assertion failed at set-tests.cx:97:9\0A\00", align 1
@42 = private unnamed_addr constant [40 x i8] c"Assertion failed at set-tests.cx:100:5\0A\00", align 1
@43 = private unnamed_addr constant [12 x i8] c"List index \00", align 1
@44 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@45 = private unnamed_addr constant [12 x i8] c"List index \00", align 1
@46 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@47 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1
@48 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1

define i32 @main() {
  call void @_EN4main10testInsertE()
  call void @_EN4main11testRemovalE()
  call void @_EN4main14testDuplicatesE()
  call void @_EN4main12testIteratorE()
  call void @_EN4main20testEmptySetIteratorE()
  call void @_EN4main19testUnitSetIteratorE()
  ret i32 0
}

define void @_EN4main10testInsertE() {
  %s = alloca %"Set<string>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  call void @_EN3std3SetI6stringE4initE(%"Set<string>"* %s)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i32 2)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i32 2)
  %1 = call i1 @_EN3std3SetI6stringE8containsEP6string(%"Set<string>"* %s, %string* %__str1)
  %assert.condition = icmp eq i1 %1, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @2, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @3, i32 0, i32 0), i32 3)
  %2 = call i1 @_EN3std3SetI6stringE8containsEP6string(%"Set<string>"* %s, %string* %__str2)
  %3 = xor i1 %2, true
  %assert.condition3 = icmp eq i1 %3, false
  br i1 %assert.condition3, label %assert.fail4, label %assert.success5

assert.fail4:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @4, i32 0, i32 0))
  unreachable

assert.success5:                                  ; preds = %assert.success
  ret void
}

define void @_EN4main11testRemovalE() {
  %s = alloca %"Set<string>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  call void @_EN3std3SetI6stringE4initE(%"Set<string>"* %s)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0), i32 2)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0), i32 2)
  %1 = call i1 @_EN3std3SetI6stringE8containsEP6string(%"Set<string>"* %s, %string* %__str1)
  %assert.condition = icmp eq i1 %1, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @7, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @8, i32 0, i32 0), i32 2)
  call void @_EN3std3SetI6stringE6removeEP6string(%"Set<string>"* %s, %string* %__str2)
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0), i32 2)
  %2 = call i1 @_EN3std3SetI6stringE8containsEP6string(%"Set<string>"* %s, %string* %__str3)
  %3 = xor i1 %2, true
  %assert.condition4 = icmp eq i1 %3, false
  br i1 %assert.condition4, label %assert.fail5, label %assert.success6

assert.fail5:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @10, i32 0, i32 0))
  unreachable

assert.success6:                                  ; preds = %assert.success
  ret void
}

define void @_EN4main14testDuplicatesE() {
  %s = alloca %"Set<string>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  call void @_EN3std3SetI6stringE4initE(%"Set<string>"* %s)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @11, i32 0, i32 0), i32 2)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @12, i32 0, i32 0), i32 2)
  %__str.load5 = load %string, %string* %__str1, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load5)
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @13, i32 0, i32 0), i32 2)
  %1 = call i1 @_EN3std3SetI6stringE8containsEP6string(%"Set<string>"* %s, %string* %__str2)
  %assert.condition = icmp eq i1 %1, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @14, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @15, i32 0, i32 0), i32 2)
  call void @_EN3std3SetI6stringE6removeEP6string(%"Set<string>"* %s, %string* %__str3)
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @16, i32 0, i32 0), i32 2)
  %2 = call i1 @_EN3std3SetI6stringE8containsEP6string(%"Set<string>"* %s, %string* %__str4)
  %3 = xor i1 %2, true
  %assert.condition6 = icmp eq i1 %3, false
  br i1 %assert.condition6, label %assert.fail7, label %assert.success8

assert.fail7:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @17, i32 0, i32 0))
  unreachable

assert.success8:                                  ; preds = %assert.success
  ret void
}

define void @_EN4main12testIteratorE() {
  %s = alloca %"Set<string>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %__str5 = alloca %string, align 8
  %a = alloca i1, align 1
  %bb = alloca i1, align 1
  %ccc = alloca i1, align 1
  %__iterator = alloca %"SetIterator<string>", align 8
  %e = alloca %string*, align 8
  %__str6 = alloca %string, align 8
  %__str7 = alloca %string, align 8
  %__str8 = alloca %string, align 8
  call void @_EN3std3SetI6stringE4initE(%"Set<string>"* %s)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @18, i32 0, i32 0), i32 1)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @19, i32 0, i32 0), i32 3)
  %__str.load9 = load %string, %string* %__str1, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load9)
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @20, i32 0, i32 0), i32 2)
  %__str.load10 = load %string, %string* %__str2, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load10)
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @21, i32 0, i32 0), i32 1)
  %__str.load11 = load %string, %string* %__str3, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load11)
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @22, i32 0, i32 0), i32 3)
  %__str.load12 = load %string, %string* %__str4, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load12)
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @23, i32 0, i32 0), i32 3)
  %__str.load13 = load %string, %string* %__str5, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load13)
  store i1 false, i1* %a, align 1
  store i1 false, i1* %bb, align 1
  store i1 false, i1* %ccc, align 1
  %1 = call %"SetIterator<string>" @_EN3std3SetI6stringE8iteratorE(%"Set<string>"* %s)
  store %"SetIterator<string>" %1, %"SetIterator<string>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std11SetIteratorI6stringE8hasValueE(%"SetIterator<string>"* %__iterator)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call %string* @_EN3std11SetIteratorI6stringE5valueE(%"SetIterator<string>"* %__iterator)
  store %string* %3, %string** %e, align 8
  %e.load = load %string*, %string** %e, align 8
  %e.load.load = load %string, %string* %e.load, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str6, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @24, i32 0, i32 0), i32 1)
  %__str.load14 = load %string, %string* %__str6, align 8
  %4 = call i1 @_EN3stdeqE6string6string(%string %e.load.load, %string %__str.load14)
  br i1 %4, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std11SetIteratorI6stringE9incrementE(%"SetIterator<string>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %a.load = load i1, i1* %a, align 1
  %assert.condition = icmp eq i1 %a.load, false
  br i1 %assert.condition, label %assert.fail37, label %assert.success38

if.then:                                          ; preds = %loop.body
  %a.load15 = load i1, i1* %a, align 1
  %5 = xor i1 %a.load15, true
  %assert.condition16 = icmp eq i1 %5, false
  br i1 %assert.condition16, label %assert.fail, label %assert.success

if.else:                                          ; preds = %loop.body
  %e.load17 = load %string*, %string** %e, align 8
  %e.load.load18 = load %string, %string* %e.load17, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str7, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @25, i32 0, i32 0), i32 2)
  %__str.load19 = load %string, %string* %__str7, align 8
  %6 = call i1 @_EN3stdeqE6string6string(%string %e.load.load18, %string %__str.load19)
  br i1 %6, label %if.then20, label %if.else22

if.end:                                           ; preds = %if.end26, %assert.success
  br label %loop.increment

assert.fail:                                      ; preds = %if.then
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @26, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %if.then
  store i1 true, i1* %a, align 1
  br label %if.end

if.then20:                                        ; preds = %if.else
  %bb.load = load i1, i1* %bb, align 1
  %7 = xor i1 %bb.load, true
  %assert.condition21 = icmp eq i1 %7, false
  br i1 %assert.condition21, label %assert.fail27, label %assert.success28

if.else22:                                        ; preds = %if.else
  %e.load23 = load %string*, %string** %e, align 8
  %e.load.load24 = load %string, %string* %e.load23, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str8, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @27, i32 0, i32 0), i32 3)
  %__str.load25 = load %string, %string* %__str8, align 8
  %8 = call i1 @_EN3stdeqE6string6string(%string %e.load.load24, %string %__str.load25)
  br i1 %8, label %if.then29, label %if.else31

if.end26:                                         ; preds = %if.end32, %assert.success28
  br label %if.end

assert.fail27:                                    ; preds = %if.then20
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @28, i32 0, i32 0))
  unreachable

assert.success28:                                 ; preds = %if.then20
  store i1 true, i1* %bb, align 1
  br label %if.end26

if.then29:                                        ; preds = %if.else22
  %ccc.load = load i1, i1* %ccc, align 1
  %9 = xor i1 %ccc.load, true
  %assert.condition30 = icmp eq i1 %9, false
  br i1 %assert.condition30, label %assert.fail33, label %assert.success34

if.else31:                                        ; preds = %if.else22
  br i1 true, label %assert.fail35, label %assert.success36

if.end32:                                         ; preds = %assert.success36, %assert.success34
  br label %if.end26

assert.fail33:                                    ; preds = %if.then29
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @29, i32 0, i32 0))
  unreachable

assert.success34:                                 ; preds = %if.then29
  store i1 true, i1* %ccc, align 1
  br label %if.end32

assert.fail35:                                    ; preds = %if.else31
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @30, i32 0, i32 0))
  unreachable

assert.success36:                                 ; preds = %if.else31
  br label %if.end32

assert.fail37:                                    ; preds = %loop.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @31, i32 0, i32 0))
  unreachable

assert.success38:                                 ; preds = %loop.end
  %bb.load39 = load i1, i1* %bb, align 1
  %assert.condition40 = icmp eq i1 %bb.load39, false
  br i1 %assert.condition40, label %assert.fail41, label %assert.success42

assert.fail41:                                    ; preds = %assert.success38
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @32, i32 0, i32 0))
  unreachable

assert.success42:                                 ; preds = %assert.success38
  %ccc.load43 = load i1, i1* %ccc, align 1
  %assert.condition44 = icmp eq i1 %ccc.load43, false
  br i1 %assert.condition44, label %assert.fail45, label %assert.success46

assert.fail45:                                    ; preds = %assert.success42
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @33, i32 0, i32 0))
  unreachable

assert.success46:                                 ; preds = %assert.success42
  ret void
}

define void @_EN4main20testEmptySetIteratorE() {
  %s = alloca %"Set<string>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__iterator = alloca %"SetIterator<string>", align 8
  %e = alloca %string*, align 8
  call void @_EN3std3SetI6stringE4initE(%"Set<string>"* %s)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @34, i32 0, i32 0), i32 3)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @35, i32 0, i32 0), i32 3)
  %__str.load3 = load %string, %string* %__str1, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load3)
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @36, i32 0, i32 0), i32 3)
  call void @_EN3std3SetI6stringE6removeEP6string(%"Set<string>"* %s, %string* %__str2)
  %1 = call %"SetIterator<string>" @_EN3std3SetI6stringE8iteratorE(%"Set<string>"* %s)
  store %"SetIterator<string>" %1, %"SetIterator<string>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std11SetIteratorI6stringE8hasValueE(%"SetIterator<string>"* %__iterator)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call %string* @_EN3std11SetIteratorI6stringE5valueE(%"SetIterator<string>"* %__iterator)
  store %string* %3, %string** %e, align 8
  br i1 true, label %assert.fail, label %assert.success

loop.increment:                                   ; preds = %assert.success
  call void @_EN3std11SetIteratorI6stringE9incrementE(%"SetIterator<string>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret void

assert.fail:                                      ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @37, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.body
  br label %loop.increment
}

define void @_EN4main19testUnitSetIteratorE() {
  %s = alloca %"Set<string>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %count = alloca i32, align 4
  %__iterator = alloca %"SetIterator<string>", align 8
  %e = alloca %string*, align 8
  %__str2 = alloca %string, align 8
  call void @_EN3std3SetI6stringE4initE(%"Set<string>"* %s)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @38, i32 0, i32 0), i32 3)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @39, i32 0, i32 0), i32 3)
  %__str.load3 = load %string, %string* %__str1, align 8
  call void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %s, %string %__str.load3)
  store i32 0, i32* %count, align 4
  %1 = call %"SetIterator<string>" @_EN3std3SetI6stringE8iteratorE(%"Set<string>"* %s)
  store %"SetIterator<string>" %1, %"SetIterator<string>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std11SetIteratorI6stringE8hasValueE(%"SetIterator<string>"* %__iterator)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call %string* @_EN3std11SetIteratorI6stringE5valueE(%"SetIterator<string>"* %__iterator)
  store %string* %3, %string** %e, align 8
  %count.load = load i32, i32* %count, align 4
  %4 = add i32 %count.load, 1
  store i32 %4, i32* %count, align 4
  %e.load = load %string*, %string** %e, align 8
  %e.load.load = load %string, %string* %e.load, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @40, i32 0, i32 0), i32 3)
  %__str.load4 = load %string, %string* %__str2, align 8
  %5 = call i1 @_EN3stdeqE6string6string(%string %e.load.load, %string %__str.load4)
  %assert.condition = icmp eq i1 %5, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

loop.increment:                                   ; preds = %assert.success
  call void @_EN3std11SetIteratorI6stringE9incrementE(%"SetIterator<string>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %count.load5 = load i32, i32* %count, align 4
  %6 = icmp eq i32 %count.load5, 1
  %assert.condition6 = icmp eq i1 %6, false
  br i1 %assert.condition6, label %assert.fail7, label %assert.success8

assert.fail:                                      ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @41, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.body
  br label %loop.increment

assert.fail7:                                     ; preds = %loop.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @42, i32 0, i32 0))
  unreachable

assert.success8:                                  ; preds = %loop.end
  ret void
}

define void @_EN3std3SetI6stringE4initE(%"Set<string>"* %this) {
  %1 = alloca %"Map<string, bool>", align 8
  %map = getelementptr inbounds %"Set<string>", %"Set<string>"* %this, i32 0, i32 0
  call void @_EN3std3MapI6string4boolE4initE(%"Map<string, bool>"* %1)
  %.load = load %"Map<string, bool>", %"Map<string, bool>"* %1, align 8
  store %"Map<string, bool>" %.load, %"Map<string, bool>"* %map, align 8
  ret void
}

declare void @_EN3std6string4initEP4char3int(%string*, i8*, i32)

define void @_EN3std3SetI6stringE6insertE6string(%"Set<string>"* %this, %string %key) {
  %map = getelementptr inbounds %"Set<string>", %"Set<string>"* %this, i32 0, i32 0
  call void @_EN3std3MapI6string4boolE6insertE6string4bool(%"Map<string, bool>"* %map, %string %key, i1 false)
  ret void
}

define i1 @_EN3std3SetI6stringE8containsEP6string(%"Set<string>"* %this, %string* %key) {
  %map = getelementptr inbounds %"Set<string>", %"Set<string>"* %this, i32 0, i32 0
  %1 = call i1 @_EN3std3MapI6string4boolE8containsEP6string(%"Map<string, bool>"* %map, %string* %key)
  ret i1 %1
}

declare void @_EN3std10assertFailEP4char(i8*)

define void @_EN3std3SetI6stringE6removeEP6string(%"Set<string>"* %this, %string* %key) {
  %map = getelementptr inbounds %"Set<string>", %"Set<string>"* %this, i32 0, i32 0
  call void @_EN3std3MapI6string4boolE6removeEP6string(%"Map<string, bool>"* %map, %string* %key)
  ret void
}

define %"SetIterator<string>" @_EN3std3SetI6stringE8iteratorE(%"Set<string>"* %this) {
  %1 = alloca %"SetIterator<string>", align 8
  call void @_EN3std11SetIteratorI6stringE4initEP3SetI6stringE(%"SetIterator<string>"* %1, %"Set<string>"* %this)
  %.load = load %"SetIterator<string>", %"SetIterator<string>"* %1, align 8
  ret %"SetIterator<string>" %.load
}

define i1 @_EN3std11SetIteratorI6stringE8hasValueE(%"SetIterator<string>"* %this) {
  %iterator = getelementptr inbounds %"SetIterator<string>", %"SetIterator<string>"* %this, i32 0, i32 0
  %1 = call i1 @_EN3std11MapIteratorI6string4boolE8hasValueE(%"MapIterator<string, bool>"* %iterator)
  ret i1 %1
}

define %string* @_EN3std11SetIteratorI6stringE5valueE(%"SetIterator<string>"* %this) {
  %iterator = getelementptr inbounds %"SetIterator<string>", %"SetIterator<string>"* %this, i32 0, i32 0
  %1 = call %"MapEntry<string, bool>"* @_EN3std11MapIteratorI6string4boolE5valueE(%"MapIterator<string, bool>"* %iterator)
  %key = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %1, i32 0, i32 0
  ret %string* %key
}

declare i1 @_EN3stdeqE6string6string(%string, %string)

define void @_EN3std11SetIteratorI6stringE9incrementE(%"SetIterator<string>"* %this) {
  %iterator = getelementptr inbounds %"SetIterator<string>", %"SetIterator<string>"* %this, i32 0, i32 0
  call void @_EN3std11MapIteratorI6string4boolE9incrementE(%"MapIterator<string, bool>"* %iterator)
  ret void
}

define void @_EN3std3MapI6string4boolE4initE(%"Map<string, bool>"* %this) {
  %1 = alloca %"List<List<MapEntry<string, bool>>>", align 8
  %size = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %hashTable = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4initE(%"List<List<MapEntry<string, bool>>>"* %1)
  %.load = load %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %1, align 8
  store %"List<List<MapEntry<string, bool>>>" %.load, %"List<List<MapEntry<string, bool>>>"* %hashTable, align 8
  %hashTable1 = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 0
  call void @_EN3std3MapI6string4boolE17increaseTableSizeEP4ListI4ListI8MapEntryI6string4boolEEE3int(%"Map<string, bool>"* %this, %"List<List<MapEntry<string, bool>>>"* %hashTable1, i32 128)
  ret void
}

define void @_EN3std3MapI6string4boolE6insertE6string4bool(%"Map<string, bool>"* %this, %string %key, i1 %value) {
  %1 = alloca %string, align 8
  %hashValue = alloca i32, align 4
  %2 = alloca %string, align 8
  %3 = alloca %"MapEntry<string, bool>", align 8
  store %string %key, %string* %1, align 8
  %4 = call i1 @_EN3std3MapI6string4boolE8containsEP6string(%"Map<string, bool>"* %this, %string* %1)
  br i1 %4, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  store %string %key, %string* %2, align 8
  %5 = call i64 @_EN3std6string4hashE(%string* %2)
  %6 = call i32 @_EN3std11convertHashE6uint64(i64 %5)
  %7 = call i32 @_EN3std3MapI6string4boolE8capacityE(%"Map<string, bool>"* %this)
  %8 = srem i32 %6, %7
  store i32 %8, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %9 = call %"List<MapEntry<string, bool>>"* @_EN3std4ListI4ListI8MapEntryI6string4boolEEEixE3int(%"List<List<MapEntry<string, bool>>>"* %hashTable, i32 %hashValue.load)
  call void @_EN3std8MapEntryI6string4boolE4initE6string4bool(%"MapEntry<string, bool>"* %3, %string %key, i1 %value)
  %.load = load %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %3, align 8
  call void @_EN3std4ListI8MapEntryI6string4boolEE4pushE8MapEntryI6string4boolE(%"List<MapEntry<string, bool>>"* %9, %"MapEntry<string, bool>" %.load)
  %size = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %10 = add i32 %size.load, 1
  store i32 %10, i32* %size, align 4
  %11 = call double @_EN3std3MapI6string4boolE10loadFactorE(%"Map<string, bool>"* %this)
  %12 = fcmp ogt double %11, 0x3FE51EB85FE0EECB
  br i1 %12, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.end
  call void @_EN3std3MapI6string4boolE6resizeE(%"Map<string, bool>"* %this)
  br label %if.end3

if.else2:                                         ; preds = %if.end
  br label %if.end3

if.end3:                                          ; preds = %if.else2, %if.then1
  ret void
}

define i1 @_EN3std3MapI6string4boolE8containsEP6string(%"Map<string, bool>"* %this, %string* %e) {
  %hashValue = alloca i32, align 4
  %slot = alloca %"List<MapEntry<string, bool>>"*, align 8
  %__iterator = alloca %"ArrayIterator<MapEntry<string, bool>>", align 8
  %element = alloca %"MapEntry<string, bool>"*, align 8
  %1 = call i64 @_EN3std6string4hashE(%string* %e)
  %2 = call i32 @_EN3std11convertHashE6uint64(i64 %1)
  %3 = call i32 @_EN3std3MapI6string4boolE8capacityE(%"Map<string, bool>"* %this)
  %4 = srem i32 %2, %3
  store i32 %4, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %5 = call %"List<MapEntry<string, bool>>"* @_EN3std4ListI4ListI8MapEntryI6string4boolEEEixE3int(%"List<List<MapEntry<string, bool>>>"* %hashTable, i32 %hashValue.load)
  store %"List<MapEntry<string, bool>>"* %5, %"List<MapEntry<string, bool>>"** %slot, align 8
  %slot.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %slot, align 8
  %6 = call i32 @_EN3std4ListI8MapEntryI6string4boolEE4sizeE(%"List<MapEntry<string, bool>>"* %slot.load)
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %slot.load1 = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %slot, align 8
  %8 = call %"ArrayIterator<MapEntry<string, bool>>" @_EN3std4ListI8MapEntryI6string4boolEE8iteratorE(%"List<MapEntry<string, bool>>"* %slot.load1)
  store %"ArrayIterator<MapEntry<string, bool>>" %8, %"ArrayIterator<MapEntry<string, bool>>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %if.end
  %9 = call i1 @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE8hasValueE(%"ArrayIterator<MapEntry<string, bool>>"* %__iterator)
  br i1 %9, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %10 = call %"MapEntry<string, bool>"* @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE5valueE(%"ArrayIterator<MapEntry<string, bool>>"* %__iterator)
  store %"MapEntry<string, bool>"* %10, %"MapEntry<string, bool>"** %element, align 8
  %element.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %element, align 8
  %key = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %element.load, i32 0, i32 0
  %key.load = load %string, %string* %key, align 8
  %e.load = load %string, %string* %e, align 8
  %11 = call i1 @_EN3stdeqE6string6string(%string %key.load, %string %e.load)
  br i1 %11, label %if.then2, label %if.else3

loop.increment:                                   ; preds = %if.end4
  call void @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE9incrementE(%"ArrayIterator<MapEntry<string, bool>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i1 false

if.then2:                                         ; preds = %loop.body
  ret i1 true

if.else3:                                         ; preds = %loop.body
  br label %if.end4

if.end4:                                          ; preds = %if.else3
  br label %loop.increment
}

define void @_EN3std3MapI6string4boolE6removeEP6string(%"Map<string, bool>"* %this, %string* %e) {
  %hashValue = alloca i32, align 4
  %slot = alloca %"List<MapEntry<string, bool>>"*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %2 = call i64 @_EN3std6string4hashE(%string* %e)
  %3 = call i32 @_EN3std11convertHashE6uint64(i64 %2)
  %4 = call i32 @_EN3std3MapI6string4boolE8capacityE(%"Map<string, bool>"* %this)
  %5 = srem i32 %3, %4
  store i32 %5, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %6 = call %"List<MapEntry<string, bool>>"* @_EN3std4ListI4ListI8MapEntryI6string4boolEEEixE3int(%"List<List<MapEntry<string, bool>>>"* %hashTable, i32 %hashValue.load)
  store %"List<MapEntry<string, bool>>"* %6, %"List<MapEntry<string, bool>>"** %slot, align 8
  %slot.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %slot, align 8
  %7 = call i32 @_EN3std4ListI8MapEntryI6string4boolEE4sizeE(%"List<MapEntry<string, bool>>"* %slot.load)
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %slot.load1 = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %slot, align 8
  %9 = call i32 @_EN3std4ListI8MapEntryI6string4boolEE4sizeE(%"List<MapEntry<string, bool>>"* %slot.load1)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 %9)
  %10 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %10, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %if.end
  %11 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %11, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %12 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %12, i32* %i, align 4
  %slot.load2 = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %slot, align 8
  %i.load = load i32, i32* %i, align 4
  %13 = call %"MapEntry<string, bool>"* @_EN3std4ListI8MapEntryI6string4boolEEixE3int(%"List<MapEntry<string, bool>>"* %slot.load2, i32 %i.load)
  %key = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %13, i32 0, i32 0
  %key.load = load %string, %string* %key, align 8
  %e.load = load %string, %string* %e, align 8
  %14 = call i1 @_EN3stdeqE6string6string(%string %key.load, %string %e.load)
  br i1 %14, label %if.then3, label %if.else6

loop.increment:                                   ; preds = %if.end7
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret void

if.then3:                                         ; preds = %loop.body
  %slot.load4 = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %slot, align 8
  %i.load5 = load i32, i32* %i, align 4
  call void @_EN3std4ListI8MapEntryI6string4boolEE8removeAtE3int(%"List<MapEntry<string, bool>>"* %slot.load4, i32 %i.load5)
  %size = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %15 = add i32 %size.load, -1
  store i32 %15, i32* %size, align 4
  ret void

if.else6:                                         ; preds = %loop.body
  br label %if.end7

if.end7:                                          ; preds = %if.else6
  br label %loop.increment
}

define void @_EN3std11SetIteratorI6stringE4initEP3SetI6stringE(%"SetIterator<string>"* %this, %"Set<string>"* %set) {
  %1 = alloca %"MapIterator<string, bool>", align 8
  %iterator = getelementptr inbounds %"SetIterator<string>", %"SetIterator<string>"* %this, i32 0, i32 0
  %map = getelementptr inbounds %"Set<string>", %"Set<string>"* %set, i32 0, i32 0
  call void @_EN3std11MapIteratorI6string4boolE4initEP3MapI6string4boolE(%"MapIterator<string, bool>"* %1, %"Map<string, bool>"* %map)
  %.load = load %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %1, align 8
  store %"MapIterator<string, bool>" %.load, %"MapIterator<string, bool>"* %iterator, align 8
  ret void
}

define i1 @_EN3std11MapIteratorI6string4boolE8hasValueE(%"MapIterator<string, bool>"* %this) {
  %hashTableIterator = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 0
  %1 = call i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE8hasValueE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %hashTableIterator)
  ret i1 %1
}

define %"MapEntry<string, bool>"* @_EN3std11MapIteratorI6string4boolE5valueE(%"MapIterator<string, bool>"* %this) {
  %current = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 1
  %current.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %current, align 8
  ret %"MapEntry<string, bool>"* %current.load
}

define void @_EN3std11MapIteratorI6string4boolE9incrementE(%"MapIterator<string, bool>"* %this) {
  %slot = alloca %"List<MapEntry<string, bool>>"*, align 8
  %slotEnd = alloca %"MapEntry<string, bool>"*, align 8
  %hashTableIterator = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 0
  %1 = call %"List<MapEntry<string, bool>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE5valueE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %hashTableIterator)
  store %"List<MapEntry<string, bool>>"* %1, %"List<MapEntry<string, bool>>"** %slot, align 8
  %slot.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %slot, align 8
  %2 = call %"MapEntry<string, bool>"* @_EN3std4ListI8MapEntryI6string4boolEE4dataE(%"List<MapEntry<string, bool>>"* %slot.load)
  %slot.load1 = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %slot, align 8
  %3 = call i32 @_EN3std4ListI8MapEntryI6string4boolEE4sizeE(%"List<MapEntry<string, bool>>"* %slot.load1)
  %4 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %2, i32 %3
  store %"MapEntry<string, bool>"* %4, %"MapEntry<string, bool>"** %slotEnd, align 8
  %current = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 1
  %current.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %current, align 8
  %5 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %current.load, i32 1
  store %"MapEntry<string, bool>"* %5, %"MapEntry<string, bool>"** %current, align 8
  %current2 = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 1
  %current.load3 = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %current2, align 8
  %6 = bitcast %"MapEntry<string, bool>"* %current.load3 to i8*
  %slotEnd.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %slotEnd, align 8
  %7 = bitcast %"MapEntry<string, bool>"* %slotEnd.load to i8*
  %8 = icmp eq i8* %6, %7
  br i1 %8, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %hashTableIterator4 = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 0
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE9incrementE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %hashTableIterator4)
  call void @_EN3std11MapIteratorI6string4boolE14skipEmptySlotsE(%"MapIterator<string, bool>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4initE(%"List<List<MapEntry<string, bool>>>"* %this) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

define void @_EN3std3MapI6string4boolE17increaseTableSizeEP4ListI4ListI8MapEntryI6string4boolEEE3int(%"Map<string, bool>"* %this, %"List<List<MapEntry<string, bool>>>"* %newTable, i32 %newCapacity) {
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %2 = alloca %"List<MapEntry<string, bool>>", align 8
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 %newCapacity)
  %3 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %3, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %4 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %5 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %5, i32* %i, align 4
  call void @_EN3std4ListI8MapEntryI6string4boolEE4initE(%"List<MapEntry<string, bool>>"* %2)
  %.load = load %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %2, align 8
  call void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4pushE4ListI8MapEntryI6string4boolEE(%"List<List<MapEntry<string, bool>>>"* %newTable, %"List<MapEntry<string, bool>>" %.load)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret void
}

define i64 @_EN3std6string4hashE(%string* %this) {
  %hashValue = alloca i64, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  store i64 5381, i64* %hashValue, align 4
  %characters = getelementptr inbounds %string, %string* %this, i32 0, i32 0
  %2 = call i32 @_EN3std8ArrayRefI4charE4sizeE(%"ArrayRef<char>"* %characters)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 %2)
  %3 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %3, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %4 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %5 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %5, i32* %index, align 4
  %hashValue.load = load i64, i64* %hashValue, align 4
  %6 = shl i64 %hashValue.load, 5
  %hashValue.load1 = load i64, i64* %hashValue, align 4
  %7 = add i64 %6, %hashValue.load1
  %index.load = load i32, i32* %index, align 4
  %8 = call i8 @_EN3std6stringixE3int(%string* %this, i32 %index.load)
  %9 = zext i8 %8 to i64
  %10 = add i64 %7, %9
  store i64 %10, i64* %hashValue, align 4
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %hashValue.load2 = load i64, i64* %hashValue, align 4
  ret i64 %hashValue.load2
}

declare i32 @_EN3std11convertHashE6uint64(i64)

define i32 @_EN3std3MapI6string4boolE8capacityE(%"Map<string, bool>"* %this) {
  %hashTable = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 0
  %1 = call i32 @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4sizeE(%"List<List<MapEntry<string, bool>>>"* %hashTable)
  ret i32 %1
}

define %"List<MapEntry<string, bool>>"* @_EN3std4ListI4ListI8MapEntryI6string4boolEEEixE3int(%"List<List<MapEntry<string, bool>>>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE16indexOutOfBoundsE3int(%"List<List<MapEntry<string, bool>>>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %buffer, align 8
  %2 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %buffer.load, i32 %index
  ret %"List<MapEntry<string, bool>>"* %2
}

define void @_EN3std8MapEntryI6string4boolE4initE6string4bool(%"MapEntry<string, bool>"* %this, %string %key, i1 %value) {
  %key1 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %this, i32 0, i32 0
  store %string %key, %string* %key1, align 8
  %value2 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %this, i32 0, i32 1
  store i1 %value, i1* %value2, align 1
  ret void
}

define void @_EN3std4ListI8MapEntryI6string4boolEE4pushE8MapEntryI6string4boolE(%"List<MapEntry<string, bool>>"* %this, %"MapEntry<string, bool>" %element) {
  %size = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string4boolEE4growE(%"List<MapEntry<string, bool>>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %buffer, align 8
  %size1 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %size.load2 = load i32, i32* %size1, align 4
  %2 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %buffer.load, i32 %size.load2
  store %"MapEntry<string, bool>" %element, %"MapEntry<string, bool>"* %2, align 8
  %size3 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %size.load4 = load i32, i32* %size3, align 4
  %3 = add i32 %size.load4, 1
  store i32 %3, i32* %size3, align 4
  ret void
}

define double @_EN3std3MapI6string4boolE10loadFactorE(%"Map<string, bool>"* %this) {
  %size = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = sitofp i32 %size.load to double
  %2 = call i32 @_EN3std3MapI6string4boolE8capacityE(%"Map<string, bool>"* %this)
  %3 = sitofp i32 %2 to double
  %4 = fdiv double %1, %3
  ret double %4
}

define void @_EN3std3MapI6string4boolE6resizeE(%"Map<string, bool>"* %this) {
  %newTable = alloca %"List<List<MapEntry<string, bool>>>", align 8
  %newCapacity = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %slot = alloca %"List<MapEntry<string, bool>>"*, align 8
  %__iterator1 = alloca %"ArrayIterator<MapEntry<string, bool>>", align 8
  %el = alloca %"MapEntry<string, bool>"*, align 8
  call void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4initE(%"List<List<MapEntry<string, bool>>>"* %newTable)
  %2 = call i32 @_EN3std3MapI6string4boolE8capacityE(%"Map<string, bool>"* %this)
  %3 = mul i32 %2, 2
  store i32 %3, i32* %newCapacity, align 4
  %newCapacity.load = load i32, i32* %newCapacity, align 4
  call void @_EN3std3MapI6string4boolE17increaseTableSizeEP4ListI4ListI8MapEntryI6string4boolEEE3int(%"Map<string, bool>"* %this, %"List<List<MapEntry<string, bool>>>"* %newTable, i32 %newCapacity.load)
  %4 = call i32 @_EN3std3MapI6string4boolE8capacityE(%"Map<string, bool>"* %this)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 %4)
  %5 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %5, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %6 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %6, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %7 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %7, i32* %i, align 4
  %hashTable = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 0
  %i.load = load i32, i32* %i, align 4
  %8 = call %"List<MapEntry<string, bool>>"* @_EN3std4ListI4ListI8MapEntryI6string4boolEEEixE3int(%"List<List<MapEntry<string, bool>>>"* %hashTable, i32 %i.load)
  store %"List<MapEntry<string, bool>>"* %8, %"List<MapEntry<string, bool>>"** %slot, align 8
  %slot.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %slot, align 8
  %9 = call %"ArrayIterator<MapEntry<string, bool>>" @_EN3std4ListI8MapEntryI6string4boolEE8iteratorE(%"List<MapEntry<string, bool>>"* %slot.load)
  store %"ArrayIterator<MapEntry<string, bool>>" %9, %"ArrayIterator<MapEntry<string, bool>>"* %__iterator1, align 8
  br label %loop.condition2

loop.increment:                                   ; preds = %loop.end7
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %hashTable1 = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE6deinitE(%"List<List<MapEntry<string, bool>>>"* %hashTable1)
  %newTable.load = load %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %newTable, align 8
  store %"List<List<MapEntry<string, bool>>>" %newTable.load, %"List<List<MapEntry<string, bool>>>"* %hashTable1, align 8
  ret void

loop.condition2:                                  ; preds = %loop.increment6, %loop.body
  %10 = call i1 @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE8hasValueE(%"ArrayIterator<MapEntry<string, bool>>"* %__iterator1)
  br i1 %10, label %loop.body3, label %loop.end7

loop.body3:                                       ; preds = %loop.condition2
  %11 = call %"MapEntry<string, bool>"* @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE5valueE(%"ArrayIterator<MapEntry<string, bool>>"* %__iterator1)
  store %"MapEntry<string, bool>"* %11, %"MapEntry<string, bool>"** %el, align 8
  %el.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %el, align 8
  %key = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %el.load, i32 0, i32 0
  %12 = call i64 @_EN3std6string4hashE(%string* %key)
  %13 = call i32 @_EN3std11convertHashE6uint64(i64 %12)
  %newCapacity.load4 = load i32, i32* %newCapacity, align 4
  %14 = srem i32 %13, %newCapacity.load4
  %15 = call %"List<MapEntry<string, bool>>"* @_EN3std4ListI4ListI8MapEntryI6string4boolEEEixE3int(%"List<List<MapEntry<string, bool>>>"* %newTable, i32 %14)
  %el.load5 = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %el, align 8
  %el.load.load = load %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %el.load5, align 8
  call void @_EN3std4ListI8MapEntryI6string4boolEE4pushE8MapEntryI6string4boolE(%"List<MapEntry<string, bool>>"* %15, %"MapEntry<string, bool>" %el.load.load)
  br label %loop.increment6

loop.increment6:                                  ; preds = %loop.body3
  call void @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE9incrementE(%"ArrayIterator<MapEntry<string, bool>>"* %__iterator1)
  br label %loop.condition2

loop.end7:                                        ; preds = %loop.condition2
  br label %loop.increment
}

define i32 @_EN3std4ListI8MapEntryI6string4boolEE4sizeE(%"List<MapEntry<string, bool>>"* %this) {
  %size = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %"ArrayIterator<MapEntry<string, bool>>" @_EN3std4ListI8MapEntryI6string4boolEE8iteratorE(%"List<MapEntry<string, bool>>"* %this) {
  %1 = alloca %"ArrayIterator<MapEntry<string, bool>>", align 8
  %2 = alloca %"ArrayRef<MapEntry<string, bool>>", align 8
  call void @_EN3std8ArrayRefI8MapEntryI6string4boolEE4initEP4ListI8MapEntryI6string4boolEE(%"ArrayRef<MapEntry<string, bool>>"* %2, %"List<MapEntry<string, bool>>"* %this)
  %.load = load %"ArrayRef<MapEntry<string, bool>>", %"ArrayRef<MapEntry<string, bool>>"* %2, align 8
  call void @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE4initE8ArrayRefI8MapEntryI6string4boolEE(%"ArrayIterator<MapEntry<string, bool>>"* %1, %"ArrayRef<MapEntry<string, bool>>" %.load)
  %.load1 = load %"ArrayIterator<MapEntry<string, bool>>", %"ArrayIterator<MapEntry<string, bool>>"* %1, align 8
  ret %"ArrayIterator<MapEntry<string, bool>>" %.load1
}

define i1 @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE8hasValueE(%"ArrayIterator<MapEntry<string, bool>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, bool>>", %"ArrayIterator<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<MapEntry<string, bool>>", %"ArrayIterator<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %end.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %end, align 8
  %1 = icmp ne %"MapEntry<string, bool>"* %current.load, %end.load
  ret i1 %1
}

define %"MapEntry<string, bool>"* @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE5valueE(%"ArrayIterator<MapEntry<string, bool>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, bool>>", %"ArrayIterator<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %current, align 8
  ret %"MapEntry<string, bool>"* %current.load
}

define void @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE9incrementE(%"ArrayIterator<MapEntry<string, bool>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, bool>>", %"ArrayIterator<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %current, align 8
  %1 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %current.load, i32 1
  store %"MapEntry<string, bool>"* %1, %"MapEntry<string, bool>"** %current, align 8
  ret void
}

declare void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"*, i32, i32)

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"*)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"*)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"*)

define %"MapEntry<string, bool>"* @_EN3std4ListI8MapEntryI6string4boolEEixE3int(%"List<MapEntry<string, bool>>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string4boolEE16indexOutOfBoundsE3int(%"List<MapEntry<string, bool>>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %buffer, align 8
  %2 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %buffer.load, i32 %index
  ret %"MapEntry<string, bool>"* %2
}

declare void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

define void @_EN3std4ListI8MapEntryI6string4boolEE8removeAtE3int(%"List<MapEntry<string, bool>>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string4boolEE16indexOutOfBoundsE3int(%"List<MapEntry<string, bool>>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @_EN3std4ListI8MapEntryI6string4boolEE14unsafeRemoveAtE3int(%"List<MapEntry<string, bool>>"* %this, i32 %index)
  ret void
}

define void @_EN3std11MapIteratorI6string4boolE4initEP3MapI6string4boolE(%"MapIterator<string, bool>"* %this, %"Map<string, bool>"* %map) {
  %hashTableIterator = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 0
  %hashTable = getelementptr inbounds %"Map<string, bool>", %"Map<string, bool>"* %map, i32 0, i32 0
  %1 = call %"ArrayIterator<List<MapEntry<string, bool>>>" @_EN3std4ListI4ListI8MapEntryI6string4boolEEE8iteratorE(%"List<List<MapEntry<string, bool>>>"* %hashTable)
  store %"ArrayIterator<List<MapEntry<string, bool>>>" %1, %"ArrayIterator<List<MapEntry<string, bool>>>"* %hashTableIterator, align 8
  call void @_EN3std11MapIteratorI6string4boolE14skipEmptySlotsE(%"MapIterator<string, bool>"* %this)
  ret void
}

define i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE8hasValueE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, bool>>>", %"ArrayIterator<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, bool>>>", %"ArrayIterator<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
  %end.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %end, align 8
  %1 = icmp ne %"List<MapEntry<string, bool>>"* %current.load, %end.load
  ret i1 %1
}

define %"List<MapEntry<string, bool>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE5valueE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, bool>>>", %"ArrayIterator<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %current, align 8
  ret %"List<MapEntry<string, bool>>"* %current.load
}

define %"MapEntry<string, bool>"* @_EN3std4ListI8MapEntryI6string4boolEE4dataE(%"List<MapEntry<string, bool>>"* %this) {
  %buffer = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %buffer, align 8
  ret %"MapEntry<string, bool>"* %buffer.load
}

define void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE9incrementE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, bool>>>", %"ArrayIterator<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %current, align 8
  %1 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %current.load, i32 1
  store %"List<MapEntry<string, bool>>"* %1, %"List<MapEntry<string, bool>>"** %current, align 8
  ret void
}

define void @_EN3std11MapIteratorI6string4boolE14skipEmptySlotsE(%"MapIterator<string, bool>"* %this) {
  br label %loop.condition

loop.condition:                                   ; preds = %if.end, %0
  %hashTableIterator = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 0
  %1 = call i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE8hasValueE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %hashTableIterator)
  br i1 %1, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %hashTableIterator1 = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 0
  %2 = call %"List<MapEntry<string, bool>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE5valueE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %hashTableIterator1)
  %3 = call i32 @_EN3std4ListI8MapEntryI6string4boolEE4sizeE(%"List<MapEntry<string, bool>>"* %2)
  %4 = icmp ne i32 %3, 0
  br i1 %4, label %if.then, label %if.else

loop.end:                                         ; preds = %if.then, %loop.condition
  ret void

if.then:                                          ; preds = %loop.body
  %current = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 1
  %hashTableIterator2 = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 0
  %5 = call %"List<MapEntry<string, bool>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE5valueE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %hashTableIterator2)
  %6 = call %"MapEntry<string, bool>"* @_EN3std4ListI8MapEntryI6string4boolEE4dataE(%"List<MapEntry<string, bool>>"* %5)
  store %"MapEntry<string, bool>"* %6, %"MapEntry<string, bool>"** %current, align 8
  br label %loop.end

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  %hashTableIterator3 = getelementptr inbounds %"MapIterator<string, bool>", %"MapIterator<string, bool>"* %this, i32 0, i32 0
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE9incrementE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %hashTableIterator3)
  br label %loop.condition
}

define void @_EN3std4ListI8MapEntryI6string4boolEE4initE(%"List<MapEntry<string, bool>>"* %this) {
  %size = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4pushE4ListI8MapEntryI6string4boolEE(%"List<List<MapEntry<string, bool>>>"* %this, %"List<MapEntry<string, bool>>" %element) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4growE(%"List<List<MapEntry<string, bool>>>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %buffer, align 8
  %size1 = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
  %size.load2 = load i32, i32* %size1, align 4
  %2 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %buffer.load, i32 %size.load2
  store %"List<MapEntry<string, bool>>" %element, %"List<MapEntry<string, bool>>"* %2, align 8
  %size3 = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
  %size.load4 = load i32, i32* %size3, align 4
  %3 = add i32 %size.load4, 1
  store i32 %3, i32* %size3, align 4
  ret void
}

declare i32 @_EN3std8ArrayRefI4charE4sizeE(%"ArrayRef<char>"*)

declare i8 @_EN3std6stringixE3int(%string*, i32)

define i32 @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4sizeE(%"List<List<MapEntry<string, bool>>>"* %this) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std4ListI8MapEntryI6string4boolEE4growE(%"List<MapEntry<string, bool>>"* %this) {
  %capacity = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string4boolEE7reserveE3int(%"List<MapEntry<string, bool>>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 2
  %capacity.load2 = load i32, i32* %capacity1, align 4
  %2 = mul i32 %capacity.load2, 2
  call void @_EN3std4ListI8MapEntryI6string4boolEE7reserveE3int(%"List<MapEntry<string, bool>>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE16indexOutOfBoundsE3int(%"List<List<MapEntry<string, bool>>>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @43, i32 0, i32 0), i32 11)
  store i32 %index, i32* %1, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @44, i32 0, i32 0), i32 27)
  %3 = call i32 @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4sizeE(%"List<List<MapEntry<string, bool>>>"* %this)
  store i32 %3, i32* %2, align 4
  %4 = call %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %__str, i32* %1, %string* %__str1, i32* %2)
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE6deinitE(%"List<List<MapEntry<string, bool>>>"* %this) {
  %__iterator = alloca %"ArrayIterator<List<MapEntry<string, bool>>>", align 8
  %element = alloca %"List<MapEntry<string, bool>>"*, align 8
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<List<MapEntry<string, bool>>>" @_EN3std4ListI4ListI8MapEntryI6string4boolEEE8iteratorE(%"List<List<MapEntry<string, bool>>>"* %this)
  store %"ArrayIterator<List<MapEntry<string, bool>>>" %2, %"ArrayIterator<List<MapEntry<string, bool>>>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE8hasValueE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call %"List<MapEntry<string, bool>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE5valueE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %__iterator)
  store %"List<MapEntry<string, bool>>"* %4, %"List<MapEntry<string, bool>>"** %element, align 8
  %element.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %element, align 8
  call void @_EN3std4ListI8MapEntryI6string4boolEE6deinitE(%"List<MapEntry<string, bool>>"* %element.load)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE9incrementE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %buffer, align 8
  call void @_EN3std10deallocateIAU_4ListI8MapEntryI6string4boolEEEEAU_4ListI8MapEntryI6string4boolEE(%"List<MapEntry<string, bool>>"* %buffer.load)
  br label %if.end
}

define void @_EN3std8ArrayRefI8MapEntryI6string4boolEE4initEP4ListI8MapEntryI6string4boolEE(%"ArrayRef<MapEntry<string, bool>>"* %this, %"List<MapEntry<string, bool>>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<MapEntry<string, bool>>", %"ArrayRef<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %1 = call %"MapEntry<string, bool>"* @_EN3std4ListI8MapEntryI6string4boolEE4dataE(%"List<MapEntry<string, bool>>"* %list)
  store %"MapEntry<string, bool>"* %1, %"MapEntry<string, bool>"** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<MapEntry<string, bool>>", %"ArrayRef<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI8MapEntryI6string4boolEE4sizeE(%"List<MapEntry<string, bool>>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE4initE8ArrayRefI8MapEntryI6string4boolEE(%"ArrayIterator<MapEntry<string, bool>>"* %this, %"ArrayRef<MapEntry<string, bool>>" %array) {
  %1 = alloca %"ArrayRef<MapEntry<string, bool>>", align 8
  %2 = alloca %"ArrayRef<MapEntry<string, bool>>", align 8
  %3 = alloca %"ArrayRef<MapEntry<string, bool>>", align 8
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, bool>>", %"ArrayIterator<MapEntry<string, bool>>"* %this, i32 0, i32 0
  store %"ArrayRef<MapEntry<string, bool>>" %array, %"ArrayRef<MapEntry<string, bool>>"* %1, align 8
  %4 = call %"MapEntry<string, bool>"* @_EN3std8ArrayRefI8MapEntryI6string4boolEE4dataE(%"ArrayRef<MapEntry<string, bool>>"* %1)
  store %"MapEntry<string, bool>"* %4, %"MapEntry<string, bool>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<MapEntry<string, bool>>", %"ArrayIterator<MapEntry<string, bool>>"* %this, i32 0, i32 1
  store %"ArrayRef<MapEntry<string, bool>>" %array, %"ArrayRef<MapEntry<string, bool>>"* %2, align 8
  %5 = call %"MapEntry<string, bool>"* @_EN3std8ArrayRefI8MapEntryI6string4boolEE4dataE(%"ArrayRef<MapEntry<string, bool>>"* %2)
  store %"ArrayRef<MapEntry<string, bool>>" %array, %"ArrayRef<MapEntry<string, bool>>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI8MapEntryI6string4boolEE4sizeE(%"ArrayRef<MapEntry<string, bool>>"* %3)
  %7 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %5, i32 %6
  store %"MapEntry<string, bool>"* %7, %"MapEntry<string, bool>"** %end, align 8
  ret void
}

define void @_EN3std4ListI8MapEntryI6string4boolEE16indexOutOfBoundsE3int(%"List<MapEntry<string, bool>>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @45, i32 0, i32 0), i32 11)
  store i32 %index, i32* %1, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @46, i32 0, i32 0), i32 27)
  %3 = call i32 @_EN3std4ListI8MapEntryI6string4boolEE4sizeE(%"List<MapEntry<string, bool>>"* %this)
  store i32 %3, i32* %2, align 4
  %4 = call %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %__str, i32* %1, %string* %__str1, i32* %2)
  ret void
}

define void @_EN3std4ListI8MapEntryI6string4boolEE14unsafeRemoveAtE3int(%"List<MapEntry<string, bool>>"* %this, i32 %index) {
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %source = alloca %"MapEntry<string, bool>"*, align 8
  %target = alloca %"MapEntry<string, bool>"*, align 8
  %2 = add i32 %index, 1
  %size = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 %2, i32 %size.load)
  %3 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %3, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %4 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %5 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %5, i32* %i, align 4
  %buffer = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %buffer, align 8
  %i.load = load i32, i32* %i, align 4
  %6 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %buffer.load, i32 %i.load
  store %"MapEntry<string, bool>"* %6, %"MapEntry<string, bool>"** %source, align 8
  %buffer1 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %buffer.load2 = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %buffer1, align 8
  %i.load3 = load i32, i32* %i, align 4
  %7 = sub i32 %i.load3, 1
  %8 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %buffer.load2, i32 %7
  store %"MapEntry<string, bool>"* %8, %"MapEntry<string, bool>"** %target, align 8
  %target.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %target, align 8
  %source.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %source, align 8
  %source.load.load = load %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %source.load, align 8
  store %"MapEntry<string, bool>" %source.load.load, %"MapEntry<string, bool>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %size4 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %size.load5 = load i32, i32* %size4, align 4
  %9 = add i32 %size.load5, -1
  store i32 %9, i32* %size4, align 4
  ret void
}

define %"ArrayIterator<List<MapEntry<string, bool>>>" @_EN3std4ListI4ListI8MapEntryI6string4boolEEE8iteratorE(%"List<List<MapEntry<string, bool>>>"* %this) {
  %1 = alloca %"ArrayIterator<List<MapEntry<string, bool>>>", align 8
  %2 = alloca %"ArrayRef<List<MapEntry<string, bool>>>", align 8
  call void @_EN3std8ArrayRefI4ListI8MapEntryI6string4boolEEE4initEP4ListI4ListI8MapEntryI6string4boolEEE(%"ArrayRef<List<MapEntry<string, bool>>>"* %2, %"List<List<MapEntry<string, bool>>>"* %this)
  %.load = load %"ArrayRef<List<MapEntry<string, bool>>>", %"ArrayRef<List<MapEntry<string, bool>>>"* %2, align 8
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE4initE8ArrayRefI4ListI8MapEntryI6string4boolEEE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %1, %"ArrayRef<List<MapEntry<string, bool>>>" %.load)
  %.load1 = load %"ArrayIterator<List<MapEntry<string, bool>>>", %"ArrayIterator<List<MapEntry<string, bool>>>"* %1, align 8
  ret %"ArrayIterator<List<MapEntry<string, bool>>>" %.load1
}

define void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4growE(%"List<List<MapEntry<string, bool>>>"* %this) {
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE7reserveE3int(%"List<List<MapEntry<string, bool>>>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 2
  %capacity.load2 = load i32, i32* %capacity1, align 4
  %2 = mul i32 %capacity.load2, 2
  call void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE7reserveE3int(%"List<List<MapEntry<string, bool>>>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std4ListI8MapEntryI6string4boolEE7reserveE3int(%"List<MapEntry<string, bool>>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca %"MapEntry<string, bool>"*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca %"MapEntry<string, bool>"*, align 8
  %target = alloca %"MapEntry<string, bool>"*, align 8
  %capacity = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call %"MapEntry<string, bool>"* @_EN3std13allocateArrayI8MapEntryI6string4boolEEE3int(i32 %minimumCapacity)
  store %"MapEntry<string, bool>"* %3, %"MapEntry<string, bool>"** %newBuffer, align 8
  %size = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %buffer.load, i32 %index.load
  store %"MapEntry<string, bool>"* %7, %"MapEntry<string, bool>"** %source, align 8
  %newBuffer.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %newBuffer.load, i32 %index.load1
  store %"MapEntry<string, bool>"* %8, %"MapEntry<string, bool>"** %target, align 8
  %target.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %target, align 8
  %source.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %source, align 8
  %source.load.load = load %"MapEntry<string, bool>", %"MapEntry<string, bool>"* %source.load, align 8
  store %"MapEntry<string, bool>" %source.load.load, %"MapEntry<string, bool>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %buffer.load6 = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %buffer5, align 8
  call void @_EN3std10deallocateIAU_8MapEntryI6string4boolEEEAU_8MapEntryI6string4boolE(%"MapEntry<string, bool>"* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %newBuffer.load10 = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %newBuffer, align 8
  store %"MapEntry<string, bool>"* %newBuffer.load10, %"MapEntry<string, bool>"** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity11, align 4
  br label %if.end
}

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

define void @_EN3std4ListI8MapEntryI6string4boolEE6deinitE(%"List<MapEntry<string, bool>>"* %this) {
  %__iterator = alloca %"ArrayIterator<MapEntry<string, bool>>", align 8
  %element = alloca %"MapEntry<string, bool>"*, align 8
  %capacity = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<MapEntry<string, bool>>" @_EN3std4ListI8MapEntryI6string4boolEE8iteratorE(%"List<MapEntry<string, bool>>"* %this)
  store %"ArrayIterator<MapEntry<string, bool>>" %2, %"ArrayIterator<MapEntry<string, bool>>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE8hasValueE(%"ArrayIterator<MapEntry<string, bool>>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call %"MapEntry<string, bool>"* @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE5valueE(%"ArrayIterator<MapEntry<string, bool>>"* %__iterator)
  store %"MapEntry<string, bool>"* %4, %"MapEntry<string, bool>"** %element, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI8MapEntryI6string4boolEE9incrementE(%"ArrayIterator<MapEntry<string, bool>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %buffer, align 8
  call void @_EN3std10deallocateIAU_8MapEntryI6string4boolEEEAU_8MapEntryI6string4boolE(%"MapEntry<string, bool>"* %buffer.load)
  br label %if.end
}

define void @_EN3std10deallocateIAU_4ListI8MapEntryI6string4boolEEEEAU_4ListI8MapEntryI6string4boolEE(%"List<MapEntry<string, bool>>"* %allocation) {
  %1 = bitcast %"List<MapEntry<string, bool>>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define %"MapEntry<string, bool>"* @_EN3std8ArrayRefI8MapEntryI6string4boolEE4dataE(%"ArrayRef<MapEntry<string, bool>>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<MapEntry<string, bool>>", %"ArrayRef<MapEntry<string, bool>>"* %this, i32 0, i32 0
  %data.load = load %"MapEntry<string, bool>"*, %"MapEntry<string, bool>"** %data, align 8
  ret %"MapEntry<string, bool>"* %data.load
}

define i32 @_EN3std8ArrayRefI8MapEntryI6string4boolEE4sizeE(%"ArrayRef<MapEntry<string, bool>>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<MapEntry<string, bool>>", %"ArrayRef<MapEntry<string, bool>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std8ArrayRefI4ListI8MapEntryI6string4boolEEE4initEP4ListI4ListI8MapEntryI6string4boolEEE(%"ArrayRef<List<MapEntry<string, bool>>>"* %this, %"List<List<MapEntry<string, bool>>>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<List<MapEntry<string, bool>>>", %"ArrayRef<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %1 = call %"List<MapEntry<string, bool>>"* @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4dataE(%"List<List<MapEntry<string, bool>>>"* %list)
  store %"List<MapEntry<string, bool>>"* %1, %"List<MapEntry<string, bool>>"** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<List<MapEntry<string, bool>>>", %"ArrayRef<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4sizeE(%"List<List<MapEntry<string, bool>>>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string4boolEEE4initE8ArrayRefI4ListI8MapEntryI6string4boolEEE(%"ArrayIterator<List<MapEntry<string, bool>>>"* %this, %"ArrayRef<List<MapEntry<string, bool>>>" %array) {
  %1 = alloca %"ArrayRef<List<MapEntry<string, bool>>>", align 8
  %2 = alloca %"ArrayRef<List<MapEntry<string, bool>>>", align 8
  %3 = alloca %"ArrayRef<List<MapEntry<string, bool>>>", align 8
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, bool>>>", %"ArrayIterator<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  store %"ArrayRef<List<MapEntry<string, bool>>>" %array, %"ArrayRef<List<MapEntry<string, bool>>>"* %1, align 8
  %4 = call %"List<MapEntry<string, bool>>"* @_EN3std8ArrayRefI4ListI8MapEntryI6string4boolEEE4dataE(%"ArrayRef<List<MapEntry<string, bool>>>"* %1)
  store %"List<MapEntry<string, bool>>"* %4, %"List<MapEntry<string, bool>>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, bool>>>", %"ArrayIterator<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
  store %"ArrayRef<List<MapEntry<string, bool>>>" %array, %"ArrayRef<List<MapEntry<string, bool>>>"* %2, align 8
  %5 = call %"List<MapEntry<string, bool>>"* @_EN3std8ArrayRefI4ListI8MapEntryI6string4boolEEE4dataE(%"ArrayRef<List<MapEntry<string, bool>>>"* %2)
  store %"ArrayRef<List<MapEntry<string, bool>>>" %array, %"ArrayRef<List<MapEntry<string, bool>>>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI4ListI8MapEntryI6string4boolEEE4sizeE(%"ArrayRef<List<MapEntry<string, bool>>>"* %3)
  %7 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %5, i32 %6
  store %"List<MapEntry<string, bool>>"* %7, %"List<MapEntry<string, bool>>"** %end, align 8
  ret void
}

define void @_EN3std10deallocateIAU_8MapEntryI6string4boolEEEAU_8MapEntryI6string4boolE(%"MapEntry<string, bool>"* %allocation) {
  %1 = bitcast %"MapEntry<string, bool>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string4boolEEE7reserveE3int(%"List<List<MapEntry<string, bool>>>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca %"List<MapEntry<string, bool>>"*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca %"List<MapEntry<string, bool>>"*, align 8
  %target = alloca %"List<MapEntry<string, bool>>"*, align 8
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call %"List<MapEntry<string, bool>>"* @_EN3std13allocateArrayI4ListI8MapEntryI6string4boolEEEE3int(i32 %minimumCapacity)
  store %"List<MapEntry<string, bool>>"* %3, %"List<MapEntry<string, bool>>"** %newBuffer, align 8
  %size = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %buffer.load, i32 %index.load
  store %"List<MapEntry<string, bool>>"* %7, %"List<MapEntry<string, bool>>"** %source, align 8
  %newBuffer.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %newBuffer.load, i32 %index.load1
  store %"List<MapEntry<string, bool>>"* %8, %"List<MapEntry<string, bool>>"** %target, align 8
  %target.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %target, align 8
  %source.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %source, align 8
  %source.load.load = load %"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* %source.load, align 8
  store %"List<MapEntry<string, bool>>" %source.load.load, %"List<MapEntry<string, bool>>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %buffer.load6 = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %buffer5, align 8
  call void @_EN3std10deallocateIAU_4ListI8MapEntryI6string4boolEEEEAU_4ListI8MapEntryI6string4boolEE(%"List<MapEntry<string, bool>>"* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %newBuffer.load10 = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %newBuffer, align 8
  store %"List<MapEntry<string, bool>>"* %newBuffer.load10, %"List<MapEntry<string, bool>>"** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity11, align 4
  br label %if.end
}

define %"MapEntry<string, bool>"* @_EN3std13allocateArrayI8MapEntryI6string4boolEEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (%"MapEntry<string, bool>"* getelementptr (%"MapEntry<string, bool>", %"MapEntry<string, bool>"* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @47, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to %"MapEntry<string, bool>"*
  ret %"MapEntry<string, bool>"* %4
}

declare %OutputFileStream @_EN3std6stderrE()

declare void @_EN3std16OutputFileStream5writeI6stringEEP6string(%OutputFileStream*, %string*)

declare void @_EN3std16OutputFileStream5writeI3intEEP3int(%OutputFileStream*, i32*)

declare void @_EN3std16OutputFileStream5writeI4charEEP4char(%OutputFileStream*, i8*)

declare %never @_EN3std12abortWrapperE()

declare void @free(i8*)

define %"List<MapEntry<string, bool>>"* @_EN3std8ArrayRefI4ListI8MapEntryI6string4boolEEE4dataE(%"ArrayRef<List<MapEntry<string, bool>>>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<List<MapEntry<string, bool>>>", %"ArrayRef<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %data.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %data, align 8
  ret %"List<MapEntry<string, bool>>"* %data.load
}

define i32 @_EN3std8ArrayRefI4ListI8MapEntryI6string4boolEEE4sizeE(%"ArrayRef<List<MapEntry<string, bool>>>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<List<MapEntry<string, bool>>>", %"ArrayRef<List<MapEntry<string, bool>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %"List<MapEntry<string, bool>>"* @_EN3std4ListI4ListI8MapEntryI6string4boolEEE4dataE(%"List<List<MapEntry<string, bool>>>"* %this) {
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, bool>>>", %"List<List<MapEntry<string, bool>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, bool>>"*, %"List<MapEntry<string, bool>>"** %buffer, align 8
  ret %"List<MapEntry<string, bool>>"* %buffer.load
}

define %"List<MapEntry<string, bool>>"* @_EN3std13allocateArrayI4ListI8MapEntryI6string4boolEEEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (%"List<MapEntry<string, bool>>"* getelementptr (%"List<MapEntry<string, bool>>", %"List<MapEntry<string, bool>>"* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @48, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to %"List<MapEntry<string, bool>>"*
  ret %"List<MapEntry<string, bool>>"* %4
}

declare i8* @malloc(i64)
