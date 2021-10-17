
%"Map<string, int>" = type { %"List<List<MapEntry<string, int>>>", i32 }
%"List<List<MapEntry<string, int>>>" = type { %"List<MapEntry<string, int>>"*, i32, i32 }
%"List<MapEntry<string, int>>" = type { %"MapEntry<string, int>"*, i32, i32 }
%"MapEntry<string, int>" = type { %string, i32 }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }
%"Map<StringBuffer, int>" = type { %"List<List<MapEntry<StringBuffer, int>>>", i32 }
%"List<List<MapEntry<StringBuffer, int>>>" = type { %"List<MapEntry<StringBuffer, int>>"*, i32, i32 }
%"List<MapEntry<StringBuffer, int>>" = type { %"MapEntry<StringBuffer, int>"*, i32, i32 }
%"MapEntry<StringBuffer, int>" = type { %StringBuffer, i32 }
%StringBuffer = type { %"List<char>" }
%"List<char>" = type { i8*, i32, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }
%"Map<string, string>" = type { %"List<List<MapEntry<string, string>>>", i32 }
%"List<List<MapEntry<string, string>>>" = type { %"List<MapEntry<string, string>>"*, i32, i32 }
%"List<MapEntry<string, string>>" = type { %"MapEntry<string, string>"*, i32, i32 }
%"MapEntry<string, string>" = type { %string, %string }
%"MapIterator<string, string>" = type { %"ArrayIterator<List<MapEntry<string, string>>>", %"MapEntry<string, string>"* }
%"ArrayIterator<List<MapEntry<string, string>>>" = type { %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"* }
%"ArrayIterator<MapEntry<string, int>>" = type { %"MapEntry<string, int>"*, %"MapEntry<string, int>"* }
%"ArrayIterator<List<MapEntry<string, int>>>" = type { %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"* }
%"ArrayRef<MapEntry<string, int>>" = type { %"MapEntry<string, int>"*, i32 }
%"ArrayIterator<MapEntry<string, string>>" = type { %"MapEntry<string, string>"*, %"MapEntry<string, string>"* }
%"ArrayIterator<List<MapEntry<StringBuffer, int>>>" = type { %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"* }
%"ArrayIterator<MapEntry<StringBuffer, int>>" = type { %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"* }
%"ArrayRef<List<MapEntry<string, int>>>" = type { %"List<MapEntry<string, int>>"*, i32 }
%never = type {}
%"ArrayRef<List<MapEntry<string, string>>>" = type { %"List<MapEntry<string, string>>"*, i32 }
%"ArrayRef<MapEntry<string, string>>" = type { %"MapEntry<string, string>"*, i32 }
%"ArrayRef<List<MapEntry<StringBuffer, int>>>" = type { %"List<MapEntry<StringBuffer, int>>"*, i32 }
%"ArrayRef<MapEntry<StringBuffer, int>>" = type { %"MapEntry<StringBuffer, int>"*, i32 }
%OutputFileStream = type { %FILE* }
%FILE = type {}

@0 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@2 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:21:5\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"lol\00", align 1
@4 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:22:5\0A\00", align 1
@5 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:59:5\0A\00", align 1
@6 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@7 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@8 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:63:5\0A\00", align 1
@9 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:64:5\0A\00", align 1
@10 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@11 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@12 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:67:5\0A\00", align 1
@13 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:68:5\0A\00", align 1
@14 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@15 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@16 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@17 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:76:5\0A\00", align 1
@18 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@19 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:78:5\0A\00", align 1
@20 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@21 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@22 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:81:5\0A\00", align 1
@23 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@24 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:82:5\0A\00", align 1
@25 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@26 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@27 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:37:5\0A\00", align 1
@28 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:53:5\0A\00", align 1
@29 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:28:5\0A\00", align 1
@30 = private unnamed_addr constant [5 x i8] c"best\00", align 1
@31 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@32 = private unnamed_addr constant [39 x i8] c"Assertion failed at map-tests.cx:30:5\0A\00", align 1
@33 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@34 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@35 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@36 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@37 = private unnamed_addr constant [3 x i8] c"bb\00", align 1
@38 = private unnamed_addr constant [3 x i8] c"bb\00", align 1
@39 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@40 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@41 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@42 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@43 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@44 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@45 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@46 = private unnamed_addr constant [3 x i8] c"bb\00", align 1
@47 = private unnamed_addr constant [41 x i8] c"Assertion failed at map-tests.cx:101:13\0A\00", align 1
@48 = private unnamed_addr constant [4 x i8] c"ccc\00", align 1
@49 = private unnamed_addr constant [41 x i8] c"Assertion failed at map-tests.cx:104:13\0A\00", align 1
@50 = private unnamed_addr constant [41 x i8] c"Assertion failed at map-tests.cx:107:13\0A\00", align 1
@51 = private unnamed_addr constant [41 x i8] c"Assertion failed at map-tests.cx:110:13\0A\00", align 1
@52 = private unnamed_addr constant [40 x i8] c"Assertion failed at map-tests.cx:114:5\0A\00", align 1
@53 = private unnamed_addr constant [40 x i8] c"Assertion failed at map-tests.cx:115:5\0A\00", align 1
@54 = private unnamed_addr constant [40 x i8] c"Assertion failed at map-tests.cx:116:5\0A\00", align 1
@55 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@56 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@57 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@58 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@59 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@60 = private unnamed_addr constant [40 x i8] c"Assertion failed at map-tests.cx:126:9\0A\00", align 1
@61 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@62 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@63 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@64 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@65 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@66 = private unnamed_addr constant [40 x i8] c"Assertion failed at map-tests.cx:138:9\0A\00", align 1
@67 = private unnamed_addr constant [40 x i8] c"Assertion failed at map-tests.cx:141:5\0A\00", align 1
@68 = private unnamed_addr constant [12 x i8] c"List index \00", align 1
@69 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@70 = private unnamed_addr constant [12 x i8] c"List index \00", align 1
@71 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@72 = private unnamed_addr constant [12 x i8] c"List index \00", align 1
@73 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@74 = private unnamed_addr constant [12 x i8] c"List index \00", align 1
@75 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@76 = private unnamed_addr constant [12 x i8] c"List index \00", align 1
@77 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@78 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1
@79 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1
@80 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1
@81 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1
@82 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1
@83 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1

define i32 @main() {
  call void @_EN4main10testInsertE()
  call void @_EN4main11testRemovalE()
  call void @_EN4main14testDuplicatesE()
  call void @_EN4main10testGetterE()
  call void @_EN4main10testResizeE()
  call void @_EN4main8testSizeE()
  call void @_EN4main12testIteratorE()
  call void @_EN4main20testEmptyMapIteratorE()
  call void @_EN4main19testUnitMapIteratorE()
  ret i32 0
}

define void @_EN4main10testInsertE() {
  %map = alloca %"Map<string, int>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  call void @_EN3std3MapI6string3intE4initE(%"Map<string, int>"* %map)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i32 2)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std3MapI6string3intE6insertE6string3int(%"Map<string, int>"* %map, %string %__str.load, i32 1)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i32 2)
  %1 = call i1 @_EN3std3MapI6string3intE8containsEP6string(%"Map<string, int>"* %map, %string* %__str1)
  %assert.condition = icmp eq i1 %1, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @2, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @3, i32 0, i32 0), i32 3)
  %2 = call i1 @_EN3std3MapI6string3intE8containsEP6string(%"Map<string, int>"* %map, %string* %__str2)
  %3 = xor i1 %2, true
  %assert.condition3 = icmp eq i1 %3, false
  br i1 %assert.condition3, label %assert.fail4, label %assert.success5

assert.fail4:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @4, i32 0, i32 0))
  unreachable

assert.success5:                                  ; preds = %assert.success
  call void @_EN3std3MapI6string3intE6deinitE(%"Map<string, int>"* %map)
  ret void
}

define void @_EN4main11testRemovalE() {
  %map = alloca %"Map<string, int>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  call void @_EN3std3MapI6string3intE4initE(%"Map<string, int>"* %map)
  %1 = call i32 @_EN3std3MapI6string3intE4sizeE(%"Map<string, int>"* %map)
  %2 = icmp eq i32 %1, 0
  %assert.condition = icmp eq i1 %2, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @5, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0), i32 2)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std3MapI6string3intE6insertE6string3int(%"Map<string, int>"* %map, %string %__str.load, i32 2)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @7, i32 0, i32 0), i32 2)
  %3 = call i1 @_EN3std3MapI6string3intE8containsEP6string(%"Map<string, int>"* %map, %string* %__str1)
  %assert.condition4 = icmp eq i1 %3, false
  br i1 %assert.condition4, label %assert.fail5, label %assert.success6

assert.fail5:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @8, i32 0, i32 0))
  unreachable

assert.success6:                                  ; preds = %assert.success
  %4 = call i32 @_EN3std3MapI6string3intE4sizeE(%"Map<string, int>"* %map)
  %5 = icmp eq i32 %4, 1
  %assert.condition7 = icmp eq i1 %5, false
  br i1 %assert.condition7, label %assert.fail8, label %assert.success9

assert.fail8:                                     ; preds = %assert.success6
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @9, i32 0, i32 0))
  unreachable

assert.success9:                                  ; preds = %assert.success6
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0), i32 2)
  call void @_EN3std3MapI6string3intE6removeEP6string(%"Map<string, int>"* %map, %string* %__str2)
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @11, i32 0, i32 0), i32 2)
  %6 = call i1 @_EN3std3MapI6string3intE8containsEP6string(%"Map<string, int>"* %map, %string* %__str3)
  %7 = xor i1 %6, true
  %assert.condition10 = icmp eq i1 %7, false
  br i1 %assert.condition10, label %assert.fail11, label %assert.success12

assert.fail11:                                    ; preds = %assert.success9
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @12, i32 0, i32 0))
  unreachable

assert.success12:                                 ; preds = %assert.success9
  %8 = call i32 @_EN3std3MapI6string3intE4sizeE(%"Map<string, int>"* %map)
  %9 = icmp eq i32 %8, 0
  %assert.condition13 = icmp eq i1 %9, false
  br i1 %assert.condition13, label %assert.fail14, label %assert.success15

assert.fail14:                                    ; preds = %assert.success12
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @13, i32 0, i32 0))
  unreachable

assert.success15:                                 ; preds = %assert.success12
  call void @_EN3std3MapI6string3intE6deinitE(%"Map<string, int>"* %map)
  ret void
}

define void @_EN4main14testDuplicatesE() {
  %map = alloca %"Map<string, int>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str4 = alloca %string, align 8
  %__str5 = alloca %string, align 8
  %__str6 = alloca %string, align 8
  call void @_EN3std3MapI6string3intE4initE(%"Map<string, int>"* %map)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @14, i32 0, i32 0), i32 2)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std3MapI6string3intE6insertE6string3int(%"Map<string, int>"* %map, %string %__str.load, i32 1)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @15, i32 0, i32 0), i32 2)
  %__str.load7 = load %string, %string* %__str1, align 8
  call void @_EN3std3MapI6string3intE6insertE6string3int(%"Map<string, int>"* %map, %string %__str.load7, i32 2)
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @16, i32 0, i32 0), i32 2)
  %2 = call i1 @_EN3std3MapI6string3intE8containsEP6string(%"Map<string, int>"* %map, %string* %__str2)
  %assert.condition = icmp eq i1 %2, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @17, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @18, i32 0, i32 0), i32 2)
  %3 = call i32* @_EN3std3MapI6string3intEixEP6string(%"Map<string, int>"* %map, %string* %__str3)
  store i32 1, i32* %1, align 4
  %4 = call i1 @_EN3stdeqI3intEEOP3intR3int(i32* %3, i32* %1)
  %assert.condition8 = icmp eq i1 %4, false
  br i1 %assert.condition8, label %assert.fail9, label %assert.success10

assert.fail9:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @19, i32 0, i32 0))
  unreachable

assert.success10:                                 ; preds = %assert.success
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @20, i32 0, i32 0), i32 2)
  call void @_EN3std3MapI6string3intE6removeEP6string(%"Map<string, int>"* %map, %string* %__str4)
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @21, i32 0, i32 0), i32 2)
  %5 = call i1 @_EN3std3MapI6string3intE8containsEP6string(%"Map<string, int>"* %map, %string* %__str5)
  %6 = xor i1 %5, true
  %assert.condition11 = icmp eq i1 %6, false
  br i1 %assert.condition11, label %assert.fail12, label %assert.success13

assert.fail12:                                    ; preds = %assert.success10
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @22, i32 0, i32 0))
  unreachable

assert.success13:                                 ; preds = %assert.success10
  call void @_EN3std6string4initEP4char3int(%string* %__str6, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @23, i32 0, i32 0), i32 2)
  %7 = call i32* @_EN3std3MapI6string3intEixEP6string(%"Map<string, int>"* %map, %string* %__str6)
  %8 = icmp eq i32* %7, null
  %assert.condition14 = icmp eq i1 %8, false
  br i1 %assert.condition14, label %assert.fail15, label %assert.success16

assert.fail15:                                    ; preds = %assert.success13
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @24, i32 0, i32 0))
  unreachable

assert.success16:                                 ; preds = %assert.success13
  call void @_EN3std3MapI6string3intE6deinitE(%"Map<string, int>"* %map)
  ret void
}

define void @_EN4main10testGetterE() {
  %map = alloca %"Map<string, int>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %1 = alloca i32, align 4
  call void @_EN3std3MapI6string3intE4initE(%"Map<string, int>"* %map)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @25, i32 0, i32 0), i32 2)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std3MapI6string3intE6insertE6string3int(%"Map<string, int>"* %map, %string %__str.load, i32 42)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @26, i32 0, i32 0), i32 2)
  %2 = call i32* @_EN3std3MapI6string3intEixEP6string(%"Map<string, int>"* %map, %string* %__str1)
  store i32 42, i32* %1, align 4
  %3 = call i1 @_EN3stdeqI3intEEOP3intR3int(i32* %2, i32* %1)
  %assert.condition = icmp eq i1 %3, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @27, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std3MapI6string3intE6deinitE(%"Map<string, int>"* %map)
  ret void
}

define void @_EN4main10testResizeE() {
  %map = alloca %"Map<StringBuffer, int>", align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %lol = alloca %StringBuffer, align 8
  %__iterator1 = alloca %"RangeIterator<int>", align 8
  %2 = alloca %"Range<int>", align 8
  %j = alloca i32, align 4
  call void @_EN3std3MapI12StringBuffer3intE4initE(%"Map<StringBuffer, int>"* %map)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 255)
  %3 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %3, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %4 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %5 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %5, i32* %i, align 4
  call void @_EN3std12StringBuffer4initE(%StringBuffer* %lol)
  %i.load = load i32, i32* %i, align 4
  %6 = add i32 %i.load, 1
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %2, i32 0, i32 %6)
  %7 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %2)
  store %"RangeIterator<int>" %7, %"RangeIterator<int>"* %__iterator1, align 4
  br label %loop.condition1

loop.increment:                                   ; preds = %loop.end4
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %8 = call i32 @_EN3std3MapI12StringBuffer3intE4sizeE(%"Map<StringBuffer, int>"* %map)
  %9 = icmp eq i32 %8, 255
  %assert.condition = icmp eq i1 %9, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

loop.condition1:                                  ; preds = %loop.increment3, %loop.body
  %10 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator1)
  br i1 %10, label %loop.body2, label %loop.end4

loop.body2:                                       ; preds = %loop.condition1
  %11 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator1)
  store i32 %11, i32* %j, align 4
  call void @_EN3std12StringBuffer4pushE4char(%StringBuffer* %lol, i8 97)
  br label %loop.increment3

loop.increment3:                                  ; preds = %loop.body2
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator1)
  br label %loop.condition1

loop.end4:                                        ; preds = %loop.condition1
  %lol.load = load %StringBuffer, %StringBuffer* %lol, align 8
  %i.load5 = load i32, i32* %i, align 4
  call void @_EN3std3MapI12StringBuffer3intE6insertE12StringBuffer3int(%"Map<StringBuffer, int>"* %map, %StringBuffer %lol.load, i32 %i.load5)
  br label %loop.increment

assert.fail:                                      ; preds = %loop.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @28, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.end
  call void @_EN3std3MapI12StringBuffer3intE6deinitE(%"Map<StringBuffer, int>"* %map)
  ret void
}

define void @_EN4main8testSizeE() {
  %map = alloca %"Map<string, string>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  call void @_EN3std3MapI6string6stringE4initE(%"Map<string, string>"* %map)
  %1 = call i32 @_EN3std3MapI6string6stringE4sizeE(%"Map<string, string>"* %map)
  %2 = icmp eq i32 %1, 0
  %assert.condition = icmp eq i1 %2, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @29, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @30, i32 0, i32 0), i32 4)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @31, i32 0, i32 0), i32 2)
  %__str.load2 = load %string, %string* %__str1, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load, %string %__str.load2)
  %3 = call i32 @_EN3std3MapI6string6stringE4sizeE(%"Map<string, string>"* %map)
  %4 = icmp eq i32 %3, 1
  %assert.condition3 = icmp eq i1 %4, false
  br i1 %assert.condition3, label %assert.fail4, label %assert.success5

assert.fail4:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @32, i32 0, i32 0))
  unreachable

assert.success5:                                  ; preds = %assert.success
  call void @_EN3std3MapI6string6stringE6deinitE(%"Map<string, string>"* %map)
  ret void
}

define void @_EN4main12testIteratorE() {
  %map = alloca %"Map<string, string>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %__str5 = alloca %string, align 8
  %__str6 = alloca %string, align 8
  %__str7 = alloca %string, align 8
  %__str8 = alloca %string, align 8
  %__str9 = alloca %string, align 8
  %__str10 = alloca %string, align 8
  %__str11 = alloca %string, align 8
  %a = alloca i1, align 1
  %bb = alloca i1, align 1
  %ccc = alloca i1, align 1
  %__iterator = alloca %"MapIterator<string, string>", align 8
  %e = alloca %"MapEntry<string, string>"*, align 8
  %__str12 = alloca %string, align 8
  %__str13 = alloca %string, align 8
  %__str14 = alloca %string, align 8
  call void @_EN3std3MapI6string6stringE4initE(%"Map<string, string>"* %map)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @33, i32 0, i32 0), i32 1)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @34, i32 0, i32 0), i32 1)
  %__str.load15 = load %string, %string* %__str1, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load, %string %__str.load15)
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @35, i32 0, i32 0), i32 3)
  %__str.load16 = load %string, %string* %__str2, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @36, i32 0, i32 0), i32 3)
  %__str.load17 = load %string, %string* %__str3, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load16, %string %__str.load17)
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @37, i32 0, i32 0), i32 2)
  %__str.load18 = load %string, %string* %__str4, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @38, i32 0, i32 0), i32 2)
  %__str.load19 = load %string, %string* %__str5, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load18, %string %__str.load19)
  call void @_EN3std6string4initEP4char3int(%string* %__str6, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @39, i32 0, i32 0), i32 1)
  %__str.load20 = load %string, %string* %__str6, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str7, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @40, i32 0, i32 0), i32 1)
  %__str.load21 = load %string, %string* %__str7, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load20, %string %__str.load21)
  call void @_EN3std6string4initEP4char3int(%string* %__str8, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @41, i32 0, i32 0), i32 3)
  %__str.load22 = load %string, %string* %__str8, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str9, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @42, i32 0, i32 0), i32 3)
  %__str.load23 = load %string, %string* %__str9, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load22, %string %__str.load23)
  call void @_EN3std6string4initEP4char3int(%string* %__str10, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @43, i32 0, i32 0), i32 3)
  %__str.load24 = load %string, %string* %__str10, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str11, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @44, i32 0, i32 0), i32 3)
  %__str.load25 = load %string, %string* %__str11, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load24, %string %__str.load25)
  store i1 false, i1* %a, align 1
  store i1 false, i1* %bb, align 1
  store i1 false, i1* %ccc, align 1
  %1 = call %"MapIterator<string, string>" @_EN3std3MapI6string6stringE8iteratorE(%"Map<string, string>"* %map)
  store %"MapIterator<string, string>" %1, %"MapIterator<string, string>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std11MapIteratorI6string6stringE8hasValueE(%"MapIterator<string, string>"* %__iterator)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call %"MapEntry<string, string>"* @_EN3std11MapIteratorI6string6stringE5valueE(%"MapIterator<string, string>"* %__iterator)
  store %"MapEntry<string, string>"* %3, %"MapEntry<string, string>"** %e, align 8
  %e.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %e, align 8
  %key = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %e.load, i32 0, i32 0
  %key.load = load %string, %string* %key, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str12, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @45, i32 0, i32 0), i32 1)
  %__str.load26 = load %string, %string* %__str12, align 8
  %4 = call i1 @_EN3stdeqE6string6string(%string %key.load, %string %__str.load26)
  br i1 %4, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std11MapIteratorI6string6stringE9incrementE(%"MapIterator<string, string>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %a.load = load i1, i1* %a, align 1
  %assert.condition = icmp eq i1 %a.load, false
  br i1 %assert.condition, label %assert.fail51, label %assert.success52

if.then:                                          ; preds = %loop.body
  %a.load27 = load i1, i1* %a, align 1
  %5 = xor i1 %a.load27, true
  %assert.condition28 = icmp eq i1 %5, false
  br i1 %assert.condition28, label %assert.fail, label %assert.success

if.else:                                          ; preds = %loop.body
  %e.load29 = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %e, align 8
  %key30 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %e.load29, i32 0, i32 0
  %key.load31 = load %string, %string* %key30, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str13, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @46, i32 0, i32 0), i32 2)
  %__str.load32 = load %string, %string* %__str13, align 8
  %6 = call i1 @_EN3stdeqE6string6string(%string %key.load31, %string %__str.load32)
  br i1 %6, label %if.then33, label %if.else35

if.end:                                           ; preds = %if.end40, %assert.success
  br label %loop.increment

assert.fail:                                      ; preds = %if.then
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([41 x i8], [41 x i8]* @47, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %if.then
  store i1 true, i1* %a, align 1
  br label %if.end

if.then33:                                        ; preds = %if.else
  %bb.load = load i1, i1* %bb, align 1
  %7 = xor i1 %bb.load, true
  %assert.condition34 = icmp eq i1 %7, false
  br i1 %assert.condition34, label %assert.fail41, label %assert.success42

if.else35:                                        ; preds = %if.else
  %e.load36 = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %e, align 8
  %key37 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %e.load36, i32 0, i32 0
  %key.load38 = load %string, %string* %key37, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str14, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @48, i32 0, i32 0), i32 3)
  %__str.load39 = load %string, %string* %__str14, align 8
  %8 = call i1 @_EN3stdeqE6string6string(%string %key.load38, %string %__str.load39)
  br i1 %8, label %if.then43, label %if.else45

if.end40:                                         ; preds = %if.end46, %assert.success42
  br label %if.end

assert.fail41:                                    ; preds = %if.then33
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([41 x i8], [41 x i8]* @49, i32 0, i32 0))
  unreachable

assert.success42:                                 ; preds = %if.then33
  store i1 true, i1* %bb, align 1
  br label %if.end40

if.then43:                                        ; preds = %if.else35
  %ccc.load = load i1, i1* %ccc, align 1
  %9 = xor i1 %ccc.load, true
  %assert.condition44 = icmp eq i1 %9, false
  br i1 %assert.condition44, label %assert.fail47, label %assert.success48

if.else45:                                        ; preds = %if.else35
  br i1 true, label %assert.fail49, label %assert.success50

if.end46:                                         ; preds = %assert.success50, %assert.success48
  br label %if.end40

assert.fail47:                                    ; preds = %if.then43
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([41 x i8], [41 x i8]* @50, i32 0, i32 0))
  unreachable

assert.success48:                                 ; preds = %if.then43
  store i1 true, i1* %ccc, align 1
  br label %if.end46

assert.fail49:                                    ; preds = %if.else45
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([41 x i8], [41 x i8]* @51, i32 0, i32 0))
  unreachable

assert.success50:                                 ; preds = %if.else45
  br label %if.end46

assert.fail51:                                    ; preds = %loop.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @52, i32 0, i32 0))
  unreachable

assert.success52:                                 ; preds = %loop.end
  %bb.load53 = load i1, i1* %bb, align 1
  %assert.condition54 = icmp eq i1 %bb.load53, false
  br i1 %assert.condition54, label %assert.fail55, label %assert.success56

assert.fail55:                                    ; preds = %assert.success52
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @53, i32 0, i32 0))
  unreachable

assert.success56:                                 ; preds = %assert.success52
  %ccc.load57 = load i1, i1* %ccc, align 1
  %assert.condition58 = icmp eq i1 %ccc.load57, false
  br i1 %assert.condition58, label %assert.fail59, label %assert.success60

assert.fail59:                                    ; preds = %assert.success56
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @54, i32 0, i32 0))
  unreachable

assert.success60:                                 ; preds = %assert.success56
  call void @_EN3std3MapI6string6stringE6deinitE(%"Map<string, string>"* %map)
  ret void
}

define void @_EN4main20testEmptyMapIteratorE() {
  %map = alloca %"Map<string, string>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %__iterator = alloca %"MapIterator<string, string>", align 8
  %e = alloca %"MapEntry<string, string>"*, align 8
  call void @_EN3std3MapI6string6stringE4initE(%"Map<string, string>"* %map)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @55, i32 0, i32 0), i32 3)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @56, i32 0, i32 0), i32 3)
  %__str.load5 = load %string, %string* %__str1, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load, %string %__str.load5)
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @57, i32 0, i32 0), i32 3)
  %__str.load6 = load %string, %string* %__str2, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @58, i32 0, i32 0), i32 3)
  %__str.load7 = load %string, %string* %__str3, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load6, %string %__str.load7)
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @59, i32 0, i32 0), i32 3)
  call void @_EN3std3MapI6string6stringE6removeEP6string(%"Map<string, string>"* %map, %string* %__str4)
  %1 = call %"MapIterator<string, string>" @_EN3std3MapI6string6stringE8iteratorE(%"Map<string, string>"* %map)
  store %"MapIterator<string, string>" %1, %"MapIterator<string, string>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std11MapIteratorI6string6stringE8hasValueE(%"MapIterator<string, string>"* %__iterator)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call %"MapEntry<string, string>"* @_EN3std11MapIteratorI6string6stringE5valueE(%"MapIterator<string, string>"* %__iterator)
  store %"MapEntry<string, string>"* %3, %"MapEntry<string, string>"** %e, align 8
  br i1 true, label %assert.fail, label %assert.success

loop.increment:                                   ; preds = %assert.success
  call void @_EN3std11MapIteratorI6string6stringE9incrementE(%"MapIterator<string, string>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std3MapI6string6stringE6deinitE(%"Map<string, string>"* %map)
  ret void

assert.fail:                                      ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @60, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.body
  br label %loop.increment
}

define void @_EN4main19testUnitMapIteratorE() {
  %map = alloca %"Map<string, string>", align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %count = alloca i32, align 4
  %__iterator = alloca %"MapIterator<string, string>", align 8
  %e = alloca %"MapEntry<string, string>"*, align 8
  %__str4 = alloca %string, align 8
  call void @_EN3std3MapI6string6stringE4initE(%"Map<string, string>"* %map)
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @61, i32 0, i32 0), i32 3)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @62, i32 0, i32 0), i32 3)
  %__str.load5 = load %string, %string* %__str1, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load, %string %__str.load5)
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @63, i32 0, i32 0), i32 3)
  %__str.load6 = load %string, %string* %__str2, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @64, i32 0, i32 0), i32 3)
  %__str.load7 = load %string, %string* %__str3, align 8
  call void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %map, %string %__str.load6, %string %__str.load7)
  store i32 0, i32* %count, align 4
  %1 = call %"MapIterator<string, string>" @_EN3std3MapI6string6stringE8iteratorE(%"Map<string, string>"* %map)
  store %"MapIterator<string, string>" %1, %"MapIterator<string, string>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std11MapIteratorI6string6stringE8hasValueE(%"MapIterator<string, string>"* %__iterator)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call %"MapEntry<string, string>"* @_EN3std11MapIteratorI6string6stringE5valueE(%"MapIterator<string, string>"* %__iterator)
  store %"MapEntry<string, string>"* %3, %"MapEntry<string, string>"** %e, align 8
  %count.load = load i32, i32* %count, align 4
  %4 = add i32 %count.load, 1
  store i32 %4, i32* %count, align 4
  %e.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %e, align 8
  %key = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %e.load, i32 0, i32 0
  %key.load = load %string, %string* %key, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @65, i32 0, i32 0), i32 3)
  %__str.load8 = load %string, %string* %__str4, align 8
  %5 = call i1 @_EN3stdeqE6string6string(%string %key.load, %string %__str.load8)
  %assert.condition = icmp eq i1 %5, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

loop.increment:                                   ; preds = %assert.success
  call void @_EN3std11MapIteratorI6string6stringE9incrementE(%"MapIterator<string, string>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %count.load9 = load i32, i32* %count, align 4
  %6 = icmp eq i32 %count.load9, 1
  %assert.condition10 = icmp eq i1 %6, false
  br i1 %assert.condition10, label %assert.fail11, label %assert.success12

assert.fail:                                      ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @66, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.body
  br label %loop.increment

assert.fail11:                                    ; preds = %loop.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @67, i32 0, i32 0))
  unreachable

assert.success12:                                 ; preds = %loop.end
  call void @_EN3std3MapI6string6stringE6deinitE(%"Map<string, string>"* %map)
  ret void
}

define void @_EN3std3MapI6string3intE4initE(%"Map<string, int>"* %this) {
  %1 = alloca %"List<List<MapEntry<string, int>>>", align 8
  %size = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %hashTable = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI6string3intEEE4initE(%"List<List<MapEntry<string, int>>>"* %1)
  %.load = load %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %1, align 8
  store %"List<List<MapEntry<string, int>>>" %.load, %"List<List<MapEntry<string, int>>>"* %hashTable, align 8
  %hashTable1 = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 0
  call void @_EN3std3MapI6string3intE17increaseTableSizeEP4ListI4ListI8MapEntryI6string3intEEE3int(%"Map<string, int>"* %this, %"List<List<MapEntry<string, int>>>"* %hashTable1, i32 128)
  ret void
}

declare void @_EN3std6string4initEP4char3int(%string*, i8*, i32)

define void @_EN3std3MapI6string3intE6insertE6string3int(%"Map<string, int>"* %this, %string %key, i32 %value) {
  %1 = alloca %string, align 8
  %hashValue = alloca i32, align 4
  %2 = alloca %string, align 8
  %3 = alloca %"MapEntry<string, int>", align 8
  store %string %key, %string* %1, align 8
  %4 = call i1 @_EN3std3MapI6string3intE8containsEP6string(%"Map<string, int>"* %this, %string* %1)
  br i1 %4, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  store %string %key, %string* %2, align 8
  %5 = call i64 @_EN3std6string4hashE(%string* %2)
  %6 = call i32 @_EN3std11convertHashE6uint64(i64 %5)
  %7 = call i32 @_EN3std3MapI6string3intE8capacityE(%"Map<string, int>"* %this)
  %8 = srem i32 %6, %7
  store i32 %8, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %9 = call %"List<MapEntry<string, int>>"* @_EN3std4ListI4ListI8MapEntryI6string3intEEEixE3int(%"List<List<MapEntry<string, int>>>"* %hashTable, i32 %hashValue.load)
  call void @_EN3std8MapEntryI6string3intE4initE6string3int(%"MapEntry<string, int>"* %3, %string %key, i32 %value)
  %.load = load %"MapEntry<string, int>", %"MapEntry<string, int>"* %3, align 8
  call void @_EN3std4ListI8MapEntryI6string3intEE4pushE8MapEntryI6string3intE(%"List<MapEntry<string, int>>"* %9, %"MapEntry<string, int>" %.load)
  %size = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %10 = add i32 %size.load, 1
  store i32 %10, i32* %size, align 4
  %11 = call double @_EN3std3MapI6string3intE10loadFactorE(%"Map<string, int>"* %this)
  %12 = fcmp ogt double %11, 0x3FE51EB85FE0EECB
  br i1 %12, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.end
  call void @_EN3std3MapI6string3intE6resizeE(%"Map<string, int>"* %this)
  br label %if.end3

if.else2:                                         ; preds = %if.end
  br label %if.end3

if.end3:                                          ; preds = %if.else2, %if.then1
  ret void
}

define i1 @_EN3std3MapI6string3intE8containsEP6string(%"Map<string, int>"* %this, %string* %e) {
  %hashValue = alloca i32, align 4
  %slot = alloca %"List<MapEntry<string, int>>", align 8
  %__iterator = alloca %"ArrayIterator<MapEntry<string, int>>", align 8
  %element = alloca %"MapEntry<string, int>"*, align 8
  %1 = call i64 @_EN3std6string4hashE(%string* %e)
  %2 = call i32 @_EN3std11convertHashE6uint64(i64 %1)
  %3 = call i32 @_EN3std3MapI6string3intE8capacityE(%"Map<string, int>"* %this)
  %4 = srem i32 %2, %3
  store i32 %4, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %5 = call %"List<MapEntry<string, int>>"* @_EN3std4ListI4ListI8MapEntryI6string3intEEEixE3int(%"List<List<MapEntry<string, int>>>"* %hashTable, i32 %hashValue.load)
  %.load = load %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %5, align 8
  store %"List<MapEntry<string, int>>" %.load, %"List<MapEntry<string, int>>"* %slot, align 8
  %6 = call i32 @_EN3std4ListI8MapEntryI6string3intEE4sizeE(%"List<MapEntry<string, int>>"* %slot)
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %slot)
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %8 = call %"ArrayIterator<MapEntry<string, int>>" @_EN3std4ListI8MapEntryI6string3intEE8iteratorE(%"List<MapEntry<string, int>>"* %slot)
  store %"ArrayIterator<MapEntry<string, int>>" %8, %"ArrayIterator<MapEntry<string, int>>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %if.end
  %9 = call i1 @_EN3std13ArrayIteratorI8MapEntryI6string3intEE8hasValueE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator)
  br i1 %9, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %10 = call %"MapEntry<string, int>"* @_EN3std13ArrayIteratorI8MapEntryI6string3intEE5valueE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator)
  store %"MapEntry<string, int>"* %10, %"MapEntry<string, int>"** %element, align 8
  %element.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %element, align 8
  %key = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %element.load, i32 0, i32 0
  %key.load = load %string, %string* %key, align 8
  %e.load = load %string, %string* %e, align 8
  %11 = call i1 @_EN3stdeqE6string6string(%string %key.load, %string %e.load)
  br i1 %11, label %if.then1, label %if.else2

loop.increment:                                   ; preds = %if.end3
  call void @_EN3std13ArrayIteratorI8MapEntryI6string3intEE9incrementE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %slot)
  ret i1 false

if.then1:                                         ; preds = %loop.body
  call void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %slot)
  ret i1 true

if.else2:                                         ; preds = %loop.body
  br label %if.end3

if.end3:                                          ; preds = %if.else2
  br label %loop.increment
}

declare void @_EN3std10assertFailEP4char(i8*)

define void @_EN3std3MapI6string3intE6deinitE(%"Map<string, int>"* %this) {
  %hashTable = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI6string3intEEE6deinitE(%"List<List<MapEntry<string, int>>>"* %hashTable)
  ret void
}

define i32 @_EN3std3MapI6string3intE4sizeE(%"Map<string, int>"* %this) {
  %size = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std3MapI6string3intE6removeEP6string(%"Map<string, int>"* %this, %string* %e) {
  %hashValue = alloca i32, align 4
  %slot = alloca %"List<MapEntry<string, int>>", align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %2 = call i64 @_EN3std6string4hashE(%string* %e)
  %3 = call i32 @_EN3std11convertHashE6uint64(i64 %2)
  %4 = call i32 @_EN3std3MapI6string3intE8capacityE(%"Map<string, int>"* %this)
  %5 = srem i32 %3, %4
  store i32 %5, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %6 = call %"List<MapEntry<string, int>>"* @_EN3std4ListI4ListI8MapEntryI6string3intEEEixE3int(%"List<List<MapEntry<string, int>>>"* %hashTable, i32 %hashValue.load)
  %.load = load %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %6, align 8
  store %"List<MapEntry<string, int>>" %.load, %"List<MapEntry<string, int>>"* %slot, align 8
  %7 = call i32 @_EN3std4ListI8MapEntryI6string3intEE4sizeE(%"List<MapEntry<string, int>>"* %slot)
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %slot)
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %9 = call i32 @_EN3std4ListI8MapEntryI6string3intEE4sizeE(%"List<MapEntry<string, int>>"* %slot)
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
  %i.load = load i32, i32* %i, align 4
  %13 = call %"MapEntry<string, int>"* @_EN3std4ListI8MapEntryI6string3intEEixE3int(%"List<MapEntry<string, int>>"* %slot, i32 %i.load)
  %key = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %13, i32 0, i32 0
  %key.load = load %string, %string* %key, align 8
  %e.load = load %string, %string* %e, align 8
  %14 = call i1 @_EN3stdeqE6string6string(%string %key.load, %string %e.load)
  br i1 %14, label %if.then1, label %if.else3

loop.increment:                                   ; preds = %if.end4
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %slot)
  ret void

if.then1:                                         ; preds = %loop.body
  %i.load2 = load i32, i32* %i, align 4
  call void @_EN3std4ListI8MapEntryI6string3intEE8removeAtE3int(%"List<MapEntry<string, int>>"* %slot, i32 %i.load2)
  %size = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %15 = add i32 %size.load, -1
  store i32 %15, i32* %size, align 4
  call void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %slot)
  ret void

if.else3:                                         ; preds = %loop.body
  br label %if.end4

if.end4:                                          ; preds = %if.else3
  br label %loop.increment
}

define i32* @_EN3std3MapI6string3intEixEP6string(%"Map<string, int>"* %this, %string* %e) {
  %hashValue = alloca i32, align 4
  %slot = alloca %"List<MapEntry<string, int>>", align 8
  %__iterator = alloca %"ArrayIterator<MapEntry<string, int>>", align 8
  %element = alloca %"MapEntry<string, int>"*, align 8
  %1 = call i64 @_EN3std6string4hashE(%string* %e)
  %2 = call i32 @_EN3std11convertHashE6uint64(i64 %1)
  %3 = call i32 @_EN3std3MapI6string3intE8capacityE(%"Map<string, int>"* %this)
  %4 = srem i32 %2, %3
  store i32 %4, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %5 = call %"List<MapEntry<string, int>>"* @_EN3std4ListI4ListI8MapEntryI6string3intEEEixE3int(%"List<List<MapEntry<string, int>>>"* %hashTable, i32 %hashValue.load)
  %.load = load %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %5, align 8
  store %"List<MapEntry<string, int>>" %.load, %"List<MapEntry<string, int>>"* %slot, align 8
  %6 = call %"ArrayIterator<MapEntry<string, int>>" @_EN3std4ListI8MapEntryI6string3intEE8iteratorE(%"List<MapEntry<string, int>>"* %slot)
  store %"ArrayIterator<MapEntry<string, int>>" %6, %"ArrayIterator<MapEntry<string, int>>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %7 = call i1 @_EN3std13ArrayIteratorI8MapEntryI6string3intEE8hasValueE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator)
  br i1 %7, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %8 = call %"MapEntry<string, int>"* @_EN3std13ArrayIteratorI8MapEntryI6string3intEE5valueE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator)
  store %"MapEntry<string, int>"* %8, %"MapEntry<string, int>"** %element, align 8
  %element.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %element, align 8
  %key = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %element.load, i32 0, i32 0
  %key.load = load %string, %string* %key, align 8
  %e.load = load %string, %string* %e, align 8
  %9 = call i1 @_EN3stdeqE6string6string(%string %key.load, %string %e.load)
  br i1 %9, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std13ArrayIteratorI8MapEntryI6string3intEE9incrementE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %slot)
  ret i32* null

if.then:                                          ; preds = %loop.body
  call void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %slot)
  %element.load1 = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %element, align 8
  %value = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %element.load1, i32 0, i32 1
  ret i32* %value

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.increment
}

define i1 @_EN3stdeqI3intEEOP3intR3int(i32* %a, i32* %b) {
  %1 = icmp eq i32* %a, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %a.load = load i32, i32* %a, align 4
  %b.load = load i32, i32* %b, align 4
  %2 = icmp eq i32 %a.load, %b.load
  ret i1 %2
}

define void @_EN3std3MapI12StringBuffer3intE4initE(%"Map<StringBuffer, int>"* %this) {
  %1 = alloca %"List<List<MapEntry<StringBuffer, int>>>", align 8
  %size = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %hashTable = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4initE(%"List<List<MapEntry<StringBuffer, int>>>"* %1)
  %.load = load %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %1, align 8
  store %"List<List<MapEntry<StringBuffer, int>>>" %.load, %"List<List<MapEntry<StringBuffer, int>>>"* %hashTable, align 8
  %hashTable1 = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 0
  call void @_EN3std3MapI12StringBuffer3intE17increaseTableSizeEP4ListI4ListI8MapEntryI12StringBuffer3intEEE3int(%"Map<StringBuffer, int>"* %this, %"List<List<MapEntry<StringBuffer, int>>>"* %hashTable1, i32 128)
  ret void
}

declare void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"*, i32, i32)

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"*)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"*)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"*)

declare void @_EN3std12StringBuffer4initE(%StringBuffer*)

declare void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

define i32 @_EN3std3MapI12StringBuffer3intE4sizeE(%"Map<StringBuffer, int>"* %this) {
  %size = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

declare void @_EN3std12StringBuffer4pushE4char(%StringBuffer*, i8)

define void @_EN3std3MapI12StringBuffer3intE6insertE12StringBuffer3int(%"Map<StringBuffer, int>"* %this, %StringBuffer %key, i32 %value) {
  %1 = alloca %StringBuffer, align 8
  %hashValue = alloca i32, align 4
  %2 = alloca %StringBuffer, align 8
  %3 = alloca %"MapEntry<StringBuffer, int>", align 8
  store %StringBuffer %key, %StringBuffer* %1, align 8
  %4 = call i1 @_EN3std3MapI12StringBuffer3intE8containsEP12StringBuffer(%"Map<StringBuffer, int>"* %this, %StringBuffer* %1)
  br i1 %4, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  store %StringBuffer %key, %StringBuffer* %2, align 8
  %5 = call i64 @_EN3std12StringBuffer4hashE(%StringBuffer* %2)
  %6 = call i32 @_EN3std11convertHashE6uint64(i64 %5)
  %7 = call i32 @_EN3std3MapI12StringBuffer3intE8capacityE(%"Map<StringBuffer, int>"* %this)
  %8 = srem i32 %6, %7
  store i32 %8, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %9 = call %"List<MapEntry<StringBuffer, int>>"* @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEEixE3int(%"List<List<MapEntry<StringBuffer, int>>>"* %hashTable, i32 %hashValue.load)
  call void @_EN3std8MapEntryI12StringBuffer3intE4initE12StringBuffer3int(%"MapEntry<StringBuffer, int>"* %3, %StringBuffer %key, i32 %value)
  %.load = load %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %3, align 8
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE4pushE8MapEntryI12StringBuffer3intE(%"List<MapEntry<StringBuffer, int>>"* %9, %"MapEntry<StringBuffer, int>" %.load)
  %size = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %10 = add i32 %size.load, 1
  store i32 %10, i32* %size, align 4
  %11 = call double @_EN3std3MapI12StringBuffer3intE10loadFactorE(%"Map<StringBuffer, int>"* %this)
  %12 = fcmp ogt double %11, 0x3FE51EB85FE0EECB
  br i1 %12, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.end
  call void @_EN3std3MapI12StringBuffer3intE6resizeE(%"Map<StringBuffer, int>"* %this)
  br label %if.end3

if.else2:                                         ; preds = %if.end
  br label %if.end3

if.end3:                                          ; preds = %if.else2, %if.then1
  ret void
}

define void @_EN3std3MapI12StringBuffer3intE6deinitE(%"Map<StringBuffer, int>"* %this) {
  %hashTable = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE6deinitE(%"List<List<MapEntry<StringBuffer, int>>>"* %hashTable)
  ret void
}

define void @_EN3std3MapI6string6stringE4initE(%"Map<string, string>"* %this) {
  %1 = alloca %"List<List<MapEntry<string, string>>>", align 8
  %size = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %hashTable = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4initE(%"List<List<MapEntry<string, string>>>"* %1)
  %.load = load %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %1, align 8
  store %"List<List<MapEntry<string, string>>>" %.load, %"List<List<MapEntry<string, string>>>"* %hashTable, align 8
  %hashTable1 = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 0
  call void @_EN3std3MapI6string6stringE17increaseTableSizeEP4ListI4ListI8MapEntryI6string6stringEEE3int(%"Map<string, string>"* %this, %"List<List<MapEntry<string, string>>>"* %hashTable1, i32 128)
  ret void
}

define i32 @_EN3std3MapI6string6stringE4sizeE(%"Map<string, string>"* %this) {
  %size = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std3MapI6string6stringE6insertE6string6string(%"Map<string, string>"* %this, %string %key, %string %value) {
  %1 = alloca %string, align 8
  %hashValue = alloca i32, align 4
  %2 = alloca %string, align 8
  %3 = alloca %"MapEntry<string, string>", align 8
  store %string %key, %string* %1, align 8
  %4 = call i1 @_EN3std3MapI6string6stringE8containsEP6string(%"Map<string, string>"* %this, %string* %1)
  br i1 %4, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  store %string %key, %string* %2, align 8
  %5 = call i64 @_EN3std6string4hashE(%string* %2)
  %6 = call i32 @_EN3std11convertHashE6uint64(i64 %5)
  %7 = call i32 @_EN3std3MapI6string6stringE8capacityE(%"Map<string, string>"* %this)
  %8 = srem i32 %6, %7
  store i32 %8, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %9 = call %"List<MapEntry<string, string>>"* @_EN3std4ListI4ListI8MapEntryI6string6stringEEEixE3int(%"List<List<MapEntry<string, string>>>"* %hashTable, i32 %hashValue.load)
  call void @_EN3std8MapEntryI6string6stringE4initE6string6string(%"MapEntry<string, string>"* %3, %string %key, %string %value)
  %.load = load %"MapEntry<string, string>", %"MapEntry<string, string>"* %3, align 8
  call void @_EN3std4ListI8MapEntryI6string6stringEE4pushE8MapEntryI6string6stringE(%"List<MapEntry<string, string>>"* %9, %"MapEntry<string, string>" %.load)
  %size = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %10 = add i32 %size.load, 1
  store i32 %10, i32* %size, align 4
  %11 = call double @_EN3std3MapI6string6stringE10loadFactorE(%"Map<string, string>"* %this)
  %12 = fcmp ogt double %11, 0x3FE51EB85FE0EECB
  br i1 %12, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.end
  call void @_EN3std3MapI6string6stringE6resizeE(%"Map<string, string>"* %this)
  br label %if.end3

if.else2:                                         ; preds = %if.end
  br label %if.end3

if.end3:                                          ; preds = %if.else2, %if.then1
  ret void
}

define void @_EN3std3MapI6string6stringE6deinitE(%"Map<string, string>"* %this) {
  %hashTable = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE6deinitE(%"List<List<MapEntry<string, string>>>"* %hashTable)
  ret void
}

define %"MapIterator<string, string>" @_EN3std3MapI6string6stringE8iteratorE(%"Map<string, string>"* %this) {
  %1 = alloca %"MapIterator<string, string>", align 8
  call void @_EN3std11MapIteratorI6string6stringE4initEP3MapI6string6stringE(%"MapIterator<string, string>"* %1, %"Map<string, string>"* %this)
  %.load = load %"MapIterator<string, string>", %"MapIterator<string, string>"* %1, align 8
  ret %"MapIterator<string, string>" %.load
}

define i1 @_EN3std11MapIteratorI6string6stringE8hasValueE(%"MapIterator<string, string>"* %this) {
  %hashTableIterator = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 0
  %1 = call i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE8hasValueE(%"ArrayIterator<List<MapEntry<string, string>>>"* %hashTableIterator)
  ret i1 %1
}

define %"MapEntry<string, string>"* @_EN3std11MapIteratorI6string6stringE5valueE(%"MapIterator<string, string>"* %this) {
  %current = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 1
  %current.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %current, align 8
  ret %"MapEntry<string, string>"* %current.load
}

declare i1 @_EN3stdeqE6string6string(%string, %string)

define void @_EN3std11MapIteratorI6string6stringE9incrementE(%"MapIterator<string, string>"* %this) {
  %slot = alloca %"List<MapEntry<string, string>>"*, align 8
  %slotEnd = alloca %"MapEntry<string, string>"*, align 8
  %hashTableIterator = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 0
  %1 = call %"List<MapEntry<string, string>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE5valueE(%"ArrayIterator<List<MapEntry<string, string>>>"* %hashTableIterator)
  store %"List<MapEntry<string, string>>"* %1, %"List<MapEntry<string, string>>"** %slot, align 8
  %slot.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %slot, align 8
  %2 = call %"MapEntry<string, string>"* @_EN3std4ListI8MapEntryI6string6stringEE4dataE(%"List<MapEntry<string, string>>"* %slot.load)
  %slot.load1 = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %slot, align 8
  %3 = call i32 @_EN3std4ListI8MapEntryI6string6stringEE4sizeE(%"List<MapEntry<string, string>>"* %slot.load1)
  %4 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %2, i32 %3
  store %"MapEntry<string, string>"* %4, %"MapEntry<string, string>"** %slotEnd, align 8
  %current = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 1
  %current.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %current, align 8
  %5 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %current.load, i32 1
  store %"MapEntry<string, string>"* %5, %"MapEntry<string, string>"** %current, align 8
  %current2 = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 1
  %current.load3 = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %current2, align 8
  %6 = bitcast %"MapEntry<string, string>"* %current.load3 to i8*
  %slotEnd.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %slotEnd, align 8
  %7 = bitcast %"MapEntry<string, string>"* %slotEnd.load to i8*
  %8 = icmp eq i8* %6, %7
  br i1 %8, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %hashTableIterator4 = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 0
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE9incrementE(%"ArrayIterator<List<MapEntry<string, string>>>"* %hashTableIterator4)
  call void @_EN3std11MapIteratorI6string6stringE14skipEmptySlotsE(%"MapIterator<string, string>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std3MapI6string6stringE6removeEP6string(%"Map<string, string>"* %this, %string* %e) {
  %hashValue = alloca i32, align 4
  %slot = alloca %"List<MapEntry<string, string>>", align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %2 = call i64 @_EN3std6string4hashE(%string* %e)
  %3 = call i32 @_EN3std11convertHashE6uint64(i64 %2)
  %4 = call i32 @_EN3std3MapI6string6stringE8capacityE(%"Map<string, string>"* %this)
  %5 = srem i32 %3, %4
  store i32 %5, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %6 = call %"List<MapEntry<string, string>>"* @_EN3std4ListI4ListI8MapEntryI6string6stringEEEixE3int(%"List<List<MapEntry<string, string>>>"* %hashTable, i32 %hashValue.load)
  %.load = load %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %6, align 8
  store %"List<MapEntry<string, string>>" %.load, %"List<MapEntry<string, string>>"* %slot, align 8
  %7 = call i32 @_EN3std4ListI8MapEntryI6string6stringEE4sizeE(%"List<MapEntry<string, string>>"* %slot)
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string6stringEE6deinitE(%"List<MapEntry<string, string>>"* %slot)
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %9 = call i32 @_EN3std4ListI8MapEntryI6string6stringEE4sizeE(%"List<MapEntry<string, string>>"* %slot)
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
  %i.load = load i32, i32* %i, align 4
  %13 = call %"MapEntry<string, string>"* @_EN3std4ListI8MapEntryI6string6stringEEixE3int(%"List<MapEntry<string, string>>"* %slot, i32 %i.load)
  %key = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %13, i32 0, i32 0
  %key.load = load %string, %string* %key, align 8
  %e.load = load %string, %string* %e, align 8
  %14 = call i1 @_EN3stdeqE6string6string(%string %key.load, %string %e.load)
  br i1 %14, label %if.then1, label %if.else3

loop.increment:                                   ; preds = %if.end4
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std4ListI8MapEntryI6string6stringEE6deinitE(%"List<MapEntry<string, string>>"* %slot)
  ret void

if.then1:                                         ; preds = %loop.body
  %i.load2 = load i32, i32* %i, align 4
  call void @_EN3std4ListI8MapEntryI6string6stringEE8removeAtE3int(%"List<MapEntry<string, string>>"* %slot, i32 %i.load2)
  %size = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %15 = add i32 %size.load, -1
  store i32 %15, i32* %size, align 4
  call void @_EN3std4ListI8MapEntryI6string6stringEE6deinitE(%"List<MapEntry<string, string>>"* %slot)
  ret void

if.else3:                                         ; preds = %loop.body
  br label %if.end4

if.end4:                                          ; preds = %if.else3
  br label %loop.increment
}

define void @_EN3std4ListI4ListI8MapEntryI6string3intEEE6deinitE(%"List<List<MapEntry<string, int>>>"* %this) {
  %__iterator = alloca %"ArrayIterator<List<MapEntry<string, int>>>", align 8
  %element = alloca %"List<MapEntry<string, int>>"*, align 8
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<List<MapEntry<string, int>>>" @_EN3std4ListI4ListI8MapEntryI6string3intEEE8iteratorE(%"List<List<MapEntry<string, int>>>"* %this)
  store %"ArrayIterator<List<MapEntry<string, int>>>" %2, %"ArrayIterator<List<MapEntry<string, int>>>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI6string3intEEE8hasValueE(%"ArrayIterator<List<MapEntry<string, int>>>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call %"List<MapEntry<string, int>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string3intEEE5valueE(%"ArrayIterator<List<MapEntry<string, int>>>"* %__iterator)
  store %"List<MapEntry<string, int>>"* %4, %"List<MapEntry<string, int>>"** %element, align 8
  %element.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %element, align 8
  call void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %element.load)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string3intEEE9incrementE(%"ArrayIterator<List<MapEntry<string, int>>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %buffer, align 8
  call void @_EN3std10deallocateIAU_4ListI8MapEntryI6string3intEEEEAU_4ListI8MapEntryI6string3intEE(%"List<MapEntry<string, int>>"* %buffer.load)
  br label %if.end
}

define void @_EN3std4ListI4ListI8MapEntryI6string3intEEE4initE(%"List<List<MapEntry<string, int>>>"* %this) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

define void @_EN3std3MapI6string3intE17increaseTableSizeEP4ListI4ListI8MapEntryI6string3intEEE3int(%"Map<string, int>"* %this, %"List<List<MapEntry<string, int>>>"* %newTable, i32 %newCapacity) {
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %2 = alloca %"List<MapEntry<string, int>>", align 8
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
  call void @_EN3std4ListI8MapEntryI6string3intEE4initE(%"List<MapEntry<string, int>>"* %2)
  %.load = load %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %2, align 8
  call void @_EN3std4ListI4ListI8MapEntryI6string3intEEE4pushE4ListI8MapEntryI6string3intEE(%"List<List<MapEntry<string, int>>>"* %newTable, %"List<MapEntry<string, int>>" %.load)
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

define i32 @_EN3std3MapI6string3intE8capacityE(%"Map<string, int>"* %this) {
  %hashTable = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 0
  %1 = call i32 @_EN3std4ListI4ListI8MapEntryI6string3intEEE4sizeE(%"List<List<MapEntry<string, int>>>"* %hashTable)
  ret i32 %1
}

define %"List<MapEntry<string, int>>"* @_EN3std4ListI4ListI8MapEntryI6string3intEEEixE3int(%"List<List<MapEntry<string, int>>>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI6string3intEEE16indexOutOfBoundsE3int(%"List<List<MapEntry<string, int>>>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %buffer, align 8
  %2 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %buffer.load, i32 %index
  ret %"List<MapEntry<string, int>>"* %2
}

define void @_EN3std8MapEntryI6string3intE4initE6string3int(%"MapEntry<string, int>"* %this, %string %key, i32 %value) {
  %key1 = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %this, i32 0, i32 0
  store %string %key, %string* %key1, align 8
  %value2 = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %this, i32 0, i32 1
  store i32 %value, i32* %value2, align 4
  ret void
}

define void @_EN3std4ListI8MapEntryI6string3intEE4pushE8MapEntryI6string3intE(%"List<MapEntry<string, int>>"* %this, %"MapEntry<string, int>" %element) {
  %size = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string3intEE4growE(%"List<MapEntry<string, int>>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %buffer, align 8
  %size1 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 1
  %size.load2 = load i32, i32* %size1, align 4
  %2 = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %buffer.load, i32 %size.load2
  store %"MapEntry<string, int>" %element, %"MapEntry<string, int>"* %2, align 8
  %size3 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 1
  %size.load4 = load i32, i32* %size3, align 4
  %3 = add i32 %size.load4, 1
  store i32 %3, i32* %size3, align 4
  ret void
}

define double @_EN3std3MapI6string3intE10loadFactorE(%"Map<string, int>"* %this) {
  %size = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = sitofp i32 %size.load to double
  %2 = call i32 @_EN3std3MapI6string3intE8capacityE(%"Map<string, int>"* %this)
  %3 = sitofp i32 %2 to double
  %4 = fdiv double %1, %3
  ret double %4
}

define void @_EN3std3MapI6string3intE6resizeE(%"Map<string, int>"* %this) {
  %newTable = alloca %"List<List<MapEntry<string, int>>>", align 8
  %newCapacity = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %slot = alloca %"List<MapEntry<string, int>>", align 8
  %__iterator1 = alloca %"ArrayIterator<MapEntry<string, int>>", align 8
  %el = alloca %"MapEntry<string, int>"*, align 8
  call void @_EN3std4ListI4ListI8MapEntryI6string3intEEE4initE(%"List<List<MapEntry<string, int>>>"* %newTable)
  %2 = call i32 @_EN3std3MapI6string3intE8capacityE(%"Map<string, int>"* %this)
  %3 = mul i32 %2, 2
  store i32 %3, i32* %newCapacity, align 4
  %newCapacity.load = load i32, i32* %newCapacity, align 4
  call void @_EN3std3MapI6string3intE17increaseTableSizeEP4ListI4ListI8MapEntryI6string3intEEE3int(%"Map<string, int>"* %this, %"List<List<MapEntry<string, int>>>"* %newTable, i32 %newCapacity.load)
  %4 = call i32 @_EN3std3MapI6string3intE8capacityE(%"Map<string, int>"* %this)
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
  %hashTable = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 0
  %i.load = load i32, i32* %i, align 4
  %8 = call %"List<MapEntry<string, int>>"* @_EN3std4ListI4ListI8MapEntryI6string3intEEEixE3int(%"List<List<MapEntry<string, int>>>"* %hashTable, i32 %i.load)
  %.load = load %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %8, align 8
  store %"List<MapEntry<string, int>>" %.load, %"List<MapEntry<string, int>>"* %slot, align 8
  %9 = call %"ArrayIterator<MapEntry<string, int>>" @_EN3std4ListI8MapEntryI6string3intEE8iteratorE(%"List<MapEntry<string, int>>"* %slot)
  store %"ArrayIterator<MapEntry<string, int>>" %9, %"ArrayIterator<MapEntry<string, int>>"* %__iterator1, align 8
  br label %loop.condition2

loop.increment:                                   ; preds = %loop.end7
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %hashTable1 = getelementptr inbounds %"Map<string, int>", %"Map<string, int>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI6string3intEEE6deinitE(%"List<List<MapEntry<string, int>>>"* %hashTable1)
  %newTable.load = load %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %newTable, align 8
  store %"List<List<MapEntry<string, int>>>" %newTable.load, %"List<List<MapEntry<string, int>>>"* %hashTable1, align 8
  ret void

loop.condition2:                                  ; preds = %loop.increment6, %loop.body
  %10 = call i1 @_EN3std13ArrayIteratorI8MapEntryI6string3intEE8hasValueE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator1)
  br i1 %10, label %loop.body3, label %loop.end7

loop.body3:                                       ; preds = %loop.condition2
  %11 = call %"MapEntry<string, int>"* @_EN3std13ArrayIteratorI8MapEntryI6string3intEE5valueE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator1)
  store %"MapEntry<string, int>"* %11, %"MapEntry<string, int>"** %el, align 8
  %el.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %el, align 8
  %key = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %el.load, i32 0, i32 0
  %12 = call i64 @_EN3std6string4hashE(%string* %key)
  %13 = call i32 @_EN3std11convertHashE6uint64(i64 %12)
  %newCapacity.load4 = load i32, i32* %newCapacity, align 4
  %14 = srem i32 %13, %newCapacity.load4
  %15 = call %"List<MapEntry<string, int>>"* @_EN3std4ListI4ListI8MapEntryI6string3intEEEixE3int(%"List<List<MapEntry<string, int>>>"* %newTable, i32 %14)
  %el.load5 = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %el, align 8
  %el.load.load = load %"MapEntry<string, int>", %"MapEntry<string, int>"* %el.load5, align 8
  call void @_EN3std4ListI8MapEntryI6string3intEE4pushE8MapEntryI6string3intE(%"List<MapEntry<string, int>>"* %15, %"MapEntry<string, int>" %el.load.load)
  br label %loop.increment6

loop.increment6:                                  ; preds = %loop.body3
  call void @_EN3std13ArrayIteratorI8MapEntryI6string3intEE9incrementE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator1)
  br label %loop.condition2

loop.end7:                                        ; preds = %loop.condition2
  call void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %slot)
  br label %loop.increment
}

define i32 @_EN3std4ListI8MapEntryI6string3intEE4sizeE(%"List<MapEntry<string, int>>"* %this) {
  %size = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std4ListI8MapEntryI6string3intEE6deinitE(%"List<MapEntry<string, int>>"* %this) {
  %__iterator = alloca %"ArrayIterator<MapEntry<string, int>>", align 8
  %element = alloca %"MapEntry<string, int>"*, align 8
  %capacity = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<MapEntry<string, int>>" @_EN3std4ListI8MapEntryI6string3intEE8iteratorE(%"List<MapEntry<string, int>>"* %this)
  store %"ArrayIterator<MapEntry<string, int>>" %2, %"ArrayIterator<MapEntry<string, int>>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI8MapEntryI6string3intEE8hasValueE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call %"MapEntry<string, int>"* @_EN3std13ArrayIteratorI8MapEntryI6string3intEE5valueE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator)
  store %"MapEntry<string, int>"* %4, %"MapEntry<string, int>"** %element, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI8MapEntryI6string3intEE9incrementE(%"ArrayIterator<MapEntry<string, int>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %buffer, align 8
  call void @_EN3std10deallocateIAU_8MapEntryI6string3intEEEAU_8MapEntryI6string3intE(%"MapEntry<string, int>"* %buffer.load)
  br label %if.end
}

define %"ArrayIterator<MapEntry<string, int>>" @_EN3std4ListI8MapEntryI6string3intEE8iteratorE(%"List<MapEntry<string, int>>"* %this) {
  %1 = alloca %"ArrayIterator<MapEntry<string, int>>", align 8
  %2 = alloca %"ArrayRef<MapEntry<string, int>>", align 8
  call void @_EN3std8ArrayRefI8MapEntryI6string3intEE4initEP4ListI8MapEntryI6string3intEE(%"ArrayRef<MapEntry<string, int>>"* %2, %"List<MapEntry<string, int>>"* %this)
  %.load = load %"ArrayRef<MapEntry<string, int>>", %"ArrayRef<MapEntry<string, int>>"* %2, align 8
  call void @_EN3std13ArrayIteratorI8MapEntryI6string3intEE4initE8ArrayRefI8MapEntryI6string3intEE(%"ArrayIterator<MapEntry<string, int>>"* %1, %"ArrayRef<MapEntry<string, int>>" %.load)
  %.load1 = load %"ArrayIterator<MapEntry<string, int>>", %"ArrayIterator<MapEntry<string, int>>"* %1, align 8
  ret %"ArrayIterator<MapEntry<string, int>>" %.load1
}

define i1 @_EN3std13ArrayIteratorI8MapEntryI6string3intEE8hasValueE(%"ArrayIterator<MapEntry<string, int>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, int>>", %"ArrayIterator<MapEntry<string, int>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<MapEntry<string, int>>", %"ArrayIterator<MapEntry<string, int>>"* %this, i32 0, i32 1
  %end.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %end, align 8
  %1 = icmp ne %"MapEntry<string, int>"* %current.load, %end.load
  ret i1 %1
}

define %"MapEntry<string, int>"* @_EN3std13ArrayIteratorI8MapEntryI6string3intEE5valueE(%"ArrayIterator<MapEntry<string, int>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, int>>", %"ArrayIterator<MapEntry<string, int>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %current, align 8
  ret %"MapEntry<string, int>"* %current.load
}

define void @_EN3std13ArrayIteratorI8MapEntryI6string3intEE9incrementE(%"ArrayIterator<MapEntry<string, int>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, int>>", %"ArrayIterator<MapEntry<string, int>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %current, align 8
  %1 = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %current.load, i32 1
  store %"MapEntry<string, int>"* %1, %"MapEntry<string, int>"** %current, align 8
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE6deinitE(%"List<List<MapEntry<string, string>>>"* %this) {
  %__iterator = alloca %"ArrayIterator<List<MapEntry<string, string>>>", align 8
  %element = alloca %"List<MapEntry<string, string>>"*, align 8
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<List<MapEntry<string, string>>>" @_EN3std4ListI4ListI8MapEntryI6string6stringEEE8iteratorE(%"List<List<MapEntry<string, string>>>"* %this)
  store %"ArrayIterator<List<MapEntry<string, string>>>" %2, %"ArrayIterator<List<MapEntry<string, string>>>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE8hasValueE(%"ArrayIterator<List<MapEntry<string, string>>>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call %"List<MapEntry<string, string>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE5valueE(%"ArrayIterator<List<MapEntry<string, string>>>"* %__iterator)
  store %"List<MapEntry<string, string>>"* %4, %"List<MapEntry<string, string>>"** %element, align 8
  %element.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %element, align 8
  call void @_EN3std4ListI8MapEntryI6string6stringEE6deinitE(%"List<MapEntry<string, string>>"* %element.load)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE9incrementE(%"ArrayIterator<List<MapEntry<string, string>>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %buffer, align 8
  call void @_EN3std10deallocateIAU_4ListI8MapEntryI6string6stringEEEEAU_4ListI8MapEntryI6string6stringEE(%"List<MapEntry<string, string>>"* %buffer.load)
  br label %if.end
}

define void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4initE(%"List<List<MapEntry<string, string>>>"* %this) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

define void @_EN3std3MapI6string6stringE17increaseTableSizeEP4ListI4ListI8MapEntryI6string6stringEEE3int(%"Map<string, string>"* %this, %"List<List<MapEntry<string, string>>>"* %newTable, i32 %newCapacity) {
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %2 = alloca %"List<MapEntry<string, string>>", align 8
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
  call void @_EN3std4ListI8MapEntryI6string6stringEE4initE(%"List<MapEntry<string, string>>"* %2)
  %.load = load %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %2, align 8
  call void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4pushE4ListI8MapEntryI6string6stringEE(%"List<List<MapEntry<string, string>>>"* %newTable, %"List<MapEntry<string, string>>" %.load)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret void
}

define i1 @_EN3std3MapI6string6stringE8containsEP6string(%"Map<string, string>"* %this, %string* %e) {
  %hashValue = alloca i32, align 4
  %slot = alloca %"List<MapEntry<string, string>>", align 8
  %__iterator = alloca %"ArrayIterator<MapEntry<string, string>>", align 8
  %element = alloca %"MapEntry<string, string>"*, align 8
  %1 = call i64 @_EN3std6string4hashE(%string* %e)
  %2 = call i32 @_EN3std11convertHashE6uint64(i64 %1)
  %3 = call i32 @_EN3std3MapI6string6stringE8capacityE(%"Map<string, string>"* %this)
  %4 = srem i32 %2, %3
  store i32 %4, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %5 = call %"List<MapEntry<string, string>>"* @_EN3std4ListI4ListI8MapEntryI6string6stringEEEixE3int(%"List<List<MapEntry<string, string>>>"* %hashTable, i32 %hashValue.load)
  %.load = load %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %5, align 8
  store %"List<MapEntry<string, string>>" %.load, %"List<MapEntry<string, string>>"* %slot, align 8
  %6 = call i32 @_EN3std4ListI8MapEntryI6string6stringEE4sizeE(%"List<MapEntry<string, string>>"* %slot)
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string6stringEE6deinitE(%"List<MapEntry<string, string>>"* %slot)
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %8 = call %"ArrayIterator<MapEntry<string, string>>" @_EN3std4ListI8MapEntryI6string6stringEE8iteratorE(%"List<MapEntry<string, string>>"* %slot)
  store %"ArrayIterator<MapEntry<string, string>>" %8, %"ArrayIterator<MapEntry<string, string>>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %if.end
  %9 = call i1 @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE8hasValueE(%"ArrayIterator<MapEntry<string, string>>"* %__iterator)
  br i1 %9, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %10 = call %"MapEntry<string, string>"* @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE5valueE(%"ArrayIterator<MapEntry<string, string>>"* %__iterator)
  store %"MapEntry<string, string>"* %10, %"MapEntry<string, string>"** %element, align 8
  %element.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %element, align 8
  %key = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %element.load, i32 0, i32 0
  %key.load = load %string, %string* %key, align 8
  %e.load = load %string, %string* %e, align 8
  %11 = call i1 @_EN3stdeqE6string6string(%string %key.load, %string %e.load)
  br i1 %11, label %if.then1, label %if.else2

loop.increment:                                   ; preds = %if.end3
  call void @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE9incrementE(%"ArrayIterator<MapEntry<string, string>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std4ListI8MapEntryI6string6stringEE6deinitE(%"List<MapEntry<string, string>>"* %slot)
  ret i1 false

if.then1:                                         ; preds = %loop.body
  call void @_EN3std4ListI8MapEntryI6string6stringEE6deinitE(%"List<MapEntry<string, string>>"* %slot)
  ret i1 true

if.else2:                                         ; preds = %loop.body
  br label %if.end3

if.end3:                                          ; preds = %if.else2
  br label %loop.increment
}

define i32 @_EN3std3MapI6string6stringE8capacityE(%"Map<string, string>"* %this) {
  %hashTable = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 0
  %1 = call i32 @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4sizeE(%"List<List<MapEntry<string, string>>>"* %hashTable)
  ret i32 %1
}

define %"List<MapEntry<string, string>>"* @_EN3std4ListI4ListI8MapEntryI6string6stringEEEixE3int(%"List<List<MapEntry<string, string>>>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE16indexOutOfBoundsE3int(%"List<List<MapEntry<string, string>>>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %buffer, align 8
  %2 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %buffer.load, i32 %index
  ret %"List<MapEntry<string, string>>"* %2
}

define void @_EN3std8MapEntryI6string6stringE4initE6string6string(%"MapEntry<string, string>"* %this, %string %key, %string %value) {
  %key1 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %this, i32 0, i32 0
  store %string %key, %string* %key1, align 8
  %value2 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %this, i32 0, i32 1
  store %string %value, %string* %value2, align 8
  ret void
}

define void @_EN3std4ListI8MapEntryI6string6stringEE4pushE8MapEntryI6string6stringE(%"List<MapEntry<string, string>>"* %this, %"MapEntry<string, string>" %element) {
  %size = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string6stringEE4growE(%"List<MapEntry<string, string>>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %buffer, align 8
  %size1 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 1
  %size.load2 = load i32, i32* %size1, align 4
  %2 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %buffer.load, i32 %size.load2
  store %"MapEntry<string, string>" %element, %"MapEntry<string, string>"* %2, align 8
  %size3 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 1
  %size.load4 = load i32, i32* %size3, align 4
  %3 = add i32 %size.load4, 1
  store i32 %3, i32* %size3, align 4
  ret void
}

define double @_EN3std3MapI6string6stringE10loadFactorE(%"Map<string, string>"* %this) {
  %size = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = sitofp i32 %size.load to double
  %2 = call i32 @_EN3std3MapI6string6stringE8capacityE(%"Map<string, string>"* %this)
  %3 = sitofp i32 %2 to double
  %4 = fdiv double %1, %3
  ret double %4
}

define void @_EN3std3MapI6string6stringE6resizeE(%"Map<string, string>"* %this) {
  %newTable = alloca %"List<List<MapEntry<string, string>>>", align 8
  %newCapacity = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %slot = alloca %"List<MapEntry<string, string>>", align 8
  %__iterator1 = alloca %"ArrayIterator<MapEntry<string, string>>", align 8
  %el = alloca %"MapEntry<string, string>"*, align 8
  call void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4initE(%"List<List<MapEntry<string, string>>>"* %newTable)
  %2 = call i32 @_EN3std3MapI6string6stringE8capacityE(%"Map<string, string>"* %this)
  %3 = mul i32 %2, 2
  store i32 %3, i32* %newCapacity, align 4
  %newCapacity.load = load i32, i32* %newCapacity, align 4
  call void @_EN3std3MapI6string6stringE17increaseTableSizeEP4ListI4ListI8MapEntryI6string6stringEEE3int(%"Map<string, string>"* %this, %"List<List<MapEntry<string, string>>>"* %newTable, i32 %newCapacity.load)
  %4 = call i32 @_EN3std3MapI6string6stringE8capacityE(%"Map<string, string>"* %this)
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
  %hashTable = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 0
  %i.load = load i32, i32* %i, align 4
  %8 = call %"List<MapEntry<string, string>>"* @_EN3std4ListI4ListI8MapEntryI6string6stringEEEixE3int(%"List<List<MapEntry<string, string>>>"* %hashTable, i32 %i.load)
  %.load = load %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %8, align 8
  store %"List<MapEntry<string, string>>" %.load, %"List<MapEntry<string, string>>"* %slot, align 8
  %9 = call %"ArrayIterator<MapEntry<string, string>>" @_EN3std4ListI8MapEntryI6string6stringEE8iteratorE(%"List<MapEntry<string, string>>"* %slot)
  store %"ArrayIterator<MapEntry<string, string>>" %9, %"ArrayIterator<MapEntry<string, string>>"* %__iterator1, align 8
  br label %loop.condition2

loop.increment:                                   ; preds = %loop.end7
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %hashTable1 = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE6deinitE(%"List<List<MapEntry<string, string>>>"* %hashTable1)
  %newTable.load = load %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %newTable, align 8
  store %"List<List<MapEntry<string, string>>>" %newTable.load, %"List<List<MapEntry<string, string>>>"* %hashTable1, align 8
  ret void

loop.condition2:                                  ; preds = %loop.increment6, %loop.body
  %10 = call i1 @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE8hasValueE(%"ArrayIterator<MapEntry<string, string>>"* %__iterator1)
  br i1 %10, label %loop.body3, label %loop.end7

loop.body3:                                       ; preds = %loop.condition2
  %11 = call %"MapEntry<string, string>"* @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE5valueE(%"ArrayIterator<MapEntry<string, string>>"* %__iterator1)
  store %"MapEntry<string, string>"* %11, %"MapEntry<string, string>"** %el, align 8
  %el.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %el, align 8
  %key = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %el.load, i32 0, i32 0
  %12 = call i64 @_EN3std6string4hashE(%string* %key)
  %13 = call i32 @_EN3std11convertHashE6uint64(i64 %12)
  %newCapacity.load4 = load i32, i32* %newCapacity, align 4
  %14 = srem i32 %13, %newCapacity.load4
  %15 = call %"List<MapEntry<string, string>>"* @_EN3std4ListI4ListI8MapEntryI6string6stringEEEixE3int(%"List<List<MapEntry<string, string>>>"* %newTable, i32 %14)
  %el.load5 = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %el, align 8
  %el.load.load = load %"MapEntry<string, string>", %"MapEntry<string, string>"* %el.load5, align 8
  call void @_EN3std4ListI8MapEntryI6string6stringEE4pushE8MapEntryI6string6stringE(%"List<MapEntry<string, string>>"* %15, %"MapEntry<string, string>" %el.load.load)
  br label %loop.increment6

loop.increment6:                                  ; preds = %loop.body3
  call void @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE9incrementE(%"ArrayIterator<MapEntry<string, string>>"* %__iterator1)
  br label %loop.condition2

loop.end7:                                        ; preds = %loop.condition2
  call void @_EN3std4ListI8MapEntryI6string6stringEE6deinitE(%"List<MapEntry<string, string>>"* %slot)
  br label %loop.increment
}

define void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE6deinitE(%"List<List<MapEntry<StringBuffer, int>>>"* %this) {
  %__iterator = alloca %"ArrayIterator<List<MapEntry<StringBuffer, int>>>", align 8
  %element = alloca %"List<MapEntry<StringBuffer, int>>"*, align 8
  %capacity = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<List<MapEntry<StringBuffer, int>>>" @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE8iteratorE(%"List<List<MapEntry<StringBuffer, int>>>"* %this)
  store %"ArrayIterator<List<MapEntry<StringBuffer, int>>>" %2, %"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI12StringBuffer3intEEE8hasValueE(%"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call %"List<MapEntry<StringBuffer, int>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI12StringBuffer3intEEE5valueE(%"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %__iterator)
  store %"List<MapEntry<StringBuffer, int>>"* %4, %"List<MapEntry<StringBuffer, int>>"** %element, align 8
  %element.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %element, align 8
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE6deinitE(%"List<MapEntry<StringBuffer, int>>"* %element.load)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI12StringBuffer3intEEE9incrementE(%"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %buffer, align 8
  call void @_EN3std10deallocateIAU_4ListI8MapEntryI12StringBuffer3intEEEEAU_4ListI8MapEntryI12StringBuffer3intEE(%"List<MapEntry<StringBuffer, int>>"* %buffer.load)
  br label %if.end
}

define void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4initE(%"List<List<MapEntry<StringBuffer, int>>>"* %this) {
  %size = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

define void @_EN3std3MapI12StringBuffer3intE17increaseTableSizeEP4ListI4ListI8MapEntryI12StringBuffer3intEEE3int(%"Map<StringBuffer, int>"* %this, %"List<List<MapEntry<StringBuffer, int>>>"* %newTable, i32 %newCapacity) {
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %2 = alloca %"List<MapEntry<StringBuffer, int>>", align 8
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
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE4initE(%"List<MapEntry<StringBuffer, int>>"* %2)
  %.load = load %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %2, align 8
  call void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4pushE4ListI8MapEntryI12StringBuffer3intEE(%"List<List<MapEntry<StringBuffer, int>>>"* %newTable, %"List<MapEntry<StringBuffer, int>>" %.load)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret void
}

declare void @_EN3std12StringBuffer6deinitE(%StringBuffer*)

define i1 @_EN3std3MapI12StringBuffer3intE8containsEP12StringBuffer(%"Map<StringBuffer, int>"* %this, %StringBuffer* %e) {
  %hashValue = alloca i32, align 4
  %slot = alloca %"List<MapEntry<StringBuffer, int>>", align 8
  %__iterator = alloca %"ArrayIterator<MapEntry<StringBuffer, int>>", align 8
  %element = alloca %"MapEntry<StringBuffer, int>"*, align 8
  %1 = call i64 @_EN3std12StringBuffer4hashE(%StringBuffer* %e)
  %2 = call i32 @_EN3std11convertHashE6uint64(i64 %1)
  %3 = call i32 @_EN3std3MapI12StringBuffer3intE8capacityE(%"Map<StringBuffer, int>"* %this)
  %4 = srem i32 %2, %3
  store i32 %4, i32* %hashValue, align 4
  %hashTable = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 0
  %hashValue.load = load i32, i32* %hashValue, align 4
  %5 = call %"List<MapEntry<StringBuffer, int>>"* @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEEixE3int(%"List<List<MapEntry<StringBuffer, int>>>"* %hashTable, i32 %hashValue.load)
  %.load = load %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %5, align 8
  store %"List<MapEntry<StringBuffer, int>>" %.load, %"List<MapEntry<StringBuffer, int>>"* %slot, align 8
  %6 = call i32 @_EN3std4ListI8MapEntryI12StringBuffer3intEE4sizeE(%"List<MapEntry<StringBuffer, int>>"* %slot)
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE6deinitE(%"List<MapEntry<StringBuffer, int>>"* %slot)
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %8 = call %"ArrayIterator<MapEntry<StringBuffer, int>>" @_EN3std4ListI8MapEntryI12StringBuffer3intEE8iteratorE(%"List<MapEntry<StringBuffer, int>>"* %slot)
  store %"ArrayIterator<MapEntry<StringBuffer, int>>" %8, %"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %if.end
  %9 = call i1 @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE8hasValueE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator)
  br i1 %9, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %10 = call %"MapEntry<StringBuffer, int>"* @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE5valueE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator)
  store %"MapEntry<StringBuffer, int>"* %10, %"MapEntry<StringBuffer, int>"** %element, align 8
  %element.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %element, align 8
  %key = getelementptr inbounds %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %element.load, i32 0, i32 0
  %11 = call i1 @_EN3stdeqER12StringBufferR12StringBuffer(%StringBuffer* %key, %StringBuffer* %e)
  br i1 %11, label %if.then1, label %if.else2

loop.increment:                                   ; preds = %if.end3
  call void @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE9incrementE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE6deinitE(%"List<MapEntry<StringBuffer, int>>"* %slot)
  ret i1 false

if.then1:                                         ; preds = %loop.body
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE6deinitE(%"List<MapEntry<StringBuffer, int>>"* %slot)
  ret i1 true

if.else2:                                         ; preds = %loop.body
  br label %if.end3

if.end3:                                          ; preds = %if.else2
  br label %loop.increment
}

define i64 @_EN3std12StringBuffer4hashE(%StringBuffer* %this) {
  %1 = alloca %string, align 8
  call void @_EN3std6string4initEP12StringBuffer(%string* %1, %StringBuffer* %this)
  %2 = call i64 @_EN3std6string4hashE(%string* %1)
  ret i64 %2
}

define i32 @_EN3std3MapI12StringBuffer3intE8capacityE(%"Map<StringBuffer, int>"* %this) {
  %hashTable = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 0
  %1 = call i32 @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4sizeE(%"List<List<MapEntry<StringBuffer, int>>>"* %hashTable)
  ret i32 %1
}

define %"List<MapEntry<StringBuffer, int>>"* @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEEixE3int(%"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE16indexOutOfBoundsE3int(%"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %buffer, align 8
  %2 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %buffer.load, i32 %index
  ret %"List<MapEntry<StringBuffer, int>>"* %2
}

define void @_EN3std8MapEntryI12StringBuffer3intE4initE12StringBuffer3int(%"MapEntry<StringBuffer, int>"* %this, %StringBuffer %key, i32 %value) {
  %key1 = getelementptr inbounds %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %this, i32 0, i32 0
  store %StringBuffer %key, %StringBuffer* %key1, align 8
  %value2 = getelementptr inbounds %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %this, i32 0, i32 1
  store i32 %value, i32* %value2, align 4
  ret void
}

define void @_EN3std4ListI8MapEntryI12StringBuffer3intEE4pushE8MapEntryI12StringBuffer3intE(%"List<MapEntry<StringBuffer, int>>"* %this, %"MapEntry<StringBuffer, int>" %element) {
  %size = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE4growE(%"List<MapEntry<StringBuffer, int>>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %buffer, align 8
  %size1 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 1
  %size.load2 = load i32, i32* %size1, align 4
  %2 = getelementptr inbounds %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %buffer.load, i32 %size.load2
  store %"MapEntry<StringBuffer, int>" %element, %"MapEntry<StringBuffer, int>"* %2, align 8
  %size3 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 1
  %size.load4 = load i32, i32* %size3, align 4
  %3 = add i32 %size.load4, 1
  store i32 %3, i32* %size3, align 4
  ret void
}

define double @_EN3std3MapI12StringBuffer3intE10loadFactorE(%"Map<StringBuffer, int>"* %this) {
  %size = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = sitofp i32 %size.load to double
  %2 = call i32 @_EN3std3MapI12StringBuffer3intE8capacityE(%"Map<StringBuffer, int>"* %this)
  %3 = sitofp i32 %2 to double
  %4 = fdiv double %1, %3
  ret double %4
}

define void @_EN3std3MapI12StringBuffer3intE6resizeE(%"Map<StringBuffer, int>"* %this) {
  %newTable = alloca %"List<List<MapEntry<StringBuffer, int>>>", align 8
  %newCapacity = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %slot = alloca %"List<MapEntry<StringBuffer, int>>", align 8
  %__iterator1 = alloca %"ArrayIterator<MapEntry<StringBuffer, int>>", align 8
  %el = alloca %"MapEntry<StringBuffer, int>"*, align 8
  call void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4initE(%"List<List<MapEntry<StringBuffer, int>>>"* %newTable)
  %2 = call i32 @_EN3std3MapI12StringBuffer3intE8capacityE(%"Map<StringBuffer, int>"* %this)
  %3 = mul i32 %2, 2
  store i32 %3, i32* %newCapacity, align 4
  %newCapacity.load = load i32, i32* %newCapacity, align 4
  call void @_EN3std3MapI12StringBuffer3intE17increaseTableSizeEP4ListI4ListI8MapEntryI12StringBuffer3intEEE3int(%"Map<StringBuffer, int>"* %this, %"List<List<MapEntry<StringBuffer, int>>>"* %newTable, i32 %newCapacity.load)
  %4 = call i32 @_EN3std3MapI12StringBuffer3intE8capacityE(%"Map<StringBuffer, int>"* %this)
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
  %hashTable = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 0
  %i.load = load i32, i32* %i, align 4
  %8 = call %"List<MapEntry<StringBuffer, int>>"* @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEEixE3int(%"List<List<MapEntry<StringBuffer, int>>>"* %hashTable, i32 %i.load)
  %.load = load %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %8, align 8
  store %"List<MapEntry<StringBuffer, int>>" %.load, %"List<MapEntry<StringBuffer, int>>"* %slot, align 8
  %9 = call %"ArrayIterator<MapEntry<StringBuffer, int>>" @_EN3std4ListI8MapEntryI12StringBuffer3intEE8iteratorE(%"List<MapEntry<StringBuffer, int>>"* %slot)
  store %"ArrayIterator<MapEntry<StringBuffer, int>>" %9, %"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator1, align 8
  br label %loop.condition2

loop.increment:                                   ; preds = %loop.end7
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %hashTable1 = getelementptr inbounds %"Map<StringBuffer, int>", %"Map<StringBuffer, int>"* %this, i32 0, i32 0
  call void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE6deinitE(%"List<List<MapEntry<StringBuffer, int>>>"* %hashTable1)
  %newTable.load = load %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %newTable, align 8
  store %"List<List<MapEntry<StringBuffer, int>>>" %newTable.load, %"List<List<MapEntry<StringBuffer, int>>>"* %hashTable1, align 8
  ret void

loop.condition2:                                  ; preds = %loop.increment6, %loop.body
  %10 = call i1 @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE8hasValueE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator1)
  br i1 %10, label %loop.body3, label %loop.end7

loop.body3:                                       ; preds = %loop.condition2
  %11 = call %"MapEntry<StringBuffer, int>"* @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE5valueE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator1)
  store %"MapEntry<StringBuffer, int>"* %11, %"MapEntry<StringBuffer, int>"** %el, align 8
  %el.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %el, align 8
  %key = getelementptr inbounds %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %el.load, i32 0, i32 0
  %12 = call i64 @_EN3std12StringBuffer4hashE(%StringBuffer* %key)
  %13 = call i32 @_EN3std11convertHashE6uint64(i64 %12)
  %newCapacity.load4 = load i32, i32* %newCapacity, align 4
  %14 = srem i32 %13, %newCapacity.load4
  %15 = call %"List<MapEntry<StringBuffer, int>>"* @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEEixE3int(%"List<List<MapEntry<StringBuffer, int>>>"* %newTable, i32 %14)
  %el.load5 = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %el, align 8
  %el.load.load = load %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %el.load5, align 8
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE4pushE8MapEntryI12StringBuffer3intE(%"List<MapEntry<StringBuffer, int>>"* %15, %"MapEntry<StringBuffer, int>" %el.load.load)
  br label %loop.increment6

loop.increment6:                                  ; preds = %loop.body3
  call void @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE9incrementE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator1)
  br label %loop.condition2

loop.end7:                                        ; preds = %loop.condition2
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE6deinitE(%"List<MapEntry<StringBuffer, int>>"* %slot)
  br label %loop.increment
}

define %"MapEntry<string, int>"* @_EN3std4ListI8MapEntryI6string3intEEixE3int(%"List<MapEntry<string, int>>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string3intEE16indexOutOfBoundsE3int(%"List<MapEntry<string, int>>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %buffer, align 8
  %2 = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %buffer.load, i32 %index
  ret %"MapEntry<string, int>"* %2
}

define void @_EN3std4ListI8MapEntryI6string3intEE8removeAtE3int(%"List<MapEntry<string, int>>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string3intEE16indexOutOfBoundsE3int(%"List<MapEntry<string, int>>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @_EN3std4ListI8MapEntryI6string3intEE14unsafeRemoveAtE3int(%"List<MapEntry<string, int>>"* %this, i32 %index)
  ret void
}

define void @_EN3std11MapIteratorI6string6stringE4initEP3MapI6string6stringE(%"MapIterator<string, string>"* %this, %"Map<string, string>"* %map) {
  %hashTableIterator = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 0
  %hashTable = getelementptr inbounds %"Map<string, string>", %"Map<string, string>"* %map, i32 0, i32 0
  %1 = call %"ArrayIterator<List<MapEntry<string, string>>>" @_EN3std4ListI4ListI8MapEntryI6string6stringEEE8iteratorE(%"List<List<MapEntry<string, string>>>"* %hashTable)
  store %"ArrayIterator<List<MapEntry<string, string>>>" %1, %"ArrayIterator<List<MapEntry<string, string>>>"* %hashTableIterator, align 8
  call void @_EN3std11MapIteratorI6string6stringE14skipEmptySlotsE(%"MapIterator<string, string>"* %this)
  ret void
}

define i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE8hasValueE(%"ArrayIterator<List<MapEntry<string, string>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, string>>>", %"ArrayIterator<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, string>>>", %"ArrayIterator<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
  %end.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %end, align 8
  %1 = icmp ne %"List<MapEntry<string, string>>"* %current.load, %end.load
  ret i1 %1
}

define %"List<MapEntry<string, string>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE5valueE(%"ArrayIterator<List<MapEntry<string, string>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, string>>>", %"ArrayIterator<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %current, align 8
  ret %"List<MapEntry<string, string>>"* %current.load
}

define %"MapEntry<string, string>"* @_EN3std4ListI8MapEntryI6string6stringEE4dataE(%"List<MapEntry<string, string>>"* %this) {
  %buffer = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %buffer, align 8
  ret %"MapEntry<string, string>"* %buffer.load
}

define i32 @_EN3std4ListI8MapEntryI6string6stringEE4sizeE(%"List<MapEntry<string, string>>"* %this) {
  %size = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE9incrementE(%"ArrayIterator<List<MapEntry<string, string>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, string>>>", %"ArrayIterator<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %current, align 8
  %1 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %current.load, i32 1
  store %"List<MapEntry<string, string>>"* %1, %"List<MapEntry<string, string>>"** %current, align 8
  ret void
}

define void @_EN3std11MapIteratorI6string6stringE14skipEmptySlotsE(%"MapIterator<string, string>"* %this) {
  br label %loop.condition

loop.condition:                                   ; preds = %if.end, %0
  %hashTableIterator = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 0
  %1 = call i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE8hasValueE(%"ArrayIterator<List<MapEntry<string, string>>>"* %hashTableIterator)
  br i1 %1, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %hashTableIterator1 = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 0
  %2 = call %"List<MapEntry<string, string>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE5valueE(%"ArrayIterator<List<MapEntry<string, string>>>"* %hashTableIterator1)
  %3 = call i32 @_EN3std4ListI8MapEntryI6string6stringEE4sizeE(%"List<MapEntry<string, string>>"* %2)
  %4 = icmp ne i32 %3, 0
  br i1 %4, label %if.then, label %if.else

loop.end:                                         ; preds = %if.then, %loop.condition
  ret void

if.then:                                          ; preds = %loop.body
  %current = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 1
  %hashTableIterator2 = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 0
  %5 = call %"List<MapEntry<string, string>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE5valueE(%"ArrayIterator<List<MapEntry<string, string>>>"* %hashTableIterator2)
  %6 = call %"MapEntry<string, string>"* @_EN3std4ListI8MapEntryI6string6stringEE4dataE(%"List<MapEntry<string, string>>"* %5)
  store %"MapEntry<string, string>"* %6, %"MapEntry<string, string>"** %current, align 8
  br label %loop.end

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  %hashTableIterator3 = getelementptr inbounds %"MapIterator<string, string>", %"MapIterator<string, string>"* %this, i32 0, i32 0
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE9incrementE(%"ArrayIterator<List<MapEntry<string, string>>>"* %hashTableIterator3)
  br label %loop.condition
}

define void @_EN3std4ListI8MapEntryI6string6stringEE6deinitE(%"List<MapEntry<string, string>>"* %this) {
  %__iterator = alloca %"ArrayIterator<MapEntry<string, string>>", align 8
  %element = alloca %"MapEntry<string, string>"*, align 8
  %capacity = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<MapEntry<string, string>>" @_EN3std4ListI8MapEntryI6string6stringEE8iteratorE(%"List<MapEntry<string, string>>"* %this)
  store %"ArrayIterator<MapEntry<string, string>>" %2, %"ArrayIterator<MapEntry<string, string>>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE8hasValueE(%"ArrayIterator<MapEntry<string, string>>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call %"MapEntry<string, string>"* @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE5valueE(%"ArrayIterator<MapEntry<string, string>>"* %__iterator)
  store %"MapEntry<string, string>"* %4, %"MapEntry<string, string>"** %element, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE9incrementE(%"ArrayIterator<MapEntry<string, string>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %buffer, align 8
  call void @_EN3std10deallocateIAU_8MapEntryI6string6stringEEEAU_8MapEntryI6string6stringE(%"MapEntry<string, string>"* %buffer.load)
  br label %if.end
}

define %"MapEntry<string, string>"* @_EN3std4ListI8MapEntryI6string6stringEEixE3int(%"List<MapEntry<string, string>>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string6stringEE16indexOutOfBoundsE3int(%"List<MapEntry<string, string>>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %buffer, align 8
  %2 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %buffer.load, i32 %index
  ret %"MapEntry<string, string>"* %2
}

define void @_EN3std4ListI8MapEntryI6string6stringEE8removeAtE3int(%"List<MapEntry<string, string>>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string6stringEE16indexOutOfBoundsE3int(%"List<MapEntry<string, string>>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @_EN3std4ListI8MapEntryI6string6stringEE14unsafeRemoveAtE3int(%"List<MapEntry<string, string>>"* %this, i32 %index)
  ret void
}

define %"ArrayIterator<List<MapEntry<string, int>>>" @_EN3std4ListI4ListI8MapEntryI6string3intEEE8iteratorE(%"List<List<MapEntry<string, int>>>"* %this) {
  %1 = alloca %"ArrayIterator<List<MapEntry<string, int>>>", align 8
  %2 = alloca %"ArrayRef<List<MapEntry<string, int>>>", align 8
  call void @_EN3std8ArrayRefI4ListI8MapEntryI6string3intEEE4initEP4ListI4ListI8MapEntryI6string3intEEE(%"ArrayRef<List<MapEntry<string, int>>>"* %2, %"List<List<MapEntry<string, int>>>"* %this)
  %.load = load %"ArrayRef<List<MapEntry<string, int>>>", %"ArrayRef<List<MapEntry<string, int>>>"* %2, align 8
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string3intEEE4initE8ArrayRefI4ListI8MapEntryI6string3intEEE(%"ArrayIterator<List<MapEntry<string, int>>>"* %1, %"ArrayRef<List<MapEntry<string, int>>>" %.load)
  %.load1 = load %"ArrayIterator<List<MapEntry<string, int>>>", %"ArrayIterator<List<MapEntry<string, int>>>"* %1, align 8
  ret %"ArrayIterator<List<MapEntry<string, int>>>" %.load1
}

define i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI6string3intEEE8hasValueE(%"ArrayIterator<List<MapEntry<string, int>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, int>>>", %"ArrayIterator<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, int>>>", %"ArrayIterator<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
  %end.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %end, align 8
  %1 = icmp ne %"List<MapEntry<string, int>>"* %current.load, %end.load
  ret i1 %1
}

define %"List<MapEntry<string, int>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI6string3intEEE5valueE(%"ArrayIterator<List<MapEntry<string, int>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, int>>>", %"ArrayIterator<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %current, align 8
  ret %"List<MapEntry<string, int>>"* %current.load
}

define void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string3intEEE9incrementE(%"ArrayIterator<List<MapEntry<string, int>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, int>>>", %"ArrayIterator<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %current, align 8
  %1 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %current.load, i32 1
  store %"List<MapEntry<string, int>>"* %1, %"List<MapEntry<string, int>>"** %current, align 8
  ret void
}

define void @_EN3std10deallocateIAU_4ListI8MapEntryI6string3intEEEEAU_4ListI8MapEntryI6string3intEE(%"List<MapEntry<string, int>>"* %allocation) {
  %1 = bitcast %"List<MapEntry<string, int>>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define void @_EN3std4ListI8MapEntryI6string3intEE4initE(%"List<MapEntry<string, int>>"* %this) {
  %size = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string3intEEE4pushE4ListI8MapEntryI6string3intEE(%"List<List<MapEntry<string, int>>>"* %this, %"List<MapEntry<string, int>>" %element) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI6string3intEEE4growE(%"List<List<MapEntry<string, int>>>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %buffer, align 8
  %size1 = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
  %size.load2 = load i32, i32* %size1, align 4
  %2 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %buffer.load, i32 %size.load2
  store %"List<MapEntry<string, int>>" %element, %"List<MapEntry<string, int>>"* %2, align 8
  %size3 = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
  %size.load4 = load i32, i32* %size3, align 4
  %3 = add i32 %size.load4, 1
  store i32 %3, i32* %size3, align 4
  ret void
}

declare i32 @_EN3std8ArrayRefI4charE4sizeE(%"ArrayRef<char>"*)

declare i8 @_EN3std6stringixE3int(%string*, i32)

define i32 @_EN3std4ListI4ListI8MapEntryI6string3intEEE4sizeE(%"List<List<MapEntry<string, int>>>"* %this) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std4ListI8MapEntryI6string3intEE4growE(%"List<MapEntry<string, int>>"* %this) {
  %capacity = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string3intEE7reserveE3int(%"List<MapEntry<string, int>>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 2
  %capacity.load2 = load i32, i32* %capacity1, align 4
  %2 = mul i32 %capacity.load2, 2
  call void @_EN3std4ListI8MapEntryI6string3intEE7reserveE3int(%"List<MapEntry<string, int>>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string3intEEE16indexOutOfBoundsE3int(%"List<List<MapEntry<string, int>>>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @68, i32 0, i32 0), i32 11)
  store i32 %index, i32* %1, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @69, i32 0, i32 0), i32 27)
  %3 = call i32 @_EN3std4ListI4ListI8MapEntryI6string3intEEE4sizeE(%"List<List<MapEntry<string, int>>>"* %this)
  store i32 %3, i32* %2, align 4
  %4 = call %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %__str, i32* %1, %string* %__str1, i32* %2)
  ret void
}

define void @_EN3std10deallocateIAU_8MapEntryI6string3intEEEAU_8MapEntryI6string3intE(%"MapEntry<string, int>"* %allocation) {
  %1 = bitcast %"MapEntry<string, int>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define void @_EN3std8ArrayRefI8MapEntryI6string3intEE4initEP4ListI8MapEntryI6string3intEE(%"ArrayRef<MapEntry<string, int>>"* %this, %"List<MapEntry<string, int>>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<MapEntry<string, int>>", %"ArrayRef<MapEntry<string, int>>"* %this, i32 0, i32 0
  %1 = call %"MapEntry<string, int>"* @_EN3std4ListI8MapEntryI6string3intEE4dataE(%"List<MapEntry<string, int>>"* %list)
  store %"MapEntry<string, int>"* %1, %"MapEntry<string, int>"** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<MapEntry<string, int>>", %"ArrayRef<MapEntry<string, int>>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI8MapEntryI6string3intEE4sizeE(%"List<MapEntry<string, int>>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI8MapEntryI6string3intEE4initE8ArrayRefI8MapEntryI6string3intEE(%"ArrayIterator<MapEntry<string, int>>"* %this, %"ArrayRef<MapEntry<string, int>>" %array) {
  %1 = alloca %"ArrayRef<MapEntry<string, int>>", align 8
  %2 = alloca %"ArrayRef<MapEntry<string, int>>", align 8
  %3 = alloca %"ArrayRef<MapEntry<string, int>>", align 8
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, int>>", %"ArrayIterator<MapEntry<string, int>>"* %this, i32 0, i32 0
  store %"ArrayRef<MapEntry<string, int>>" %array, %"ArrayRef<MapEntry<string, int>>"* %1, align 8
  %4 = call %"MapEntry<string, int>"* @_EN3std8ArrayRefI8MapEntryI6string3intEE4dataE(%"ArrayRef<MapEntry<string, int>>"* %1)
  store %"MapEntry<string, int>"* %4, %"MapEntry<string, int>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<MapEntry<string, int>>", %"ArrayIterator<MapEntry<string, int>>"* %this, i32 0, i32 1
  store %"ArrayRef<MapEntry<string, int>>" %array, %"ArrayRef<MapEntry<string, int>>"* %2, align 8
  %5 = call %"MapEntry<string, int>"* @_EN3std8ArrayRefI8MapEntryI6string3intEE4dataE(%"ArrayRef<MapEntry<string, int>>"* %2)
  store %"ArrayRef<MapEntry<string, int>>" %array, %"ArrayRef<MapEntry<string, int>>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI8MapEntryI6string3intEE4sizeE(%"ArrayRef<MapEntry<string, int>>"* %3)
  %7 = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %5, i32 %6
  store %"MapEntry<string, int>"* %7, %"MapEntry<string, int>"** %end, align 8
  ret void
}

define %"ArrayIterator<List<MapEntry<string, string>>>" @_EN3std4ListI4ListI8MapEntryI6string6stringEEE8iteratorE(%"List<List<MapEntry<string, string>>>"* %this) {
  %1 = alloca %"ArrayIterator<List<MapEntry<string, string>>>", align 8
  %2 = alloca %"ArrayRef<List<MapEntry<string, string>>>", align 8
  call void @_EN3std8ArrayRefI4ListI8MapEntryI6string6stringEEE4initEP4ListI4ListI8MapEntryI6string6stringEEE(%"ArrayRef<List<MapEntry<string, string>>>"* %2, %"List<List<MapEntry<string, string>>>"* %this)
  %.load = load %"ArrayRef<List<MapEntry<string, string>>>", %"ArrayRef<List<MapEntry<string, string>>>"* %2, align 8
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE4initE8ArrayRefI4ListI8MapEntryI6string6stringEEE(%"ArrayIterator<List<MapEntry<string, string>>>"* %1, %"ArrayRef<List<MapEntry<string, string>>>" %.load)
  %.load1 = load %"ArrayIterator<List<MapEntry<string, string>>>", %"ArrayIterator<List<MapEntry<string, string>>>"* %1, align 8
  ret %"ArrayIterator<List<MapEntry<string, string>>>" %.load1
}

define void @_EN3std10deallocateIAU_4ListI8MapEntryI6string6stringEEEEAU_4ListI8MapEntryI6string6stringEE(%"List<MapEntry<string, string>>"* %allocation) {
  %1 = bitcast %"List<MapEntry<string, string>>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define void @_EN3std4ListI8MapEntryI6string6stringEE4initE(%"List<MapEntry<string, string>>"* %this) {
  %size = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4pushE4ListI8MapEntryI6string6stringEE(%"List<List<MapEntry<string, string>>>"* %this, %"List<MapEntry<string, string>>" %element) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4growE(%"List<List<MapEntry<string, string>>>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %buffer, align 8
  %size1 = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
  %size.load2 = load i32, i32* %size1, align 4
  %2 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %buffer.load, i32 %size.load2
  store %"List<MapEntry<string, string>>" %element, %"List<MapEntry<string, string>>"* %2, align 8
  %size3 = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
  %size.load4 = load i32, i32* %size3, align 4
  %3 = add i32 %size.load4, 1
  store i32 %3, i32* %size3, align 4
  ret void
}

define %"ArrayIterator<MapEntry<string, string>>" @_EN3std4ListI8MapEntryI6string6stringEE8iteratorE(%"List<MapEntry<string, string>>"* %this) {
  %1 = alloca %"ArrayIterator<MapEntry<string, string>>", align 8
  %2 = alloca %"ArrayRef<MapEntry<string, string>>", align 8
  call void @_EN3std8ArrayRefI8MapEntryI6string6stringEE4initEP4ListI8MapEntryI6string6stringEE(%"ArrayRef<MapEntry<string, string>>"* %2, %"List<MapEntry<string, string>>"* %this)
  %.load = load %"ArrayRef<MapEntry<string, string>>", %"ArrayRef<MapEntry<string, string>>"* %2, align 8
  call void @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE4initE8ArrayRefI8MapEntryI6string6stringEE(%"ArrayIterator<MapEntry<string, string>>"* %1, %"ArrayRef<MapEntry<string, string>>" %.load)
  %.load1 = load %"ArrayIterator<MapEntry<string, string>>", %"ArrayIterator<MapEntry<string, string>>"* %1, align 8
  ret %"ArrayIterator<MapEntry<string, string>>" %.load1
}

define i1 @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE8hasValueE(%"ArrayIterator<MapEntry<string, string>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, string>>", %"ArrayIterator<MapEntry<string, string>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<MapEntry<string, string>>", %"ArrayIterator<MapEntry<string, string>>"* %this, i32 0, i32 1
  %end.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %end, align 8
  %1 = icmp ne %"MapEntry<string, string>"* %current.load, %end.load
  ret i1 %1
}

define %"MapEntry<string, string>"* @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE5valueE(%"ArrayIterator<MapEntry<string, string>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, string>>", %"ArrayIterator<MapEntry<string, string>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %current, align 8
  ret %"MapEntry<string, string>"* %current.load
}

define void @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE9incrementE(%"ArrayIterator<MapEntry<string, string>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, string>>", %"ArrayIterator<MapEntry<string, string>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %current, align 8
  %1 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %current.load, i32 1
  store %"MapEntry<string, string>"* %1, %"MapEntry<string, string>"** %current, align 8
  ret void
}

define i32 @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4sizeE(%"List<List<MapEntry<string, string>>>"* %this) {
  %size = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std4ListI8MapEntryI6string6stringEE4growE(%"List<MapEntry<string, string>>"* %this) {
  %capacity = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI6string6stringEE7reserveE3int(%"List<MapEntry<string, string>>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 2
  %capacity.load2 = load i32, i32* %capacity1, align 4
  %2 = mul i32 %capacity.load2, 2
  call void @_EN3std4ListI8MapEntryI6string6stringEE7reserveE3int(%"List<MapEntry<string, string>>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE16indexOutOfBoundsE3int(%"List<List<MapEntry<string, string>>>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @70, i32 0, i32 0), i32 11)
  store i32 %index, i32* %1, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @71, i32 0, i32 0), i32 27)
  %3 = call i32 @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4sizeE(%"List<List<MapEntry<string, string>>>"* %this)
  store i32 %3, i32* %2, align 4
  %4 = call %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %__str, i32* %1, %string* %__str1, i32* %2)
  ret void
}

define %"ArrayIterator<List<MapEntry<StringBuffer, int>>>" @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE8iteratorE(%"List<List<MapEntry<StringBuffer, int>>>"* %this) {
  %1 = alloca %"ArrayIterator<List<MapEntry<StringBuffer, int>>>", align 8
  %2 = alloca %"ArrayRef<List<MapEntry<StringBuffer, int>>>", align 8
  call void @_EN3std8ArrayRefI4ListI8MapEntryI12StringBuffer3intEEE4initEP4ListI4ListI8MapEntryI12StringBuffer3intEEE(%"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %2, %"List<List<MapEntry<StringBuffer, int>>>"* %this)
  %.load = load %"ArrayRef<List<MapEntry<StringBuffer, int>>>", %"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %2, align 8
  call void @_EN3std13ArrayIteratorI4ListI8MapEntryI12StringBuffer3intEEE4initE8ArrayRefI4ListI8MapEntryI12StringBuffer3intEEE(%"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %1, %"ArrayRef<List<MapEntry<StringBuffer, int>>>" %.load)
  %.load1 = load %"ArrayIterator<List<MapEntry<StringBuffer, int>>>", %"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %1, align 8
  ret %"ArrayIterator<List<MapEntry<StringBuffer, int>>>" %.load1
}

define i1 @_EN3std13ArrayIteratorI4ListI8MapEntryI12StringBuffer3intEEE8hasValueE(%"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<StringBuffer, int>>>", %"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<List<MapEntry<StringBuffer, int>>>", %"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
  %end.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %end, align 8
  %1 = icmp ne %"List<MapEntry<StringBuffer, int>>"* %current.load, %end.load
  ret i1 %1
}

define %"List<MapEntry<StringBuffer, int>>"* @_EN3std13ArrayIteratorI4ListI8MapEntryI12StringBuffer3intEEE5valueE(%"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<StringBuffer, int>>>", %"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %current, align 8
  ret %"List<MapEntry<StringBuffer, int>>"* %current.load
}

define void @_EN3std4ListI8MapEntryI12StringBuffer3intEE6deinitE(%"List<MapEntry<StringBuffer, int>>"* %this) {
  %__iterator = alloca %"ArrayIterator<MapEntry<StringBuffer, int>>", align 8
  %element = alloca %"MapEntry<StringBuffer, int>"*, align 8
  %capacity = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<MapEntry<StringBuffer, int>>" @_EN3std4ListI8MapEntryI12StringBuffer3intEE8iteratorE(%"List<MapEntry<StringBuffer, int>>"* %this)
  store %"ArrayIterator<MapEntry<StringBuffer, int>>" %2, %"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE8hasValueE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call %"MapEntry<StringBuffer, int>"* @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE5valueE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator)
  store %"MapEntry<StringBuffer, int>"* %4, %"MapEntry<StringBuffer, int>"** %element, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE9incrementE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %buffer, align 8
  call void @_EN3std10deallocateIAU_8MapEntryI12StringBuffer3intEEEAU_8MapEntryI12StringBuffer3intE(%"MapEntry<StringBuffer, int>"* %buffer.load)
  br label %if.end
}

define void @_EN3std13ArrayIteratorI4ListI8MapEntryI12StringBuffer3intEEE9incrementE(%"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<StringBuffer, int>>>", %"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %current.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %current, align 8
  %1 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %current.load, i32 1
  store %"List<MapEntry<StringBuffer, int>>"* %1, %"List<MapEntry<StringBuffer, int>>"** %current, align 8
  ret void
}

define void @_EN3std10deallocateIAU_4ListI8MapEntryI12StringBuffer3intEEEEAU_4ListI8MapEntryI12StringBuffer3intEE(%"List<MapEntry<StringBuffer, int>>"* %allocation) {
  %1 = bitcast %"List<MapEntry<StringBuffer, int>>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define void @_EN3std4ListI8MapEntryI12StringBuffer3intEE4initE(%"List<MapEntry<StringBuffer, int>>"* %this) {
  %size = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4pushE4ListI8MapEntryI12StringBuffer3intEE(%"List<List<MapEntry<StringBuffer, int>>>"* %this, %"List<MapEntry<StringBuffer, int>>" %element) {
  %size = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4growE(%"List<List<MapEntry<StringBuffer, int>>>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %buffer, align 8
  %size1 = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
  %size.load2 = load i32, i32* %size1, align 4
  %2 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %buffer.load, i32 %size.load2
  store %"List<MapEntry<StringBuffer, int>>" %element, %"List<MapEntry<StringBuffer, int>>"* %2, align 8
  %size3 = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
  %size.load4 = load i32, i32* %size3, align 4
  %3 = add i32 %size.load4, 1
  store i32 %3, i32* %size3, align 4
  ret void
}

define i32 @_EN3std4ListI8MapEntryI12StringBuffer3intEE4sizeE(%"List<MapEntry<StringBuffer, int>>"* %this) {
  %size = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %"ArrayIterator<MapEntry<StringBuffer, int>>" @_EN3std4ListI8MapEntryI12StringBuffer3intEE8iteratorE(%"List<MapEntry<StringBuffer, int>>"* %this) {
  %1 = alloca %"ArrayIterator<MapEntry<StringBuffer, int>>", align 8
  %2 = alloca %"ArrayRef<MapEntry<StringBuffer, int>>", align 8
  call void @_EN3std8ArrayRefI8MapEntryI12StringBuffer3intEE4initEP4ListI8MapEntryI12StringBuffer3intEE(%"ArrayRef<MapEntry<StringBuffer, int>>"* %2, %"List<MapEntry<StringBuffer, int>>"* %this)
  %.load = load %"ArrayRef<MapEntry<StringBuffer, int>>", %"ArrayRef<MapEntry<StringBuffer, int>>"* %2, align 8
  call void @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE4initE8ArrayRefI8MapEntryI12StringBuffer3intEE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %1, %"ArrayRef<MapEntry<StringBuffer, int>>" %.load)
  %.load1 = load %"ArrayIterator<MapEntry<StringBuffer, int>>", %"ArrayIterator<MapEntry<StringBuffer, int>>"* %1, align 8
  ret %"ArrayIterator<MapEntry<StringBuffer, int>>" %.load1
}

define i1 @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE8hasValueE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<StringBuffer, int>>", %"ArrayIterator<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<MapEntry<StringBuffer, int>>", %"ArrayIterator<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 1
  %end.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %end, align 8
  %1 = icmp ne %"MapEntry<StringBuffer, int>"* %current.load, %end.load
  ret i1 %1
}

define %"MapEntry<StringBuffer, int>"* @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE5valueE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<StringBuffer, int>>", %"ArrayIterator<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %current, align 8
  ret %"MapEntry<StringBuffer, int>"* %current.load
}

declare i1 @_EN3stdeqER12StringBufferR12StringBuffer(%StringBuffer*, %StringBuffer*)

define void @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE9incrementE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<StringBuffer, int>>", %"ArrayIterator<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %current.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %current, align 8
  %1 = getelementptr inbounds %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %current.load, i32 1
  store %"MapEntry<StringBuffer, int>"* %1, %"MapEntry<StringBuffer, int>"** %current, align 8
  ret void
}

declare void @_EN3std6string4initEP12StringBuffer(%string*, %StringBuffer*)

define i32 @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4sizeE(%"List<List<MapEntry<StringBuffer, int>>>"* %this) {
  %size = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define void @_EN3std4ListI8MapEntryI12StringBuffer3intEE4growE(%"List<MapEntry<StringBuffer, int>>"* %this) {
  %capacity = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE7reserveE3int(%"List<MapEntry<StringBuffer, int>>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 2
  %capacity.load2 = load i32, i32* %capacity1, align 4
  %2 = mul i32 %capacity.load2, 2
  call void @_EN3std4ListI8MapEntryI12StringBuffer3intEE7reserveE3int(%"List<MapEntry<StringBuffer, int>>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE16indexOutOfBoundsE3int(%"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @72, i32 0, i32 0), i32 11)
  store i32 %index, i32* %1, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @73, i32 0, i32 0), i32 27)
  %3 = call i32 @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4sizeE(%"List<List<MapEntry<StringBuffer, int>>>"* %this)
  store i32 %3, i32* %2, align 4
  %4 = call %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %__str, i32* %1, %string* %__str1, i32* %2)
  ret void
}

define void @_EN3std4ListI8MapEntryI6string3intEE16indexOutOfBoundsE3int(%"List<MapEntry<string, int>>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @74, i32 0, i32 0), i32 11)
  store i32 %index, i32* %1, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @75, i32 0, i32 0), i32 27)
  %3 = call i32 @_EN3std4ListI8MapEntryI6string3intEE4sizeE(%"List<MapEntry<string, int>>"* %this)
  store i32 %3, i32* %2, align 4
  %4 = call %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %__str, i32* %1, %string* %__str1, i32* %2)
  ret void
}

define void @_EN3std4ListI8MapEntryI6string3intEE14unsafeRemoveAtE3int(%"List<MapEntry<string, int>>"* %this, i32 %index) {
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %source = alloca %"MapEntry<string, int>"*, align 8
  %target = alloca %"MapEntry<string, int>"*, align 8
  %2 = add i32 %index, 1
  %size = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %buffer, align 8
  %i.load = load i32, i32* %i, align 4
  %6 = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %buffer.load, i32 %i.load
  store %"MapEntry<string, int>"* %6, %"MapEntry<string, int>"** %source, align 8
  %buffer1 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 0
  %buffer.load2 = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %buffer1, align 8
  %i.load3 = load i32, i32* %i, align 4
  %7 = sub i32 %i.load3, 1
  %8 = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %buffer.load2, i32 %7
  store %"MapEntry<string, int>"* %8, %"MapEntry<string, int>"** %target, align 8
  %target.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %target, align 8
  %source.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %source, align 8
  %source.load.load = load %"MapEntry<string, int>", %"MapEntry<string, int>"* %source.load, align 8
  store %"MapEntry<string, int>" %source.load.load, %"MapEntry<string, int>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %size4 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 1
  %size.load5 = load i32, i32* %size4, align 4
  %9 = add i32 %size.load5, -1
  store i32 %9, i32* %size4, align 4
  ret void
}

define void @_EN3std10deallocateIAU_8MapEntryI6string6stringEEEAU_8MapEntryI6string6stringE(%"MapEntry<string, string>"* %allocation) {
  %1 = bitcast %"MapEntry<string, string>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define void @_EN3std4ListI8MapEntryI6string6stringEE16indexOutOfBoundsE3int(%"List<MapEntry<string, string>>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @76, i32 0, i32 0), i32 11)
  store i32 %index, i32* %1, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @77, i32 0, i32 0), i32 27)
  %3 = call i32 @_EN3std4ListI8MapEntryI6string6stringEE4sizeE(%"List<MapEntry<string, string>>"* %this)
  store i32 %3, i32* %2, align 4
  %4 = call %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %__str, i32* %1, %string* %__str1, i32* %2)
  ret void
}

define void @_EN3std4ListI8MapEntryI6string6stringEE14unsafeRemoveAtE3int(%"List<MapEntry<string, string>>"* %this, i32 %index) {
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %source = alloca %"MapEntry<string, string>"*, align 8
  %target = alloca %"MapEntry<string, string>"*, align 8
  %2 = add i32 %index, 1
  %size = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %buffer, align 8
  %i.load = load i32, i32* %i, align 4
  %6 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %buffer.load, i32 %i.load
  store %"MapEntry<string, string>"* %6, %"MapEntry<string, string>"** %source, align 8
  %buffer1 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 0
  %buffer.load2 = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %buffer1, align 8
  %i.load3 = load i32, i32* %i, align 4
  %7 = sub i32 %i.load3, 1
  %8 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %buffer.load2, i32 %7
  store %"MapEntry<string, string>"* %8, %"MapEntry<string, string>"** %target, align 8
  %target.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %target, align 8
  %source.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %source, align 8
  %source.load.load = load %"MapEntry<string, string>", %"MapEntry<string, string>"* %source.load, align 8
  store %"MapEntry<string, string>" %source.load.load, %"MapEntry<string, string>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %size4 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 1
  %size.load5 = load i32, i32* %size4, align 4
  %9 = add i32 %size.load5, -1
  store i32 %9, i32* %size4, align 4
  ret void
}

define void @_EN3std8ArrayRefI4ListI8MapEntryI6string3intEEE4initEP4ListI4ListI8MapEntryI6string3intEEE(%"ArrayRef<List<MapEntry<string, int>>>"* %this, %"List<List<MapEntry<string, int>>>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<List<MapEntry<string, int>>>", %"ArrayRef<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %1 = call %"List<MapEntry<string, int>>"* @_EN3std4ListI4ListI8MapEntryI6string3intEEE4dataE(%"List<List<MapEntry<string, int>>>"* %list)
  store %"List<MapEntry<string, int>>"* %1, %"List<MapEntry<string, int>>"** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<List<MapEntry<string, int>>>", %"ArrayRef<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI4ListI8MapEntryI6string3intEEE4sizeE(%"List<List<MapEntry<string, int>>>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string3intEEE4initE8ArrayRefI4ListI8MapEntryI6string3intEEE(%"ArrayIterator<List<MapEntry<string, int>>>"* %this, %"ArrayRef<List<MapEntry<string, int>>>" %array) {
  %1 = alloca %"ArrayRef<List<MapEntry<string, int>>>", align 8
  %2 = alloca %"ArrayRef<List<MapEntry<string, int>>>", align 8
  %3 = alloca %"ArrayRef<List<MapEntry<string, int>>>", align 8
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, int>>>", %"ArrayIterator<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  store %"ArrayRef<List<MapEntry<string, int>>>" %array, %"ArrayRef<List<MapEntry<string, int>>>"* %1, align 8
  %4 = call %"List<MapEntry<string, int>>"* @_EN3std8ArrayRefI4ListI8MapEntryI6string3intEEE4dataE(%"ArrayRef<List<MapEntry<string, int>>>"* %1)
  store %"List<MapEntry<string, int>>"* %4, %"List<MapEntry<string, int>>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, int>>>", %"ArrayIterator<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
  store %"ArrayRef<List<MapEntry<string, int>>>" %array, %"ArrayRef<List<MapEntry<string, int>>>"* %2, align 8
  %5 = call %"List<MapEntry<string, int>>"* @_EN3std8ArrayRefI4ListI8MapEntryI6string3intEEE4dataE(%"ArrayRef<List<MapEntry<string, int>>>"* %2)
  store %"ArrayRef<List<MapEntry<string, int>>>" %array, %"ArrayRef<List<MapEntry<string, int>>>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI4ListI8MapEntryI6string3intEEE4sizeE(%"ArrayRef<List<MapEntry<string, int>>>"* %3)
  %7 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %5, i32 %6
  store %"List<MapEntry<string, int>>"* %7, %"List<MapEntry<string, int>>"** %end, align 8
  ret void
}

declare void @free(i8*)

define void @_EN3std4ListI4ListI8MapEntryI6string3intEEE4growE(%"List<List<MapEntry<string, int>>>"* %this) {
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI6string3intEEE7reserveE3int(%"List<List<MapEntry<string, int>>>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 2
  %capacity.load2 = load i32, i32* %capacity1, align 4
  %2 = mul i32 %capacity.load2, 2
  call void @_EN3std4ListI4ListI8MapEntryI6string3intEEE7reserveE3int(%"List<List<MapEntry<string, int>>>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std4ListI8MapEntryI6string3intEE7reserveE3int(%"List<MapEntry<string, int>>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca %"MapEntry<string, int>"*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca %"MapEntry<string, int>"*, align 8
  %target = alloca %"MapEntry<string, int>"*, align 8
  %capacity = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call %"MapEntry<string, int>"* @_EN3std13allocateArrayI8MapEntryI6string3intEEE3int(i32 %minimumCapacity)
  store %"MapEntry<string, int>"* %3, %"MapEntry<string, int>"** %newBuffer, align 8
  %size = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %buffer.load, i32 %index.load
  store %"MapEntry<string, int>"* %7, %"MapEntry<string, int>"** %source, align 8
  %newBuffer.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds %"MapEntry<string, int>", %"MapEntry<string, int>"* %newBuffer.load, i32 %index.load1
  store %"MapEntry<string, int>"* %8, %"MapEntry<string, int>"** %target, align 8
  %target.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %target, align 8
  %source.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %source, align 8
  %source.load.load = load %"MapEntry<string, int>", %"MapEntry<string, int>"* %source.load, align 8
  store %"MapEntry<string, int>" %source.load.load, %"MapEntry<string, int>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 0
  %buffer.load6 = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %buffer5, align 8
  call void @_EN3std10deallocateIAU_8MapEntryI6string3intEEEAU_8MapEntryI6string3intE(%"MapEntry<string, int>"* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 0
  %newBuffer.load10 = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %newBuffer, align 8
  store %"MapEntry<string, int>"* %newBuffer.load10, %"MapEntry<string, int>"** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 2
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

define %"MapEntry<string, int>"* @_EN3std8ArrayRefI8MapEntryI6string3intEE4dataE(%"ArrayRef<MapEntry<string, int>>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<MapEntry<string, int>>", %"ArrayRef<MapEntry<string, int>>"* %this, i32 0, i32 0
  %data.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %data, align 8
  ret %"MapEntry<string, int>"* %data.load
}

define i32 @_EN3std8ArrayRefI8MapEntryI6string3intEE4sizeE(%"ArrayRef<MapEntry<string, int>>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<MapEntry<string, int>>", %"ArrayRef<MapEntry<string, int>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %"MapEntry<string, int>"* @_EN3std4ListI8MapEntryI6string3intEE4dataE(%"List<MapEntry<string, int>>"* %this) {
  %buffer = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, int>"*, %"MapEntry<string, int>"** %buffer, align 8
  ret %"MapEntry<string, int>"* %buffer.load
}

define void @_EN3std8ArrayRefI4ListI8MapEntryI6string6stringEEE4initEP4ListI4ListI8MapEntryI6string6stringEEE(%"ArrayRef<List<MapEntry<string, string>>>"* %this, %"List<List<MapEntry<string, string>>>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<List<MapEntry<string, string>>>", %"ArrayRef<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %1 = call %"List<MapEntry<string, string>>"* @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4dataE(%"List<List<MapEntry<string, string>>>"* %list)
  store %"List<MapEntry<string, string>>"* %1, %"List<MapEntry<string, string>>"** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<List<MapEntry<string, string>>>", %"ArrayRef<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4sizeE(%"List<List<MapEntry<string, string>>>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI4ListI8MapEntryI6string6stringEEE4initE8ArrayRefI4ListI8MapEntryI6string6stringEEE(%"ArrayIterator<List<MapEntry<string, string>>>"* %this, %"ArrayRef<List<MapEntry<string, string>>>" %array) {
  %1 = alloca %"ArrayRef<List<MapEntry<string, string>>>", align 8
  %2 = alloca %"ArrayRef<List<MapEntry<string, string>>>", align 8
  %3 = alloca %"ArrayRef<List<MapEntry<string, string>>>", align 8
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, string>>>", %"ArrayIterator<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  store %"ArrayRef<List<MapEntry<string, string>>>" %array, %"ArrayRef<List<MapEntry<string, string>>>"* %1, align 8
  %4 = call %"List<MapEntry<string, string>>"* @_EN3std8ArrayRefI4ListI8MapEntryI6string6stringEEE4dataE(%"ArrayRef<List<MapEntry<string, string>>>"* %1)
  store %"List<MapEntry<string, string>>"* %4, %"List<MapEntry<string, string>>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<List<MapEntry<string, string>>>", %"ArrayIterator<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
  store %"ArrayRef<List<MapEntry<string, string>>>" %array, %"ArrayRef<List<MapEntry<string, string>>>"* %2, align 8
  %5 = call %"List<MapEntry<string, string>>"* @_EN3std8ArrayRefI4ListI8MapEntryI6string6stringEEE4dataE(%"ArrayRef<List<MapEntry<string, string>>>"* %2)
  store %"ArrayRef<List<MapEntry<string, string>>>" %array, %"ArrayRef<List<MapEntry<string, string>>>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI4ListI8MapEntryI6string6stringEEE4sizeE(%"ArrayRef<List<MapEntry<string, string>>>"* %3)
  %7 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %5, i32 %6
  store %"List<MapEntry<string, string>>"* %7, %"List<MapEntry<string, string>>"** %end, align 8
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4growE(%"List<List<MapEntry<string, string>>>"* %this) {
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE7reserveE3int(%"List<List<MapEntry<string, string>>>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 2
  %capacity.load2 = load i32, i32* %capacity1, align 4
  %2 = mul i32 %capacity.load2, 2
  call void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE7reserveE3int(%"List<List<MapEntry<string, string>>>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std8ArrayRefI8MapEntryI6string6stringEE4initEP4ListI8MapEntryI6string6stringEE(%"ArrayRef<MapEntry<string, string>>"* %this, %"List<MapEntry<string, string>>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<MapEntry<string, string>>", %"ArrayRef<MapEntry<string, string>>"* %this, i32 0, i32 0
  %1 = call %"MapEntry<string, string>"* @_EN3std4ListI8MapEntryI6string6stringEE4dataE(%"List<MapEntry<string, string>>"* %list)
  store %"MapEntry<string, string>"* %1, %"MapEntry<string, string>"** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<MapEntry<string, string>>", %"ArrayRef<MapEntry<string, string>>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI8MapEntryI6string6stringEE4sizeE(%"List<MapEntry<string, string>>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI8MapEntryI6string6stringEE4initE8ArrayRefI8MapEntryI6string6stringEE(%"ArrayIterator<MapEntry<string, string>>"* %this, %"ArrayRef<MapEntry<string, string>>" %array) {
  %1 = alloca %"ArrayRef<MapEntry<string, string>>", align 8
  %2 = alloca %"ArrayRef<MapEntry<string, string>>", align 8
  %3 = alloca %"ArrayRef<MapEntry<string, string>>", align 8
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<string, string>>", %"ArrayIterator<MapEntry<string, string>>"* %this, i32 0, i32 0
  store %"ArrayRef<MapEntry<string, string>>" %array, %"ArrayRef<MapEntry<string, string>>"* %1, align 8
  %4 = call %"MapEntry<string, string>"* @_EN3std8ArrayRefI8MapEntryI6string6stringEE4dataE(%"ArrayRef<MapEntry<string, string>>"* %1)
  store %"MapEntry<string, string>"* %4, %"MapEntry<string, string>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<MapEntry<string, string>>", %"ArrayIterator<MapEntry<string, string>>"* %this, i32 0, i32 1
  store %"ArrayRef<MapEntry<string, string>>" %array, %"ArrayRef<MapEntry<string, string>>"* %2, align 8
  %5 = call %"MapEntry<string, string>"* @_EN3std8ArrayRefI8MapEntryI6string6stringEE4dataE(%"ArrayRef<MapEntry<string, string>>"* %2)
  store %"ArrayRef<MapEntry<string, string>>" %array, %"ArrayRef<MapEntry<string, string>>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI8MapEntryI6string6stringEE4sizeE(%"ArrayRef<MapEntry<string, string>>"* %3)
  %7 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %5, i32 %6
  store %"MapEntry<string, string>"* %7, %"MapEntry<string, string>"** %end, align 8
  ret void
}

define void @_EN3std4ListI8MapEntryI6string6stringEE7reserveE3int(%"List<MapEntry<string, string>>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca %"MapEntry<string, string>"*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca %"MapEntry<string, string>"*, align 8
  %target = alloca %"MapEntry<string, string>"*, align 8
  %capacity = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call %"MapEntry<string, string>"* @_EN3std13allocateArrayI8MapEntryI6string6stringEEE3int(i32 %minimumCapacity)
  store %"MapEntry<string, string>"* %3, %"MapEntry<string, string>"** %newBuffer, align 8
  %size = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %buffer.load, i32 %index.load
  store %"MapEntry<string, string>"* %7, %"MapEntry<string, string>"** %source, align 8
  %newBuffer.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds %"MapEntry<string, string>", %"MapEntry<string, string>"* %newBuffer.load, i32 %index.load1
  store %"MapEntry<string, string>"* %8, %"MapEntry<string, string>"** %target, align 8
  %target.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %target, align 8
  %source.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %source, align 8
  %source.load.load = load %"MapEntry<string, string>", %"MapEntry<string, string>"* %source.load, align 8
  store %"MapEntry<string, string>" %source.load.load, %"MapEntry<string, string>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 0
  %buffer.load6 = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %buffer5, align 8
  call void @_EN3std10deallocateIAU_8MapEntryI6string6stringEEEAU_8MapEntryI6string6stringE(%"MapEntry<string, string>"* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 0
  %newBuffer.load10 = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %newBuffer, align 8
  store %"MapEntry<string, string>"* %newBuffer.load10, %"MapEntry<string, string>"** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity11, align 4
  br label %if.end
}

define void @_EN3std8ArrayRefI4ListI8MapEntryI12StringBuffer3intEEE4initEP4ListI4ListI8MapEntryI12StringBuffer3intEEE(%"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %this, %"List<List<MapEntry<StringBuffer, int>>>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<List<MapEntry<StringBuffer, int>>>", %"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %1 = call %"List<MapEntry<StringBuffer, int>>"* @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4dataE(%"List<List<MapEntry<StringBuffer, int>>>"* %list)
  store %"List<MapEntry<StringBuffer, int>>"* %1, %"List<MapEntry<StringBuffer, int>>"** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<List<MapEntry<StringBuffer, int>>>", %"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4sizeE(%"List<List<MapEntry<StringBuffer, int>>>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI4ListI8MapEntryI12StringBuffer3intEEE4initE8ArrayRefI4ListI8MapEntryI12StringBuffer3intEEE(%"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %this, %"ArrayRef<List<MapEntry<StringBuffer, int>>>" %array) {
  %1 = alloca %"ArrayRef<List<MapEntry<StringBuffer, int>>>", align 8
  %2 = alloca %"ArrayRef<List<MapEntry<StringBuffer, int>>>", align 8
  %3 = alloca %"ArrayRef<List<MapEntry<StringBuffer, int>>>", align 8
  %current = getelementptr inbounds %"ArrayIterator<List<MapEntry<StringBuffer, int>>>", %"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  store %"ArrayRef<List<MapEntry<StringBuffer, int>>>" %array, %"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %1, align 8
  %4 = call %"List<MapEntry<StringBuffer, int>>"* @_EN3std8ArrayRefI4ListI8MapEntryI12StringBuffer3intEEE4dataE(%"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %1)
  store %"List<MapEntry<StringBuffer, int>>"* %4, %"List<MapEntry<StringBuffer, int>>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<List<MapEntry<StringBuffer, int>>>", %"ArrayIterator<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
  store %"ArrayRef<List<MapEntry<StringBuffer, int>>>" %array, %"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %2, align 8
  %5 = call %"List<MapEntry<StringBuffer, int>>"* @_EN3std8ArrayRefI4ListI8MapEntryI12StringBuffer3intEEE4dataE(%"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %2)
  store %"ArrayRef<List<MapEntry<StringBuffer, int>>>" %array, %"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI4ListI8MapEntryI12StringBuffer3intEEE4sizeE(%"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %3)
  %7 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %5, i32 %6
  store %"List<MapEntry<StringBuffer, int>>"* %7, %"List<MapEntry<StringBuffer, int>>"** %end, align 8
  ret void
}

define void @_EN3std10deallocateIAU_8MapEntryI12StringBuffer3intEEEAU_8MapEntryI12StringBuffer3intE(%"MapEntry<StringBuffer, int>"* %allocation) {
  %1 = bitcast %"MapEntry<StringBuffer, int>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4growE(%"List<List<MapEntry<StringBuffer, int>>>"* %this) {
  %capacity = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE7reserveE3int(%"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 2
  %capacity.load2 = load i32, i32* %capacity1, align 4
  %2 = mul i32 %capacity.load2, 2
  call void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE7reserveE3int(%"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std8ArrayRefI8MapEntryI12StringBuffer3intEE4initEP4ListI8MapEntryI12StringBuffer3intEE(%"ArrayRef<MapEntry<StringBuffer, int>>"* %this, %"List<MapEntry<StringBuffer, int>>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<MapEntry<StringBuffer, int>>", %"ArrayRef<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %1 = call %"MapEntry<StringBuffer, int>"* @_EN3std4ListI8MapEntryI12StringBuffer3intEE4dataE(%"List<MapEntry<StringBuffer, int>>"* %list)
  store %"MapEntry<StringBuffer, int>"* %1, %"MapEntry<StringBuffer, int>"** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<MapEntry<StringBuffer, int>>", %"ArrayRef<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI8MapEntryI12StringBuffer3intEE4sizeE(%"List<MapEntry<StringBuffer, int>>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI8MapEntryI12StringBuffer3intEE4initE8ArrayRefI8MapEntryI12StringBuffer3intEE(%"ArrayIterator<MapEntry<StringBuffer, int>>"* %this, %"ArrayRef<MapEntry<StringBuffer, int>>" %array) {
  %1 = alloca %"ArrayRef<MapEntry<StringBuffer, int>>", align 8
  %2 = alloca %"ArrayRef<MapEntry<StringBuffer, int>>", align 8
  %3 = alloca %"ArrayRef<MapEntry<StringBuffer, int>>", align 8
  %current = getelementptr inbounds %"ArrayIterator<MapEntry<StringBuffer, int>>", %"ArrayIterator<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  store %"ArrayRef<MapEntry<StringBuffer, int>>" %array, %"ArrayRef<MapEntry<StringBuffer, int>>"* %1, align 8
  %4 = call %"MapEntry<StringBuffer, int>"* @_EN3std8ArrayRefI8MapEntryI12StringBuffer3intEE4dataE(%"ArrayRef<MapEntry<StringBuffer, int>>"* %1)
  store %"MapEntry<StringBuffer, int>"* %4, %"MapEntry<StringBuffer, int>"** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<MapEntry<StringBuffer, int>>", %"ArrayIterator<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 1
  store %"ArrayRef<MapEntry<StringBuffer, int>>" %array, %"ArrayRef<MapEntry<StringBuffer, int>>"* %2, align 8
  %5 = call %"MapEntry<StringBuffer, int>"* @_EN3std8ArrayRefI8MapEntryI12StringBuffer3intEE4dataE(%"ArrayRef<MapEntry<StringBuffer, int>>"* %2)
  store %"ArrayRef<MapEntry<StringBuffer, int>>" %array, %"ArrayRef<MapEntry<StringBuffer, int>>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI8MapEntryI12StringBuffer3intEE4sizeE(%"ArrayRef<MapEntry<StringBuffer, int>>"* %3)
  %7 = getelementptr inbounds %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %5, i32 %6
  store %"MapEntry<StringBuffer, int>"* %7, %"MapEntry<StringBuffer, int>"** %end, align 8
  ret void
}

define void @_EN3std4ListI8MapEntryI12StringBuffer3intEE7reserveE3int(%"List<MapEntry<StringBuffer, int>>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca %"MapEntry<StringBuffer, int>"*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca %"MapEntry<StringBuffer, int>"*, align 8
  %target = alloca %"MapEntry<StringBuffer, int>"*, align 8
  %capacity = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call %"MapEntry<StringBuffer, int>"* @_EN3std13allocateArrayI8MapEntryI12StringBuffer3intEEE3int(i32 %minimumCapacity)
  store %"MapEntry<StringBuffer, int>"* %3, %"MapEntry<StringBuffer, int>"** %newBuffer, align 8
  %size = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %buffer.load, i32 %index.load
  store %"MapEntry<StringBuffer, int>"* %7, %"MapEntry<StringBuffer, int>"** %source, align 8
  %newBuffer.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %newBuffer.load, i32 %index.load1
  store %"MapEntry<StringBuffer, int>"* %8, %"MapEntry<StringBuffer, int>"** %target, align 8
  %target.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %target, align 8
  %source.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %source, align 8
  %source.load.load = load %"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* %source.load, align 8
  store %"MapEntry<StringBuffer, int>" %source.load.load, %"MapEntry<StringBuffer, int>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %buffer.load6 = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %buffer5, align 8
  call void @_EN3std10deallocateIAU_8MapEntryI12StringBuffer3intEEEAU_8MapEntryI12StringBuffer3intE(%"MapEntry<StringBuffer, int>"* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %newBuffer.load10 = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %newBuffer, align 8
  store %"MapEntry<StringBuffer, int>"* %newBuffer.load10, %"MapEntry<StringBuffer, int>"** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity11, align 4
  br label %if.end
}

define %"List<MapEntry<string, int>>"* @_EN3std8ArrayRefI4ListI8MapEntryI6string3intEEE4dataE(%"ArrayRef<List<MapEntry<string, int>>>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<List<MapEntry<string, int>>>", %"ArrayRef<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %data.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %data, align 8
  ret %"List<MapEntry<string, int>>"* %data.load
}

define i32 @_EN3std8ArrayRefI4ListI8MapEntryI6string3intEEE4sizeE(%"ArrayRef<List<MapEntry<string, int>>>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<List<MapEntry<string, int>>>", %"ArrayRef<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %"List<MapEntry<string, int>>"* @_EN3std4ListI4ListI8MapEntryI6string3intEEE4dataE(%"List<List<MapEntry<string, int>>>"* %this) {
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %buffer, align 8
  ret %"List<MapEntry<string, int>>"* %buffer.load
}

define void @_EN3std4ListI4ListI8MapEntryI6string3intEEE7reserveE3int(%"List<List<MapEntry<string, int>>>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca %"List<MapEntry<string, int>>"*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca %"List<MapEntry<string, int>>"*, align 8
  %target = alloca %"List<MapEntry<string, int>>"*, align 8
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call %"List<MapEntry<string, int>>"* @_EN3std13allocateArrayI4ListI8MapEntryI6string3intEEEE3int(i32 %minimumCapacity)
  store %"List<MapEntry<string, int>>"* %3, %"List<MapEntry<string, int>>"** %newBuffer, align 8
  %size = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %buffer.load, i32 %index.load
  store %"List<MapEntry<string, int>>"* %7, %"List<MapEntry<string, int>>"** %source, align 8
  %newBuffer.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %newBuffer.load, i32 %index.load1
  store %"List<MapEntry<string, int>>"* %8, %"List<MapEntry<string, int>>"** %target, align 8
  %target.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %target, align 8
  %source.load = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %source, align 8
  %source.load.load = load %"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* %source.load, align 8
  store %"List<MapEntry<string, int>>" %source.load.load, %"List<MapEntry<string, int>>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %buffer.load6 = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %buffer5, align 8
  call void @_EN3std10deallocateIAU_4ListI8MapEntryI6string3intEEEEAU_4ListI8MapEntryI6string3intEE(%"List<MapEntry<string, int>>"* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 0
  %newBuffer.load10 = load %"List<MapEntry<string, int>>"*, %"List<MapEntry<string, int>>"** %newBuffer, align 8
  store %"List<MapEntry<string, int>>"* %newBuffer.load10, %"List<MapEntry<string, int>>"** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<List<MapEntry<string, int>>>", %"List<List<MapEntry<string, int>>>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity11, align 4
  br label %if.end
}

define %"MapEntry<string, int>"* @_EN3std13allocateArrayI8MapEntryI6string3intEEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (%"MapEntry<string, int>"* getelementptr (%"MapEntry<string, int>", %"MapEntry<string, int>"* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @78, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to %"MapEntry<string, int>"*
  ret %"MapEntry<string, int>"* %4
}

declare %OutputFileStream @_EN3std6stderrE()

declare void @_EN3std16OutputFileStream5writeI6stringEEP6string(%OutputFileStream*, %string*)

declare void @_EN3std16OutputFileStream5writeI3intEEP3int(%OutputFileStream*, i32*)

declare void @_EN3std16OutputFileStream5writeI4charEEP4char(%OutputFileStream*, i8*)

declare %never @_EN3std12abortWrapperE()

define %"List<MapEntry<string, string>>"* @_EN3std8ArrayRefI4ListI8MapEntryI6string6stringEEE4dataE(%"ArrayRef<List<MapEntry<string, string>>>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<List<MapEntry<string, string>>>", %"ArrayRef<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %data.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %data, align 8
  ret %"List<MapEntry<string, string>>"* %data.load
}

define i32 @_EN3std8ArrayRefI4ListI8MapEntryI6string6stringEEE4sizeE(%"ArrayRef<List<MapEntry<string, string>>>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<List<MapEntry<string, string>>>", %"ArrayRef<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %"List<MapEntry<string, string>>"* @_EN3std4ListI4ListI8MapEntryI6string6stringEEE4dataE(%"List<List<MapEntry<string, string>>>"* %this) {
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %buffer, align 8
  ret %"List<MapEntry<string, string>>"* %buffer.load
}

define void @_EN3std4ListI4ListI8MapEntryI6string6stringEEE7reserveE3int(%"List<List<MapEntry<string, string>>>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca %"List<MapEntry<string, string>>"*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca %"List<MapEntry<string, string>>"*, align 8
  %target = alloca %"List<MapEntry<string, string>>"*, align 8
  %capacity = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call %"List<MapEntry<string, string>>"* @_EN3std13allocateArrayI4ListI8MapEntryI6string6stringEEEE3int(i32 %minimumCapacity)
  store %"List<MapEntry<string, string>>"* %3, %"List<MapEntry<string, string>>"** %newBuffer, align 8
  %size = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %buffer.load, i32 %index.load
  store %"List<MapEntry<string, string>>"* %7, %"List<MapEntry<string, string>>"** %source, align 8
  %newBuffer.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %newBuffer.load, i32 %index.load1
  store %"List<MapEntry<string, string>>"* %8, %"List<MapEntry<string, string>>"** %target, align 8
  %target.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %target, align 8
  %source.load = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %source, align 8
  %source.load.load = load %"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* %source.load, align 8
  store %"List<MapEntry<string, string>>" %source.load.load, %"List<MapEntry<string, string>>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %buffer.load6 = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %buffer5, align 8
  call void @_EN3std10deallocateIAU_4ListI8MapEntryI6string6stringEEEEAU_4ListI8MapEntryI6string6stringEE(%"List<MapEntry<string, string>>"* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 0
  %newBuffer.load10 = load %"List<MapEntry<string, string>>"*, %"List<MapEntry<string, string>>"** %newBuffer, align 8
  store %"List<MapEntry<string, string>>"* %newBuffer.load10, %"List<MapEntry<string, string>>"** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<List<MapEntry<string, string>>>", %"List<List<MapEntry<string, string>>>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity11, align 4
  br label %if.end
}

define %"MapEntry<string, string>"* @_EN3std8ArrayRefI8MapEntryI6string6stringEE4dataE(%"ArrayRef<MapEntry<string, string>>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<MapEntry<string, string>>", %"ArrayRef<MapEntry<string, string>>"* %this, i32 0, i32 0
  %data.load = load %"MapEntry<string, string>"*, %"MapEntry<string, string>"** %data, align 8
  ret %"MapEntry<string, string>"* %data.load
}

define i32 @_EN3std8ArrayRefI8MapEntryI6string6stringEE4sizeE(%"ArrayRef<MapEntry<string, string>>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<MapEntry<string, string>>", %"ArrayRef<MapEntry<string, string>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %"MapEntry<string, string>"* @_EN3std13allocateArrayI8MapEntryI6string6stringEEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 mul nuw (i64 ptrtoint (%"ArrayRef<char>"* getelementptr (%"ArrayRef<char>", %"ArrayRef<char>"* null, i32 1) to i64), i64 2), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @79, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to %"MapEntry<string, string>"*
  ret %"MapEntry<string, string>"* %4
}

define %"List<MapEntry<StringBuffer, int>>"* @_EN3std8ArrayRefI4ListI8MapEntryI12StringBuffer3intEEE4dataE(%"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<List<MapEntry<StringBuffer, int>>>", %"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %data.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %data, align 8
  ret %"List<MapEntry<StringBuffer, int>>"* %data.load
}

define i32 @_EN3std8ArrayRefI4ListI8MapEntryI12StringBuffer3intEEE4sizeE(%"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<List<MapEntry<StringBuffer, int>>>", %"ArrayRef<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %"List<MapEntry<StringBuffer, int>>"* @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE4dataE(%"List<List<MapEntry<StringBuffer, int>>>"* %this) {
  %buffer = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %buffer, align 8
  ret %"List<MapEntry<StringBuffer, int>>"* %buffer.load
}

define void @_EN3std4ListI4ListI8MapEntryI12StringBuffer3intEEE7reserveE3int(%"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca %"List<MapEntry<StringBuffer, int>>"*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca %"List<MapEntry<StringBuffer, int>>"*, align 8
  %target = alloca %"List<MapEntry<StringBuffer, int>>"*, align 8
  %capacity = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call %"List<MapEntry<StringBuffer, int>>"* @_EN3std13allocateArrayI4ListI8MapEntryI12StringBuffer3intEEEE3int(i32 %minimumCapacity)
  store %"List<MapEntry<StringBuffer, int>>"* %3, %"List<MapEntry<StringBuffer, int>>"** %newBuffer, align 8
  %size = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %buffer.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %buffer.load, i32 %index.load
  store %"List<MapEntry<StringBuffer, int>>"* %7, %"List<MapEntry<StringBuffer, int>>"** %source, align 8
  %newBuffer.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %newBuffer.load, i32 %index.load1
  store %"List<MapEntry<StringBuffer, int>>"* %8, %"List<MapEntry<StringBuffer, int>>"** %target, align 8
  %target.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %target, align 8
  %source.load = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %source, align 8
  %source.load.load = load %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %source.load, align 8
  store %"List<MapEntry<StringBuffer, int>>" %source.load.load, %"List<MapEntry<StringBuffer, int>>"* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %buffer.load6 = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %buffer5, align 8
  call void @_EN3std10deallocateIAU_4ListI8MapEntryI12StringBuffer3intEEEEAU_4ListI8MapEntryI12StringBuffer3intEE(%"List<MapEntry<StringBuffer, int>>"* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 0
  %newBuffer.load10 = load %"List<MapEntry<StringBuffer, int>>"*, %"List<MapEntry<StringBuffer, int>>"** %newBuffer, align 8
  store %"List<MapEntry<StringBuffer, int>>"* %newBuffer.load10, %"List<MapEntry<StringBuffer, int>>"** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<List<MapEntry<StringBuffer, int>>>", %"List<List<MapEntry<StringBuffer, int>>>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity11, align 4
  br label %if.end
}

define %"MapEntry<StringBuffer, int>"* @_EN3std8ArrayRefI8MapEntryI12StringBuffer3intEE4dataE(%"ArrayRef<MapEntry<StringBuffer, int>>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<MapEntry<StringBuffer, int>>", %"ArrayRef<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %data.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %data, align 8
  ret %"MapEntry<StringBuffer, int>"* %data.load
}

define i32 @_EN3std8ArrayRefI8MapEntryI12StringBuffer3intEE4sizeE(%"ArrayRef<MapEntry<StringBuffer, int>>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<MapEntry<StringBuffer, int>>", %"ArrayRef<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %"MapEntry<StringBuffer, int>"* @_EN3std4ListI8MapEntryI12StringBuffer3intEE4dataE(%"List<MapEntry<StringBuffer, int>>"* %this) {
  %buffer = getelementptr inbounds %"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* %this, i32 0, i32 0
  %buffer.load = load %"MapEntry<StringBuffer, int>"*, %"MapEntry<StringBuffer, int>"** %buffer, align 8
  ret %"MapEntry<StringBuffer, int>"* %buffer.load
}

define %"MapEntry<StringBuffer, int>"* @_EN3std13allocateArrayI8MapEntryI12StringBuffer3intEEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (%"MapEntry<StringBuffer, int>"* getelementptr (%"MapEntry<StringBuffer, int>", %"MapEntry<StringBuffer, int>"* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @80, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to %"MapEntry<StringBuffer, int>"*
  ret %"MapEntry<StringBuffer, int>"* %4
}

define %"List<MapEntry<string, int>>"* @_EN3std13allocateArrayI4ListI8MapEntryI6string3intEEEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (%"List<MapEntry<string, int>>"* getelementptr (%"List<MapEntry<string, int>>", %"List<MapEntry<string, int>>"* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @81, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to %"List<MapEntry<string, int>>"*
  ret %"List<MapEntry<string, int>>"* %4
}

declare i8* @malloc(i64)

define %"List<MapEntry<string, string>>"* @_EN3std13allocateArrayI4ListI8MapEntryI6string6stringEEEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (%"List<MapEntry<string, string>>"* getelementptr (%"List<MapEntry<string, string>>", %"List<MapEntry<string, string>>"* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @82, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to %"List<MapEntry<string, string>>"*
  ret %"List<MapEntry<string, string>>"* %4
}

define %"List<MapEntry<StringBuffer, int>>"* @_EN3std13allocateArrayI4ListI8MapEntryI12StringBuffer3intEEEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (%"List<MapEntry<StringBuffer, int>>"* getelementptr (%"List<MapEntry<StringBuffer, int>>", %"List<MapEntry<StringBuffer, int>>"* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @83, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to %"List<MapEntry<StringBuffer, int>>"*
  ret %"List<MapEntry<StringBuffer, int>>"* %4
}
