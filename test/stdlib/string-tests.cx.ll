
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }
%StringIterator = type { i8*, i8* }
%StringBuffer = type { %"List<char>" }
%"List<char>" = type { i8*, i32, i32 }
%"Range<int>" = type { i32, i32 }
%"List<string>" = type { %string*, i32, i32 }
%ByteIterator = type { %string }
%LineIterator = type { %string, i32 }
%"Optional<int>" = type { i1, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"ArrayIterator<string>" = type { %string*, %string* }
%never = type {}
%"ArrayRef<string>" = type { %string*, i32 }
%OutputFileStream = type { %FILE* }
%FILE = type {}

@0 = private unnamed_addr constant [4 x i8] c"abc\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"abc\00", align 1
@2 = private unnamed_addr constant [41 x i8] c"Assertion failed at string-tests.cx:5:5\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"abc\00", align 1
@4 = private unnamed_addr constant [4 x i8] c"not\00", align 1
@5 = private unnamed_addr constant [41 x i8] c"Assertion failed at string-tests.cx:6:5\0A\00", align 1
@6 = private unnamed_addr constant [4 x i8] c"abc\00", align 1
@7 = private unnamed_addr constant [4 x i8] c"bcd\00", align 1
@8 = private unnamed_addr constant [41 x i8] c"Assertion failed at string-tests.cx:7:5\0A\00", align 1
@9 = private unnamed_addr constant [3 x i8] c"xy\00", align 1
@10 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:24:5\0A\00", align 1
@11 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:25:5\0A\00", align 1
@12 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:28:5\0A\00", align 1
@13 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:29:5\0A\00", align 1
@14 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:32:5\0A\00", align 1
@15 = private unnamed_addr constant [5 x i8] c"word\00", align 1
@16 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:39:5\0A\00", align 1
@17 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:40:5\0A\00", align 1
@18 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:41:5\0A\00", align 1
@19 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:42:5\0A\00", align 1
@20 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:45:5\0A\00", align 1
@21 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:46:5\0A\00", align 1
@22 = private unnamed_addr constant [5 x i8] c"word\00", align 1
@23 = private unnamed_addr constant [2 x i8] c"w\00", align 1
@24 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:51:5\0A\00", align 1
@25 = private unnamed_addr constant [3 x i8] c"wo\00", align 1
@26 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:52:5\0A\00", align 1
@27 = private unnamed_addr constant [4 x i8] c"wor\00", align 1
@28 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:53:5\0A\00", align 1
@29 = private unnamed_addr constant [5 x i8] c"word\00", align 1
@30 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:54:5\0A\00", align 1
@31 = private unnamed_addr constant [4 x i8] c"ord\00", align 1
@32 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:55:5\0A\00", align 1
@33 = private unnamed_addr constant [3 x i8] c"rd\00", align 1
@34 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:56:5\0A\00", align 1
@35 = private unnamed_addr constant [3 x i8] c"or\00", align 1
@36 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:57:5\0A\00", align 1
@37 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@38 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:58:5\0A\00", align 1
@39 = private unnamed_addr constant [26 x i8] c"this is the best sentence\00", align 1
@40 = private unnamed_addr constant [5 x i8] c"this\00", align 1
@41 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:61:5\0A\00", align 1
@42 = private unnamed_addr constant [3 x i8] c"is\00", align 1
@43 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:62:5\0A\00", align 1
@44 = private unnamed_addr constant [4 x i8] c"the\00", align 1
@45 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:63:5\0A\00", align 1
@46 = private unnamed_addr constant [5 x i8] c"best\00", align 1
@47 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:64:5\0A\00", align 1
@48 = private unnamed_addr constant [9 x i8] c"sentence\00", align 1
@49 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:65:5\0A\00", align 1
@50 = private unnamed_addr constant [5 x i8] c"best\00", align 1
@51 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@52 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:68:5\0A\00", align 1
@53 = private unnamed_addr constant [26 x i8] c"this is the best sentence\00", align 1
@54 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:74:5\0A\00", align 1
@55 = private unnamed_addr constant [5 x i8] c"this\00", align 1
@56 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:75:5\0A\00", align 1
@57 = private unnamed_addr constant [3 x i8] c"is\00", align 1
@58 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:76:5\0A\00", align 1
@59 = private unnamed_addr constant [4 x i8] c"the\00", align 1
@60 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:77:5\0A\00", align 1
@61 = private unnamed_addr constant [5 x i8] c"best\00", align 1
@62 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:78:5\0A\00", align 1
@63 = private unnamed_addr constant [9 x i8] c"sentence\00", align 1
@64 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:79:5\0A\00", align 1
@65 = private unnamed_addr constant [33 x i8] c"this,sentence,is,comma,separated\00", align 1
@66 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:85:5\0A\00", align 1
@67 = private unnamed_addr constant [5 x i8] c"this\00", align 1
@68 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:86:5\0A\00", align 1
@69 = private unnamed_addr constant [9 x i8] c"sentence\00", align 1
@70 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:87:5\0A\00", align 1
@71 = private unnamed_addr constant [3 x i8] c"is\00", align 1
@72 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:88:5\0A\00", align 1
@73 = private unnamed_addr constant [6 x i8] c"comma\00", align 1
@74 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:89:5\0A\00", align 1
@75 = private unnamed_addr constant [10 x i8] c"separated\00", align 1
@76 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:90:5\0A\00", align 1
@77 = private unnamed_addr constant [7 x i8] c",,1,,2\00", align 1
@78 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:94:5\0A\00", align 1
@79 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@80 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:95:5\0A\00", align 1
@81 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@82 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:96:5\0A\00", align 1
@83 = private unnamed_addr constant [2 x i8] c"1\00", align 1
@84 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:97:5\0A\00", align 1
@85 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@86 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:98:5\0A\00", align 1
@87 = private unnamed_addr constant [2 x i8] c"2\00", align 1
@88 = private unnamed_addr constant [42 x i8] c"Assertion failed at string-tests.cx:99:5\0A\00", align 1
@89 = private unnamed_addr constant [10 x i8] c",,,b,a,,,\00", align 1
@90 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:103:5\0A\00", align 1
@91 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@92 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:104:5\0A\00", align 1
@93 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@94 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:105:5\0A\00", align 1
@95 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@96 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:106:5\0A\00", align 1
@97 = private unnamed_addr constant [2 x i8] c"b\00", align 1
@98 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:107:5\0A\00", align 1
@99 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@100 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:108:5\0A\00", align 1
@101 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@102 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:109:5\0A\00", align 1
@103 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@104 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:110:5\0A\00", align 1
@105 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@106 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:111:5\0A\00", align 1
@107 = private unnamed_addr constant [44 x i8] c"The quick brown fox jumps over the lazy dog\00", align 1
@108 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:120:9\0A\00", align 1
@109 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:124:5\0A\00", align 1
@110 = private unnamed_addr constant [50 x i8] c"The quick brown fox \0A\0A\0A jumps over\0A the lazy dog\0A\00", align 1
@111 = private unnamed_addr constant [21 x i8] c"The quick brown fox \00", align 1
@112 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@113 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@114 = private unnamed_addr constant [12 x i8] c" jumps over\00", align 1
@115 = private unnamed_addr constant [14 x i8] c" the lazy dog\00", align 1
@116 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:133:9\0A\00", align 1
@117 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:137:5\0A\00", align 1
@118 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@119 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:141:5\0A\00", align 1
@120 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@121 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:142:5\0A\00", align 1
@122 = private unnamed_addr constant [3 x i8] c"1a\00", align 1
@123 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:143:5\0A\00", align 1
@124 = private unnamed_addr constant [2 x i8] c"1\00", align 1
@125 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:144:5\0A\00", align 1
@126 = private unnamed_addr constant [7 x i8] c"123789\00", align 1
@127 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:145:5\0A\00", align 1
@128 = private unnamed_addr constant [11 x i8] c"2147483647\00", align 1
@129 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:146:5\0A\00", align 1
@130 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@131 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@132 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:150:5\0A\00", align 1
@133 = private unnamed_addr constant [10 x i8] c"0 r t ' n\00", align 1
@134 = private unnamed_addr constant [10 x i8] c"0 r t ' n\00", align 1
@135 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:151:5\0A\00", align 1
@136 = private unnamed_addr constant [14 x i8] c"\00 \0D \09 ' \22 \\ \0A\00", align 1
@137 = private unnamed_addr constant [20 x i8] c"\\0 \\r \\t ' \\\22 \\\\ \\n\00", align 1
@138 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:152:5\0A\00", align 1
@139 = private unnamed_addr constant [4 x i8] c"abc\00", align 1
@140 = private unnamed_addr constant [10 x i8] c"abcabcabc\00", align 1
@141 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:156:5\0A\00", align 1
@142 = private unnamed_addr constant [2 x i8] c"1\00", align 1
@143 = private unnamed_addr constant [6 x i8] c"11111\00", align 1
@144 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:157:5\0A\00", align 1
@145 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@146 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@147 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:158:5\0A\00", align 1
@148 = private unnamed_addr constant [4 x i8] c"abc\00", align 1
@149 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@150 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:159:5\0A\00", align 1
@151 = private unnamed_addr constant [4 x i8] c"abc\00", align 1
@152 = private unnamed_addr constant [4 x i8] c"abc\00", align 1
@153 = private unnamed_addr constant [43 x i8] c"Assertion failed at string-tests.cx:160:5\0A\00", align 1
@154 = private unnamed_addr constant [7 x i8] c"substr\00", align 1
@155 = private unnamed_addr constant [7 x i8] c"substr\00", align 1
@156 = private unnamed_addr constant [3 x i8] c"\\0\00", align 1
@157 = private unnamed_addr constant [3 x i8] c"\\n\00", align 1
@158 = private unnamed_addr constant [3 x i8] c"\\r\00", align 1
@159 = private unnamed_addr constant [3 x i8] c"\\t\00", align 1
@160 = private unnamed_addr constant [3 x i8] c"\\\22\00", align 1
@161 = private unnamed_addr constant [3 x i8] c"\\\\\00", align 1
@162 = private unnamed_addr constant [14 x i8] c"StringBuffer.\00", align 1
@163 = private unnamed_addr constant [9 x i8] c": index \00", align 1
@164 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@165 = private unnamed_addr constant [12 x i8] c"List index \00", align 1
@166 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@167 = private unnamed_addr constant [7 x i8] c"substr\00", align 1
@168 = private unnamed_addr constant [7 x i8] c"substr\00", align 1
@169 = private unnamed_addr constant [7 x i8] c"substr\00", align 1
@170 = private unnamed_addr constant [12 x i8] c"List index \00", align 1
@171 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@172 = private unnamed_addr constant [6 x i8] c"front\00", align 1
@173 = private unnamed_addr constant [8 x i8] c"string.\00", align 1
@174 = private unnamed_addr constant [9 x i8] c": index \00", align 1
@175 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1
@176 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1

define i32 @main() {
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %__str5 = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @1, i32 0, i32 0), i32 3)
  %__str.load6 = load %string, %string* %__str1, align 8
  %1 = call i1 @_EN3stdeqE6string6string(%string %__str.load, %string %__str.load6)
  %assert.condition = icmp eq i1 %1, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([41 x i8], [41 x i8]* @2, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @3, i32 0, i32 0), i32 3)
  %__str.load7 = load %string, %string* %__str2, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @4, i32 0, i32 0), i32 3)
  %__str.load8 = load %string, %string* %__str3, align 8
  %2 = call i1 @_EN3stdeqE6string6string(%string %__str.load7, %string %__str.load8)
  %3 = xor i1 %2, true
  %assert.condition9 = icmp eq i1 %3, false
  br i1 %assert.condition9, label %assert.fail10, label %assert.success11

assert.fail10:                                    ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([41 x i8], [41 x i8]* @5, i32 0, i32 0))
  unreachable

assert.success11:                                 ; preds = %assert.success
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @6, i32 0, i32 0), i32 3)
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @7, i32 0, i32 0), i32 3)
  %4 = call i1 @_EN3stdltI6stringEEP6stringP6string(%string* %__str4, %string* %__str5)
  %assert.condition12 = icmp eq i1 %4, false
  br i1 %assert.condition12, label %assert.fail13, label %assert.success14

assert.fail13:                                    ; preds = %assert.success11
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([41 x i8], [41 x i8]* @8, i32 0, i32 0))
  unreachable

assert.success14:                                 ; preds = %assert.success11
  call void @_EN4main18testStringIteratorE()
  call void @_EN4main8testFindE()
  call void @_EN4main10testSubstrE()
  call void @_EN4main9testSplitE()
  call void @_EN4main14testOtherSplitE()
  call void @_EN4main9testBytesE()
  call void @_EN4main9testLinesE()
  call void @_EN4main12testParseIntE()
  call void @_EN4main10testEscapeE()
  call void @_EN4main10testRepeatE()
  ret i32 0
}

declare void @_EN3std6string4initEP4char3int(%string*, i8*, i32)

declare i1 @_EN3stdeqE6string6string(%string, %string)

declare void @_EN3std10assertFailEP4char(i8*)

define i1 @_EN3stdltI6stringEEP6stringP6string(%string* %a, %string* %b) {
  %1 = call i32 @_EN3std6string7compareEP6string(%string* %a, %string* %b)
  %2 = icmp eq i32 %1, 0
  ret i1 %2
}

define void @_EN4main18testStringIteratorE() {
  %iterator = alloca %StringIterator, align 8
  %__str = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0), i32 2)
  %1 = call %StringIterator @_EN3std6string8iteratorE(%string* %__str)
  store %StringIterator %1, %StringIterator* %iterator, align 8
  %2 = call i1 @_EN3std14StringIterator8hasValueE(%StringIterator* %iterator)
  %assert.condition = icmp eq i1 %2, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @10, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %3 = call i8 @_EN3std14StringIterator5valueE(%StringIterator* %iterator)
  %4 = icmp eq i8 %3, 120
  %assert.condition1 = icmp eq i1 %4, false
  br i1 %assert.condition1, label %assert.fail2, label %assert.success3

assert.fail2:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @11, i32 0, i32 0))
  unreachable

assert.success3:                                  ; preds = %assert.success
  call void @_EN3std14StringIterator9incrementE(%StringIterator* %iterator)
  %5 = call i1 @_EN3std14StringIterator8hasValueE(%StringIterator* %iterator)
  %assert.condition4 = icmp eq i1 %5, false
  br i1 %assert.condition4, label %assert.fail5, label %assert.success6

assert.fail5:                                     ; preds = %assert.success3
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @12, i32 0, i32 0))
  unreachable

assert.success6:                                  ; preds = %assert.success3
  %6 = call i8 @_EN3std14StringIterator5valueE(%StringIterator* %iterator)
  %7 = icmp eq i8 %6, 121
  %assert.condition7 = icmp eq i1 %7, false
  br i1 %assert.condition7, label %assert.fail8, label %assert.success9

assert.fail8:                                     ; preds = %assert.success6
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @13, i32 0, i32 0))
  unreachable

assert.success9:                                  ; preds = %assert.success6
  call void @_EN3std14StringIterator9incrementE(%StringIterator* %iterator)
  %8 = call i1 @_EN3std14StringIterator8hasValueE(%StringIterator* %iterator)
  %9 = xor i1 %8, true
  %assert.condition10 = icmp eq i1 %9, false
  br i1 %assert.condition10, label %assert.fail11, label %assert.success12

assert.fail11:                                    ; preds = %assert.success9
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @14, i32 0, i32 0))
  unreachable

assert.success12:                                 ; preds = %assert.success9
  ret void
}

define void @_EN4main8testFindE() {
  %s = alloca %StringBuffer, align 8
  %__str = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @15, i32 0, i32 0), i32 4)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std12StringBuffer4initE6string(%StringBuffer* %s, %string %__str.load)
  %1 = call i32 @_EN3std12StringBuffer4findE4char(%StringBuffer* %s, i8 119)
  %2 = icmp eq i32 %1, 0
  %assert.condition = icmp eq i1 %2, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @16, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %3 = call i32 @_EN3std12StringBuffer4findE4char(%StringBuffer* %s, i8 111)
  %4 = icmp eq i32 %3, 1
  %assert.condition1 = icmp eq i1 %4, false
  br i1 %assert.condition1, label %assert.fail2, label %assert.success3

assert.fail2:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @17, i32 0, i32 0))
  unreachable

assert.success3:                                  ; preds = %assert.success
  %5 = call i32 @_EN3std12StringBuffer4findE4char(%StringBuffer* %s, i8 114)
  %6 = icmp eq i32 %5, 2
  %assert.condition4 = icmp eq i1 %6, false
  br i1 %assert.condition4, label %assert.fail5, label %assert.success6

assert.fail5:                                     ; preds = %assert.success3
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @18, i32 0, i32 0))
  unreachable

assert.success6:                                  ; preds = %assert.success3
  %7 = call i32 @_EN3std12StringBuffer4findE4char(%StringBuffer* %s, i8 100)
  %8 = icmp eq i32 %7, 3
  %assert.condition7 = icmp eq i1 %8, false
  br i1 %assert.condition7, label %assert.fail8, label %assert.success9

assert.fail8:                                     ; preds = %assert.success6
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @19, i32 0, i32 0))
  unreachable

assert.success9:                                  ; preds = %assert.success6
  %9 = call i32 @_EN3std12StringBuffer4findE4char3int(%StringBuffer* %s, i8 119, i32 1)
  %10 = call i32 @_EN3std12StringBuffer4sizeE(%StringBuffer* %s)
  %11 = icmp eq i32 %9, %10
  %assert.condition10 = icmp eq i1 %11, false
  br i1 %assert.condition10, label %assert.fail11, label %assert.success12

assert.fail11:                                    ; preds = %assert.success9
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @20, i32 0, i32 0))
  unreachable

assert.success12:                                 ; preds = %assert.success9
  %12 = call i32 @_EN3std12StringBuffer4findE4char3int(%StringBuffer* %s, i8 100, i32 2)
  %13 = icmp eq i32 %12, 3
  %assert.condition13 = icmp eq i1 %13, false
  br i1 %assert.condition13, label %assert.fail14, label %assert.success15

assert.fail14:                                    ; preds = %assert.success12
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @21, i32 0, i32 0))
  unreachable

assert.success15:                                 ; preds = %assert.success12
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %s)
  ret void
}

define void @_EN4main10testSubstrE() {
  %s = alloca %StringBuffer, align 8
  %__str = alloca %string, align 8
  %1 = alloca %"Range<int>", align 8
  %__str1 = alloca %string, align 8
  %2 = alloca %"Range<int>", align 8
  %__str2 = alloca %string, align 8
  %3 = alloca %"Range<int>", align 8
  %__str3 = alloca %string, align 8
  %4 = alloca %"Range<int>", align 8
  %__str4 = alloca %string, align 8
  %5 = alloca %"Range<int>", align 8
  %__str5 = alloca %string, align 8
  %6 = alloca %"Range<int>", align 8
  %__str6 = alloca %string, align 8
  %7 = alloca %"Range<int>", align 8
  %__str7 = alloca %string, align 8
  %8 = alloca %"Range<int>", align 8
  %__str8 = alloca %string, align 8
  %s2 = alloca %StringBuffer, align 8
  %__str9 = alloca %string, align 8
  %9 = alloca %"Range<int>", align 8
  %__str10 = alloca %string, align 8
  %10 = alloca %"Range<int>", align 8
  %__str11 = alloca %string, align 8
  %11 = alloca %"Range<int>", align 8
  %__str12 = alloca %string, align 8
  %12 = alloca %"Range<int>", align 8
  %__str13 = alloca %string, align 8
  %13 = alloca %"Range<int>", align 8
  %__str14 = alloca %string, align 8
  %s3 = alloca %StringBuffer, align 8
  %__str15 = alloca %string, align 8
  %14 = alloca %"Range<int>", align 8
  %__str16 = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @22, i32 0, i32 0), i32 4)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std12StringBuffer4initE6string(%StringBuffer* %s, %string %__str.load)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 1)
  %.load = load %"Range<int>", %"Range<int>"* %1, align 4
  %15 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s, %"Range<int>" %.load)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @23, i32 0, i32 0), i32 1)
  %__str.load17 = load %string, %string* %__str1, align 8
  %16 = call i1 @_EN3stdeqE6string6string(%string %15, %string %__str.load17)
  %assert.condition = icmp eq i1 %16, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @24, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %2, i32 0, i32 2)
  %.load18 = load %"Range<int>", %"Range<int>"* %2, align 4
  %17 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s, %"Range<int>" %.load18)
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @25, i32 0, i32 0), i32 2)
  %__str.load19 = load %string, %string* %__str2, align 8
  %18 = call i1 @_EN3stdeqE6string6string(%string %17, %string %__str.load19)
  %assert.condition20 = icmp eq i1 %18, false
  br i1 %assert.condition20, label %assert.fail21, label %assert.success22

assert.fail21:                                    ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @26, i32 0, i32 0))
  unreachable

assert.success22:                                 ; preds = %assert.success
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %3, i32 0, i32 3)
  %.load23 = load %"Range<int>", %"Range<int>"* %3, align 4
  %19 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s, %"Range<int>" %.load23)
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @27, i32 0, i32 0), i32 3)
  %__str.load24 = load %string, %string* %__str3, align 8
  %20 = call i1 @_EN3stdeqE6string6string(%string %19, %string %__str.load24)
  %assert.condition25 = icmp eq i1 %20, false
  br i1 %assert.condition25, label %assert.fail26, label %assert.success27

assert.fail26:                                    ; preds = %assert.success22
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @28, i32 0, i32 0))
  unreachable

assert.success27:                                 ; preds = %assert.success22
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %4, i32 0, i32 4)
  %.load28 = load %"Range<int>", %"Range<int>"* %4, align 4
  %21 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s, %"Range<int>" %.load28)
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @29, i32 0, i32 0), i32 4)
  %__str.load29 = load %string, %string* %__str4, align 8
  %22 = call i1 @_EN3stdeqE6string6string(%string %21, %string %__str.load29)
  %assert.condition30 = icmp eq i1 %22, false
  br i1 %assert.condition30, label %assert.fail31, label %assert.success32

assert.fail31:                                    ; preds = %assert.success27
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @30, i32 0, i32 0))
  unreachable

assert.success32:                                 ; preds = %assert.success27
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %5, i32 1, i32 4)
  %.load33 = load %"Range<int>", %"Range<int>"* %5, align 4
  %23 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s, %"Range<int>" %.load33)
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @31, i32 0, i32 0), i32 3)
  %__str.load34 = load %string, %string* %__str5, align 8
  %24 = call i1 @_EN3stdeqE6string6string(%string %23, %string %__str.load34)
  %assert.condition35 = icmp eq i1 %24, false
  br i1 %assert.condition35, label %assert.fail36, label %assert.success37

assert.fail36:                                    ; preds = %assert.success32
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @32, i32 0, i32 0))
  unreachable

assert.success37:                                 ; preds = %assert.success32
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %6, i32 2, i32 4)
  %.load38 = load %"Range<int>", %"Range<int>"* %6, align 4
  %25 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s, %"Range<int>" %.load38)
  call void @_EN3std6string4initEP4char3int(%string* %__str6, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @33, i32 0, i32 0), i32 2)
  %__str.load39 = load %string, %string* %__str6, align 8
  %26 = call i1 @_EN3stdeqE6string6string(%string %25, %string %__str.load39)
  %assert.condition40 = icmp eq i1 %26, false
  br i1 %assert.condition40, label %assert.fail41, label %assert.success42

assert.fail41:                                    ; preds = %assert.success37
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @34, i32 0, i32 0))
  unreachable

assert.success42:                                 ; preds = %assert.success37
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %7, i32 1, i32 3)
  %.load43 = load %"Range<int>", %"Range<int>"* %7, align 4
  %27 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s, %"Range<int>" %.load43)
  call void @_EN3std6string4initEP4char3int(%string* %__str7, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @35, i32 0, i32 0), i32 2)
  %__str.load44 = load %string, %string* %__str7, align 8
  %28 = call i1 @_EN3stdeqE6string6string(%string %27, %string %__str.load44)
  %assert.condition45 = icmp eq i1 %28, false
  br i1 %assert.condition45, label %assert.fail46, label %assert.success47

assert.fail46:                                    ; preds = %assert.success42
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @36, i32 0, i32 0))
  unreachable

assert.success47:                                 ; preds = %assert.success42
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %8, i32 2, i32 3)
  %.load48 = load %"Range<int>", %"Range<int>"* %8, align 4
  %29 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s, %"Range<int>" %.load48)
  call void @_EN3std6string4initEP4char3int(%string* %__str8, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @37, i32 0, i32 0), i32 1)
  %__str.load49 = load %string, %string* %__str8, align 8
  %30 = call i1 @_EN3stdeqE6string6string(%string %29, %string %__str.load49)
  %assert.condition50 = icmp eq i1 %30, false
  br i1 %assert.condition50, label %assert.fail51, label %assert.success52

assert.fail51:                                    ; preds = %assert.success47
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @38, i32 0, i32 0))
  unreachable

assert.success52:                                 ; preds = %assert.success47
  call void @_EN3std6string4initEP4char3int(%string* %__str9, i8* getelementptr inbounds ([26 x i8], [26 x i8]* @39, i32 0, i32 0), i32 25)
  %__str.load53 = load %string, %string* %__str9, align 8
  call void @_EN3std12StringBuffer4initE6string(%StringBuffer* %s2, %string %__str.load53)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %9, i32 0, i32 4)
  %.load54 = load %"Range<int>", %"Range<int>"* %9, align 4
  %31 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s2, %"Range<int>" %.load54)
  call void @_EN3std6string4initEP4char3int(%string* %__str10, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @40, i32 0, i32 0), i32 4)
  %__str.load55 = load %string, %string* %__str10, align 8
  %32 = call i1 @_EN3stdeqE6string6string(%string %31, %string %__str.load55)
  %assert.condition56 = icmp eq i1 %32, false
  br i1 %assert.condition56, label %assert.fail57, label %assert.success58

assert.fail57:                                    ; preds = %assert.success52
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @41, i32 0, i32 0))
  unreachable

assert.success58:                                 ; preds = %assert.success52
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %10, i32 5, i32 7)
  %.load59 = load %"Range<int>", %"Range<int>"* %10, align 4
  %33 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s2, %"Range<int>" %.load59)
  call void @_EN3std6string4initEP4char3int(%string* %__str11, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @42, i32 0, i32 0), i32 2)
  %__str.load60 = load %string, %string* %__str11, align 8
  %34 = call i1 @_EN3stdeqE6string6string(%string %33, %string %__str.load60)
  %assert.condition61 = icmp eq i1 %34, false
  br i1 %assert.condition61, label %assert.fail62, label %assert.success63

assert.fail62:                                    ; preds = %assert.success58
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @43, i32 0, i32 0))
  unreachable

assert.success63:                                 ; preds = %assert.success58
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %11, i32 8, i32 11)
  %.load64 = load %"Range<int>", %"Range<int>"* %11, align 4
  %35 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s2, %"Range<int>" %.load64)
  call void @_EN3std6string4initEP4char3int(%string* %__str12, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @44, i32 0, i32 0), i32 3)
  %__str.load65 = load %string, %string* %__str12, align 8
  %36 = call i1 @_EN3stdeqE6string6string(%string %35, %string %__str.load65)
  %assert.condition66 = icmp eq i1 %36, false
  br i1 %assert.condition66, label %assert.fail67, label %assert.success68

assert.fail67:                                    ; preds = %assert.success63
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @45, i32 0, i32 0))
  unreachable

assert.success68:                                 ; preds = %assert.success63
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %12, i32 12, i32 16)
  %.load69 = load %"Range<int>", %"Range<int>"* %12, align 4
  %37 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s2, %"Range<int>" %.load69)
  call void @_EN3std6string4initEP4char3int(%string* %__str13, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @46, i32 0, i32 0), i32 4)
  %__str.load70 = load %string, %string* %__str13, align 8
  %38 = call i1 @_EN3stdeqE6string6string(%string %37, %string %__str.load70)
  %assert.condition71 = icmp eq i1 %38, false
  br i1 %assert.condition71, label %assert.fail72, label %assert.success73

assert.fail72:                                    ; preds = %assert.success68
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @47, i32 0, i32 0))
  unreachable

assert.success73:                                 ; preds = %assert.success68
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %13, i32 17, i32 25)
  %.load74 = load %"Range<int>", %"Range<int>"* %13, align 4
  %39 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s2, %"Range<int>" %.load74)
  call void @_EN3std6string4initEP4char3int(%string* %__str14, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @48, i32 0, i32 0), i32 8)
  %__str.load75 = load %string, %string* %__str14, align 8
  %40 = call i1 @_EN3stdeqE6string6string(%string %39, %string %__str.load75)
  %assert.condition76 = icmp eq i1 %40, false
  br i1 %assert.condition76, label %assert.fail77, label %assert.success78

assert.fail77:                                    ; preds = %assert.success73
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @49, i32 0, i32 0))
  unreachable

assert.success78:                                 ; preds = %assert.success73
  call void @_EN3std6string4initEP4char3int(%string* %__str15, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @50, i32 0, i32 0), i32 4)
  %__str.load79 = load %string, %string* %__str15, align 8
  call void @_EN3std12StringBuffer4initE6string(%StringBuffer* %s3, %string %__str.load79)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %14, i32 0, i32 0)
  %.load80 = load %"Range<int>", %"Range<int>"* %14, align 4
  %41 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %s3, %"Range<int>" %.load80)
  call void @_EN3std6string4initEP4char3int(%string* %__str16, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @51, i32 0, i32 0), i32 0)
  %__str.load81 = load %string, %string* %__str16, align 8
  %42 = call i1 @_EN3stdeqE6string6string(%string %41, %string %__str.load81)
  %assert.condition82 = icmp eq i1 %42, false
  br i1 %assert.condition82, label %assert.fail83, label %assert.success84

assert.fail83:                                    ; preds = %assert.success78
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @52, i32 0, i32 0))
  unreachable

assert.success84:                                 ; preds = %assert.success78
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %s3)
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %s2)
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %s)
  ret void
}

define void @_EN4main9testSplitE() {
  %s = alloca %StringBuffer, align 8
  %__str = alloca %string, align 8
  %words = alloca %"List<string>", align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %__str5 = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([26 x i8], [26 x i8]* @53, i32 0, i32 0), i32 25)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std12StringBuffer4initE6string(%StringBuffer* %s, %string %__str.load)
  %1 = call %"List<string>" @_EN3std12StringBuffer5splitE(%StringBuffer* %s)
  store %"List<string>" %1, %"List<string>"* %words, align 8
  %2 = call i32 @_EN3std4ListI6stringE4sizeE(%"List<string>"* %words)
  %3 = icmp eq i32 %2, 5
  %assert.condition = icmp eq i1 %3, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @54, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 0)
  %.load = load %string, %string* %4, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @55, i32 0, i32 0), i32 4)
  %__str.load6 = load %string, %string* %__str1, align 8
  %5 = call i1 @_EN3stdeqE6string6string(%string %.load, %string %__str.load6)
  %assert.condition7 = icmp eq i1 %5, false
  br i1 %assert.condition7, label %assert.fail8, label %assert.success9

assert.fail8:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @56, i32 0, i32 0))
  unreachable

assert.success9:                                  ; preds = %assert.success
  %6 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 1)
  %.load10 = load %string, %string* %6, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @57, i32 0, i32 0), i32 2)
  %__str.load11 = load %string, %string* %__str2, align 8
  %7 = call i1 @_EN3stdeqE6string6string(%string %.load10, %string %__str.load11)
  %assert.condition12 = icmp eq i1 %7, false
  br i1 %assert.condition12, label %assert.fail13, label %assert.success14

assert.fail13:                                    ; preds = %assert.success9
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @58, i32 0, i32 0))
  unreachable

assert.success14:                                 ; preds = %assert.success9
  %8 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 2)
  %.load15 = load %string, %string* %8, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @59, i32 0, i32 0), i32 3)
  %__str.load16 = load %string, %string* %__str3, align 8
  %9 = call i1 @_EN3stdeqE6string6string(%string %.load15, %string %__str.load16)
  %assert.condition17 = icmp eq i1 %9, false
  br i1 %assert.condition17, label %assert.fail18, label %assert.success19

assert.fail18:                                    ; preds = %assert.success14
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @60, i32 0, i32 0))
  unreachable

assert.success19:                                 ; preds = %assert.success14
  %10 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 3)
  %.load20 = load %string, %string* %10, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @61, i32 0, i32 0), i32 4)
  %__str.load21 = load %string, %string* %__str4, align 8
  %11 = call i1 @_EN3stdeqE6string6string(%string %.load20, %string %__str.load21)
  %assert.condition22 = icmp eq i1 %11, false
  br i1 %assert.condition22, label %assert.fail23, label %assert.success24

assert.fail23:                                    ; preds = %assert.success19
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @62, i32 0, i32 0))
  unreachable

assert.success24:                                 ; preds = %assert.success19
  %12 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 4)
  %.load25 = load %string, %string* %12, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @63, i32 0, i32 0), i32 8)
  %__str.load26 = load %string, %string* %__str5, align 8
  %13 = call i1 @_EN3stdeqE6string6string(%string %.load25, %string %__str.load26)
  %assert.condition27 = icmp eq i1 %13, false
  br i1 %assert.condition27, label %assert.fail28, label %assert.success29

assert.fail28:                                    ; preds = %assert.success24
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @64, i32 0, i32 0))
  unreachable

assert.success29:                                 ; preds = %assert.success24
  call void @_EN3std4ListI6stringE6deinitE(%"List<string>"* %words)
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %s)
  ret void
}

define void @_EN4main14testOtherSplitE() {
  %s = alloca %StringBuffer, align 8
  %__str = alloca %string, align 8
  %words = alloca %"List<string>", align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %__str5 = alloca %string, align 8
  %s2 = alloca %StringBuffer, align 8
  %__str6 = alloca %string, align 8
  %__str7 = alloca %string, align 8
  %__str8 = alloca %string, align 8
  %__str9 = alloca %string, align 8
  %__str10 = alloca %string, align 8
  %__str11 = alloca %string, align 8
  %s3 = alloca %StringBuffer, align 8
  %__str12 = alloca %string, align 8
  %__str13 = alloca %string, align 8
  %__str14 = alloca %string, align 8
  %__str15 = alloca %string, align 8
  %__str16 = alloca %string, align 8
  %__str17 = alloca %string, align 8
  %__str18 = alloca %string, align 8
  %__str19 = alloca %string, align 8
  %__str20 = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @65, i32 0, i32 0), i32 32)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std12StringBuffer4initE6string(%StringBuffer* %s, %string %__str.load)
  %1 = call %"List<string>" @_EN3std12StringBuffer5splitE4char(%StringBuffer* %s, i8 44)
  store %"List<string>" %1, %"List<string>"* %words, align 8
  %2 = call i32 @_EN3std4ListI6stringE4sizeE(%"List<string>"* %words)
  %3 = icmp eq i32 %2, 5
  %assert.condition = icmp eq i1 %3, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @66, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 0)
  %.load = load %string, %string* %4, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @67, i32 0, i32 0), i32 4)
  %__str.load21 = load %string, %string* %__str1, align 8
  %5 = call i1 @_EN3stdeqE6string6string(%string %.load, %string %__str.load21)
  %assert.condition22 = icmp eq i1 %5, false
  br i1 %assert.condition22, label %assert.fail23, label %assert.success24

assert.fail23:                                    ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @68, i32 0, i32 0))
  unreachable

assert.success24:                                 ; preds = %assert.success
  %6 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 1)
  %.load25 = load %string, %string* %6, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @69, i32 0, i32 0), i32 8)
  %__str.load26 = load %string, %string* %__str2, align 8
  %7 = call i1 @_EN3stdeqE6string6string(%string %.load25, %string %__str.load26)
  %assert.condition27 = icmp eq i1 %7, false
  br i1 %assert.condition27, label %assert.fail28, label %assert.success29

assert.fail28:                                    ; preds = %assert.success24
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @70, i32 0, i32 0))
  unreachable

assert.success29:                                 ; preds = %assert.success24
  %8 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 2)
  %.load30 = load %string, %string* %8, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @71, i32 0, i32 0), i32 2)
  %__str.load31 = load %string, %string* %__str3, align 8
  %9 = call i1 @_EN3stdeqE6string6string(%string %.load30, %string %__str.load31)
  %assert.condition32 = icmp eq i1 %9, false
  br i1 %assert.condition32, label %assert.fail33, label %assert.success34

assert.fail33:                                    ; preds = %assert.success29
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @72, i32 0, i32 0))
  unreachable

assert.success34:                                 ; preds = %assert.success29
  %10 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 3)
  %.load35 = load %string, %string* %10, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @73, i32 0, i32 0), i32 5)
  %__str.load36 = load %string, %string* %__str4, align 8
  %11 = call i1 @_EN3stdeqE6string6string(%string %.load35, %string %__str.load36)
  %assert.condition37 = icmp eq i1 %11, false
  br i1 %assert.condition37, label %assert.fail38, label %assert.success39

assert.fail38:                                    ; preds = %assert.success34
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @74, i32 0, i32 0))
  unreachable

assert.success39:                                 ; preds = %assert.success34
  %12 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 4)
  %.load40 = load %string, %string* %12, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @75, i32 0, i32 0), i32 9)
  %__str.load41 = load %string, %string* %__str5, align 8
  %13 = call i1 @_EN3stdeqE6string6string(%string %.load40, %string %__str.load41)
  %assert.condition42 = icmp eq i1 %13, false
  br i1 %assert.condition42, label %assert.fail43, label %assert.success44

assert.fail43:                                    ; preds = %assert.success39
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @76, i32 0, i32 0))
  unreachable

assert.success44:                                 ; preds = %assert.success39
  call void @_EN3std6string4initEP4char3int(%string* %__str6, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @77, i32 0, i32 0), i32 6)
  %__str.load45 = load %string, %string* %__str6, align 8
  call void @_EN3std12StringBuffer4initE6string(%StringBuffer* %s2, %string %__str.load45)
  call void @_EN3std4ListI6stringE6deinitE(%"List<string>"* %words)
  %14 = call %"List<string>" @_EN3std12StringBuffer5splitE4char(%StringBuffer* %s2, i8 44)
  store %"List<string>" %14, %"List<string>"* %words, align 8
  %15 = call i32 @_EN3std4ListI6stringE4sizeE(%"List<string>"* %words)
  %16 = icmp eq i32 %15, 5
  %assert.condition46 = icmp eq i1 %16, false
  br i1 %assert.condition46, label %assert.fail47, label %assert.success48

assert.fail47:                                    ; preds = %assert.success44
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @78, i32 0, i32 0))
  unreachable

assert.success48:                                 ; preds = %assert.success44
  %17 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 0)
  %.load49 = load %string, %string* %17, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str7, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @79, i32 0, i32 0), i32 0)
  %__str.load50 = load %string, %string* %__str7, align 8
  %18 = call i1 @_EN3stdeqE6string6string(%string %.load49, %string %__str.load50)
  %assert.condition51 = icmp eq i1 %18, false
  br i1 %assert.condition51, label %assert.fail52, label %assert.success53

assert.fail52:                                    ; preds = %assert.success48
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @80, i32 0, i32 0))
  unreachable

assert.success53:                                 ; preds = %assert.success48
  %19 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 1)
  %.load54 = load %string, %string* %19, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str8, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @81, i32 0, i32 0), i32 0)
  %__str.load55 = load %string, %string* %__str8, align 8
  %20 = call i1 @_EN3stdeqE6string6string(%string %.load54, %string %__str.load55)
  %assert.condition56 = icmp eq i1 %20, false
  br i1 %assert.condition56, label %assert.fail57, label %assert.success58

assert.fail57:                                    ; preds = %assert.success53
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @82, i32 0, i32 0))
  unreachable

assert.success58:                                 ; preds = %assert.success53
  %21 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 2)
  %.load59 = load %string, %string* %21, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str9, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @83, i32 0, i32 0), i32 1)
  %__str.load60 = load %string, %string* %__str9, align 8
  %22 = call i1 @_EN3stdeqE6string6string(%string %.load59, %string %__str.load60)
  %assert.condition61 = icmp eq i1 %22, false
  br i1 %assert.condition61, label %assert.fail62, label %assert.success63

assert.fail62:                                    ; preds = %assert.success58
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @84, i32 0, i32 0))
  unreachable

assert.success63:                                 ; preds = %assert.success58
  %23 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 3)
  %.load64 = load %string, %string* %23, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @85, i32 0, i32 0), i32 0)
  %__str.load65 = load %string, %string* %__str10, align 8
  %24 = call i1 @_EN3stdeqE6string6string(%string %.load64, %string %__str.load65)
  %assert.condition66 = icmp eq i1 %24, false
  br i1 %assert.condition66, label %assert.fail67, label %assert.success68

assert.fail67:                                    ; preds = %assert.success63
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @86, i32 0, i32 0))
  unreachable

assert.success68:                                 ; preds = %assert.success63
  %25 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 4)
  %.load69 = load %string, %string* %25, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str11, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @87, i32 0, i32 0), i32 1)
  %__str.load70 = load %string, %string* %__str11, align 8
  %26 = call i1 @_EN3stdeqE6string6string(%string %.load69, %string %__str.load70)
  %assert.condition71 = icmp eq i1 %26, false
  br i1 %assert.condition71, label %assert.fail72, label %assert.success73

assert.fail72:                                    ; preds = %assert.success68
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([42 x i8], [42 x i8]* @88, i32 0, i32 0))
  unreachable

assert.success73:                                 ; preds = %assert.success68
  call void @_EN3std6string4initEP4char3int(%string* %__str12, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @89, i32 0, i32 0), i32 9)
  %__str.load74 = load %string, %string* %__str12, align 8
  call void @_EN3std12StringBuffer4initE6string(%StringBuffer* %s3, %string %__str.load74)
  call void @_EN3std4ListI6stringE6deinitE(%"List<string>"* %words)
  %27 = call %"List<string>" @_EN3std12StringBuffer5splitE4char(%StringBuffer* %s3, i8 44)
  store %"List<string>" %27, %"List<string>"* %words, align 8
  %28 = call i32 @_EN3std4ListI6stringE4sizeE(%"List<string>"* %words)
  %29 = icmp eq i32 %28, 8
  %assert.condition75 = icmp eq i1 %29, false
  br i1 %assert.condition75, label %assert.fail76, label %assert.success77

assert.fail76:                                    ; preds = %assert.success73
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @90, i32 0, i32 0))
  unreachable

assert.success77:                                 ; preds = %assert.success73
  %30 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 0)
  %.load78 = load %string, %string* %30, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str13, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @91, i32 0, i32 0), i32 0)
  %__str.load79 = load %string, %string* %__str13, align 8
  %31 = call i1 @_EN3stdeqE6string6string(%string %.load78, %string %__str.load79)
  %assert.condition80 = icmp eq i1 %31, false
  br i1 %assert.condition80, label %assert.fail81, label %assert.success82

assert.fail81:                                    ; preds = %assert.success77
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @92, i32 0, i32 0))
  unreachable

assert.success82:                                 ; preds = %assert.success77
  %32 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 1)
  %.load83 = load %string, %string* %32, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str14, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @93, i32 0, i32 0), i32 0)
  %__str.load84 = load %string, %string* %__str14, align 8
  %33 = call i1 @_EN3stdeqE6string6string(%string %.load83, %string %__str.load84)
  %assert.condition85 = icmp eq i1 %33, false
  br i1 %assert.condition85, label %assert.fail86, label %assert.success87

assert.fail86:                                    ; preds = %assert.success82
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @94, i32 0, i32 0))
  unreachable

assert.success87:                                 ; preds = %assert.success82
  %34 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 2)
  %.load88 = load %string, %string* %34, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str15, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @95, i32 0, i32 0), i32 0)
  %__str.load89 = load %string, %string* %__str15, align 8
  %35 = call i1 @_EN3stdeqE6string6string(%string %.load88, %string %__str.load89)
  %assert.condition90 = icmp eq i1 %35, false
  br i1 %assert.condition90, label %assert.fail91, label %assert.success92

assert.fail91:                                    ; preds = %assert.success87
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @96, i32 0, i32 0))
  unreachable

assert.success92:                                 ; preds = %assert.success87
  %36 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 3)
  %.load93 = load %string, %string* %36, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str16, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @97, i32 0, i32 0), i32 1)
  %__str.load94 = load %string, %string* %__str16, align 8
  %37 = call i1 @_EN3stdeqE6string6string(%string %.load93, %string %__str.load94)
  %assert.condition95 = icmp eq i1 %37, false
  br i1 %assert.condition95, label %assert.fail96, label %assert.success97

assert.fail96:                                    ; preds = %assert.success92
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @98, i32 0, i32 0))
  unreachable

assert.success97:                                 ; preds = %assert.success92
  %38 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 4)
  %.load98 = load %string, %string* %38, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str17, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @99, i32 0, i32 0), i32 1)
  %__str.load99 = load %string, %string* %__str17, align 8
  %39 = call i1 @_EN3stdeqE6string6string(%string %.load98, %string %__str.load99)
  %assert.condition100 = icmp eq i1 %39, false
  br i1 %assert.condition100, label %assert.fail101, label %assert.success102

assert.fail101:                                   ; preds = %assert.success97
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @100, i32 0, i32 0))
  unreachable

assert.success102:                                ; preds = %assert.success97
  %40 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 5)
  %.load103 = load %string, %string* %40, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str18, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @101, i32 0, i32 0), i32 0)
  %__str.load104 = load %string, %string* %__str18, align 8
  %41 = call i1 @_EN3stdeqE6string6string(%string %.load103, %string %__str.load104)
  %assert.condition105 = icmp eq i1 %41, false
  br i1 %assert.condition105, label %assert.fail106, label %assert.success107

assert.fail106:                                   ; preds = %assert.success102
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @102, i32 0, i32 0))
  unreachable

assert.success107:                                ; preds = %assert.success102
  %42 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 6)
  %.load108 = load %string, %string* %42, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str19, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @103, i32 0, i32 0), i32 0)
  %__str.load109 = load %string, %string* %__str19, align 8
  %43 = call i1 @_EN3stdeqE6string6string(%string %.load108, %string %__str.load109)
  %assert.condition110 = icmp eq i1 %43, false
  br i1 %assert.condition110, label %assert.fail111, label %assert.success112

assert.fail111:                                   ; preds = %assert.success107
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @104, i32 0, i32 0))
  unreachable

assert.success112:                                ; preds = %assert.success107
  %44 = call %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %words, i32 7)
  %.load113 = load %string, %string* %44, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str20, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @105, i32 0, i32 0), i32 0)
  %__str.load114 = load %string, %string* %__str20, align 8
  %45 = call i1 @_EN3stdeqE6string6string(%string %.load113, %string %__str.load114)
  %assert.condition115 = icmp eq i1 %45, false
  br i1 %assert.condition115, label %assert.fail116, label %assert.success117

assert.fail116:                                   ; preds = %assert.success112
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @106, i32 0, i32 0))
  unreachable

assert.success117:                                ; preds = %assert.success112
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %s3)
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %s2)
  call void @_EN3std4ListI6stringE6deinitE(%"List<string>"* %words)
  call void @_EN3std12StringBuffer6deinitE(%StringBuffer* %s)
  ret void
}

define void @_EN4main9testBytesE() {
  %text = alloca %string, align 8
  %__str = alloca %string, align 8
  %i = alloca i32, align 4
  %__iterator = alloca %ByteIterator, align 8
  %byte = alloca i8, align 1
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([44 x i8], [44 x i8]* @107, i32 0, i32 0), i32 43)
  %__str.load = load %string, %string* %__str, align 8
  store %string %__str.load, %string* %text, align 8
  store i32 0, i32* %i, align 4
  %1 = call %ByteIterator @_EN3std6string5bytesE(%string* %text)
  store %ByteIterator %1, %ByteIterator* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std12ByteIterator8hasValueE(%ByteIterator* %__iterator)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call i8 @_EN3std12ByteIterator5valueE(%ByteIterator* %__iterator)
  store i8 %3, i8* %byte, align 1
  %byte.load = load i8, i8* %byte, align 1
  %i.load = load i32, i32* %i, align 4
  %4 = call i8 @_EN3std6stringixE3int(%string* %text, i32 %i.load)
  %5 = icmp eq i8 %byte.load, %4
  %assert.condition = icmp eq i1 %5, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

loop.increment:                                   ; preds = %assert.success
  call void @_EN3std12ByteIterator9incrementE(%ByteIterator* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %i.load1 = load i32, i32* %i, align 4
  %6 = call i32 @_EN3std6string4sizeE(%string* %text)
  %7 = icmp eq i32 %i.load1, %6
  %assert.condition2 = icmp eq i1 %7, false
  br i1 %assert.condition2, label %assert.fail4, label %assert.success5

assert.fail:                                      ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @108, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.body
  %i.load3 = load i32, i32* %i, align 4
  %8 = add i32 %i.load3, 1
  store i32 %8, i32* %i, align 4
  br label %loop.increment

assert.fail4:                                     ; preds = %loop.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @109, i32 0, i32 0))
  unreachable

assert.success5:                                  ; preds = %loop.end
  ret void
}

define void @_EN4main9testLinesE() {
  %text = alloca %string, align 8
  %__str = alloca %string, align 8
  %expected = alloca [5 x %string], align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %__str5 = alloca %string, align 8
  %i = alloca i32, align 4
  %__iterator = alloca %LineIterator, align 8
  %line = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([50 x i8], [50 x i8]* @110, i32 0, i32 0), i32 49)
  %__str.load = load %string, %string* %__str, align 8
  store %string %__str.load, %string* %text, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([21 x i8], [21 x i8]* @111, i32 0, i32 0), i32 20)
  %__str.load6 = load %string, %string* %__str1, align 8
  %1 = insertvalue [5 x %string] undef, %string %__str.load6, 0
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @112, i32 0, i32 0), i32 0)
  %__str.load7 = load %string, %string* %__str2, align 8
  %2 = insertvalue [5 x %string] %1, %string %__str.load7, 1
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @113, i32 0, i32 0), i32 0)
  %__str.load8 = load %string, %string* %__str3, align 8
  %3 = insertvalue [5 x %string] %2, %string %__str.load8, 2
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @114, i32 0, i32 0), i32 11)
  %__str.load9 = load %string, %string* %__str4, align 8
  %4 = insertvalue [5 x %string] %3, %string %__str.load9, 3
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @115, i32 0, i32 0), i32 13)
  %__str.load10 = load %string, %string* %__str5, align 8
  %5 = insertvalue [5 x %string] %4, %string %__str.load10, 4
  store [5 x %string] %5, [5 x %string]* %expected, align 8
  store i32 0, i32* %i, align 4
  %6 = call %LineIterator @_EN3std6string5linesE(%string* %text)
  store %LineIterator %6, %LineIterator* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %7 = call i1 @_EN3std12LineIterator8hasValueE(%LineIterator* %__iterator)
  br i1 %7, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %8 = call %string @_EN3std12LineIterator5valueE(%LineIterator* %__iterator)
  store %string %8, %string* %line, align 8
  %line.load = load %string, %string* %line, align 8
  %i.load = load i32, i32* %i, align 4
  %9 = getelementptr inbounds [5 x %string], [5 x %string]* %expected, i32 0, i32 %i.load
  %.load = load %string, %string* %9, align 8
  %10 = call i1 @_EN3stdeqE6string6string(%string %line.load, %string %.load)
  %assert.condition = icmp eq i1 %10, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

loop.increment:                                   ; preds = %assert.success
  call void @_EN3std12LineIterator9incrementE(%LineIterator* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %i.load11 = load i32, i32* %i, align 4
  %11 = icmp eq i32 %i.load11, 5
  %assert.condition12 = icmp eq i1 %11, false
  br i1 %assert.condition12, label %assert.fail14, label %assert.success15

assert.fail:                                      ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @116, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %loop.body
  %i.load13 = load i32, i32* %i, align 4
  %12 = add i32 %i.load13, 1
  store i32 %12, i32* %i, align 4
  br label %loop.increment

assert.fail14:                                    ; preds = %loop.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @117, i32 0, i32 0))
  unreachable

assert.success15:                                 ; preds = %loop.end
  ret void
}

define void @_EN4main12testParseIntE() {
  %__str = alloca %string, align 8
  %1 = alloca %"Optional<int>", align 8
  %2 = alloca %"Optional<int>", align 8
  %__str1 = alloca %string, align 8
  %3 = alloca %"Optional<int>", align 8
  %4 = alloca %"Optional<int>", align 8
  %__str2 = alloca %string, align 8
  %5 = alloca %"Optional<int>", align 8
  %6 = alloca %"Optional<int>", align 8
  %__str3 = alloca %string, align 8
  %7 = alloca %"Optional<int>", align 8
  %8 = alloca i32, align 4
  %__str4 = alloca %string, align 8
  %9 = alloca %"Optional<int>", align 8
  %10 = alloca i32, align 4
  %__str5 = alloca %string, align 8
  %11 = alloca %"Optional<int>", align 8
  %12 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @118, i32 0, i32 0), i32 0)
  %13 = call %"Optional<int>" @_EN3std6string8parseIntE(%string* %__str)
  store %"Optional<int>" %13, %"Optional<int>"* %1, align 4
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %2)
  %14 = call i1 @_EN3stdeqI3intEEPO3intPO3int(%"Optional<int>"* %1, %"Optional<int>"* %2)
  %assert.condition = icmp eq i1 %14, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @119, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @120, i32 0, i32 0), i32 1)
  %15 = call %"Optional<int>" @_EN3std6string8parseIntE(%string* %__str1)
  store %"Optional<int>" %15, %"Optional<int>"* %3, align 4
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %4)
  %16 = call i1 @_EN3stdeqI3intEEPO3intPO3int(%"Optional<int>"* %3, %"Optional<int>"* %4)
  %assert.condition6 = icmp eq i1 %16, false
  br i1 %assert.condition6, label %assert.fail7, label %assert.success8

assert.fail7:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @121, i32 0, i32 0))
  unreachable

assert.success8:                                  ; preds = %assert.success
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @122, i32 0, i32 0), i32 2)
  %17 = call %"Optional<int>" @_EN3std6string8parseIntE(%string* %__str2)
  store %"Optional<int>" %17, %"Optional<int>"* %5, align 4
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %6)
  %18 = call i1 @_EN3stdeqI3intEEPO3intPO3int(%"Optional<int>"* %5, %"Optional<int>"* %6)
  %assert.condition9 = icmp eq i1 %18, false
  br i1 %assert.condition9, label %assert.fail10, label %assert.success11

assert.fail10:                                    ; preds = %assert.success8
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @123, i32 0, i32 0))
  unreachable

assert.success11:                                 ; preds = %assert.success8
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @124, i32 0, i32 0), i32 1)
  %19 = call %"Optional<int>" @_EN3std6string8parseIntE(%string* %__str3)
  store %"Optional<int>" %19, %"Optional<int>"* %7, align 4
  store i32 1, i32* %8, align 4
  %20 = call i1 @_EN3stdeqI3intEEPO3intP3int(%"Optional<int>"* %7, i32* %8)
  %assert.condition12 = icmp eq i1 %20, false
  br i1 %assert.condition12, label %assert.fail13, label %assert.success14

assert.fail13:                                    ; preds = %assert.success11
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @125, i32 0, i32 0))
  unreachable

assert.success14:                                 ; preds = %assert.success11
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @126, i32 0, i32 0), i32 6)
  %21 = call %"Optional<int>" @_EN3std6string8parseIntE(%string* %__str4)
  store %"Optional<int>" %21, %"Optional<int>"* %9, align 4
  store i32 123789, i32* %10, align 4
  %22 = call i1 @_EN3stdeqI3intEEPO3intP3int(%"Optional<int>"* %9, i32* %10)
  %assert.condition15 = icmp eq i1 %22, false
  br i1 %assert.condition15, label %assert.fail16, label %assert.success17

assert.fail16:                                    ; preds = %assert.success14
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @127, i32 0, i32 0))
  unreachable

assert.success17:                                 ; preds = %assert.success14
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @128, i32 0, i32 0), i32 10)
  %23 = call %"Optional<int>" @_EN3std6string8parseIntE(%string* %__str5)
  store %"Optional<int>" %23, %"Optional<int>"* %11, align 4
  store i32 2147483647, i32* %12, align 4
  %24 = call i1 @_EN3stdeqI3intEEPO3intP3int(%"Optional<int>"* %11, i32* %12)
  %assert.condition18 = icmp eq i1 %24, false
  br i1 %assert.condition18, label %assert.fail19, label %assert.success20

assert.fail19:                                    ; preds = %assert.success17
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @129, i32 0, i32 0))
  unreachable

assert.success20:                                 ; preds = %assert.success17
  ret void
}

define void @_EN4main10testEscapeE() {
  %__str = alloca %string, align 8
  %1 = alloca %StringBuffer, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %2 = alloca %StringBuffer, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %3 = alloca %StringBuffer, align 8
  %__str5 = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @130, i32 0, i32 0), i32 0)
  %4 = call %StringBuffer @_EN3std6string6escapeE(%string* %__str)
  store %StringBuffer %4, %StringBuffer* %1, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @131, i32 0, i32 0), i32 0)
  %__str.load = load %string, %string* %__str1, align 8
  %5 = call i1 @_EN3stdeqEP12StringBuffer6string(%StringBuffer* %1, %string %__str.load)
  %assert.condition = icmp eq i1 %5, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @132, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @133, i32 0, i32 0), i32 9)
  %6 = call %StringBuffer @_EN3std6string6escapeE(%string* %__str2)
  store %StringBuffer %6, %StringBuffer* %2, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @134, i32 0, i32 0), i32 9)
  %__str.load6 = load %string, %string* %__str3, align 8
  %7 = call i1 @_EN3stdeqEP12StringBuffer6string(%StringBuffer* %2, %string %__str.load6)
  %assert.condition7 = icmp eq i1 %7, false
  br i1 %assert.condition7, label %assert.fail8, label %assert.success9

assert.fail8:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @135, i32 0, i32 0))
  unreachable

assert.success9:                                  ; preds = %assert.success
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @136, i32 0, i32 0), i32 13)
  %8 = call %StringBuffer @_EN3std6string6escapeE(%string* %__str4)
  store %StringBuffer %8, %StringBuffer* %3, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @137, i32 0, i32 0), i32 19)
  %__str.load10 = load %string, %string* %__str5, align 8
  %9 = call i1 @_EN3stdeqEP12StringBuffer6string(%StringBuffer* %3, %string %__str.load10)
  %assert.condition11 = icmp eq i1 %9, false
  br i1 %assert.condition11, label %assert.fail12, label %assert.success13

assert.fail12:                                    ; preds = %assert.success9
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @138, i32 0, i32 0))
  unreachable

assert.success13:                                 ; preds = %assert.success9
  ret void
}

define void @_EN4main10testRepeatE() {
  %__str = alloca %string, align 8
  %1 = alloca %StringBuffer, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %2 = alloca %StringBuffer, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %3 = alloca %StringBuffer, align 8
  %__str5 = alloca %string, align 8
  %__str6 = alloca %string, align 8
  %4 = alloca %StringBuffer, align 8
  %__str7 = alloca %string, align 8
  %__str8 = alloca %string, align 8
  %5 = alloca %StringBuffer, align 8
  %__str9 = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @139, i32 0, i32 0), i32 3)
  %6 = call %StringBuffer @_EN3std6string6repeatE3int(%string* %__str, i32 3)
  store %StringBuffer %6, %StringBuffer* %1, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @140, i32 0, i32 0), i32 9)
  %__str.load = load %string, %string* %__str1, align 8
  %7 = call i1 @_EN3stdeqEP12StringBuffer6string(%StringBuffer* %1, %string %__str.load)
  %assert.condition = icmp eq i1 %7, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @141, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @142, i32 0, i32 0), i32 1)
  %8 = call %StringBuffer @_EN3std6string6repeatE3int(%string* %__str2, i32 5)
  store %StringBuffer %8, %StringBuffer* %2, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @143, i32 0, i32 0), i32 5)
  %__str.load10 = load %string, %string* %__str3, align 8
  %9 = call i1 @_EN3stdeqEP12StringBuffer6string(%StringBuffer* %2, %string %__str.load10)
  %assert.condition11 = icmp eq i1 %9, false
  br i1 %assert.condition11, label %assert.fail12, label %assert.success13

assert.fail12:                                    ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @144, i32 0, i32 0))
  unreachable

assert.success13:                                 ; preds = %assert.success
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @145, i32 0, i32 0), i32 0)
  %10 = call %StringBuffer @_EN3std6string6repeatE3int(%string* %__str4, i32 3)
  store %StringBuffer %10, %StringBuffer* %3, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @146, i32 0, i32 0), i32 0)
  %__str.load14 = load %string, %string* %__str5, align 8
  %11 = call i1 @_EN3stdeqEP12StringBuffer6string(%StringBuffer* %3, %string %__str.load14)
  %assert.condition15 = icmp eq i1 %11, false
  br i1 %assert.condition15, label %assert.fail16, label %assert.success17

assert.fail16:                                    ; preds = %assert.success13
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @147, i32 0, i32 0))
  unreachable

assert.success17:                                 ; preds = %assert.success13
  call void @_EN3std6string4initEP4char3int(%string* %__str6, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @148, i32 0, i32 0), i32 3)
  %12 = call %StringBuffer @_EN3std6string6repeatE3int(%string* %__str6, i32 0)
  store %StringBuffer %12, %StringBuffer* %4, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str7, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @149, i32 0, i32 0), i32 0)
  %__str.load18 = load %string, %string* %__str7, align 8
  %13 = call i1 @_EN3stdeqEP12StringBuffer6string(%StringBuffer* %4, %string %__str.load18)
  %assert.condition19 = icmp eq i1 %13, false
  br i1 %assert.condition19, label %assert.fail20, label %assert.success21

assert.fail20:                                    ; preds = %assert.success17
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @150, i32 0, i32 0))
  unreachable

assert.success21:                                 ; preds = %assert.success17
  call void @_EN3std6string4initEP4char3int(%string* %__str8, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @151, i32 0, i32 0), i32 3)
  %14 = call %StringBuffer @_EN3std6string6repeatE3int(%string* %__str8, i32 1)
  store %StringBuffer %14, %StringBuffer* %5, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str9, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @152, i32 0, i32 0), i32 3)
  %__str.load22 = load %string, %string* %__str9, align 8
  %15 = call i1 @_EN3stdeqEP12StringBuffer6string(%StringBuffer* %5, %string %__str.load22)
  %assert.condition23 = icmp eq i1 %15, false
  br i1 %assert.condition23, label %assert.fail24, label %assert.success25

assert.fail24:                                    ; preds = %assert.success21
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([43 x i8], [43 x i8]* @153, i32 0, i32 0))
  unreachable

assert.success25:                                 ; preds = %assert.success21
  ret void
}

declare i32 @_EN3std6string7compareEP6string(%string*, %string*)

declare %StringIterator @_EN3std6string8iteratorE(%string*)

declare i1 @_EN3std14StringIterator8hasValueE(%StringIterator*)

declare i8 @_EN3std14StringIterator5valueE(%StringIterator*)

declare void @_EN3std14StringIterator9incrementE(%StringIterator*)

declare void @_EN3std12StringBuffer4initE6string(%StringBuffer*, %string)

define i32 @_EN3std12StringBuffer4findE4char(%StringBuffer* %this, i8 %c) {
  %1 = call i32 @_EN3std12StringBuffer4findE4char3int(%StringBuffer* %this, i8 %c, i32 0)
  ret i32 %1
}

define i32 @_EN3std12StringBuffer4findE4char3int(%StringBuffer* %this, i8 %c, i32 %start) {
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %2 = call i32 @_EN3std12StringBuffer4sizeE(%StringBuffer* %this)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 %start, i32 %2)
  %3 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %3, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %4 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %5 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %5, i32* %i, align 4
  %characters = getelementptr inbounds %StringBuffer, %StringBuffer* %this, i32 0, i32 0
  %i.load = load i32, i32* %i, align 4
  %6 = call i8* @_EN3std4ListI4charEixE3int(%"List<char>"* %characters, i32 %i.load)
  %.load = load i8, i8* %6, align 1
  %7 = icmp eq i8 %.load, %c
  br i1 %7, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %8 = call i32 @_EN3std12StringBuffer4sizeE(%StringBuffer* %this)
  ret i32 %8

if.then:                                          ; preds = %loop.body
  %i.load1 = load i32, i32* %i, align 4
  ret i32 %i.load1

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.increment
}

declare i32 @_EN3std12StringBuffer4sizeE(%StringBuffer*)

declare void @_EN3std12StringBuffer6deinitE(%StringBuffer*)

declare void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"*, i32, i32)

define %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %this, %"Range<int>" %range) {
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %1 = alloca %string, align 8
  %2 = alloca %"Range<int>", align 8
  %start = extractvalue %"Range<int>" %range, 0
  %3 = icmp slt i32 %start, 0
  br i1 %3, label %or.end, label %or.rhs

or.rhs:                                           ; preds = %0
  %start2 = extractvalue %"Range<int>" %range, 0
  %4 = call i32 @_EN3std12StringBuffer4sizeE(%StringBuffer* %this)
  %5 = icmp sgt i32 %start2, %4
  br label %or.end

or.end:                                           ; preds = %or.rhs, %0
  %or = phi i1 [ %3, %0 ], [ %5, %or.rhs ]
  br i1 %or, label %if.then, label %if.else

if.then:                                          ; preds = %or.end
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @154, i32 0, i32 0), i32 6)
  %__str.load = load %string, %string* %__str, align 8
  %start3 = extractvalue %"Range<int>" %range, 0
  call void @_EN3std12StringBuffer16indexOutOfBoundsE6string3int(%StringBuffer* %this, %string %__str.load, i32 %start3)
  br label %if.end

if.else:                                          ; preds = %or.end
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %end = extractvalue %"Range<int>" %range, 1
  %6 = icmp slt i32 %end, 0
  br i1 %6, label %or.end6, label %or.rhs4

or.rhs4:                                          ; preds = %if.end
  %end5 = extractvalue %"Range<int>" %range, 1
  %7 = call i32 @_EN3std12StringBuffer4sizeE(%StringBuffer* %this)
  %8 = icmp sgt i32 %end5, %7
  br label %or.end6

or.end6:                                          ; preds = %or.rhs4, %if.end
  %or7 = phi i1 [ %6, %if.end ], [ %8, %or.rhs4 ]
  br i1 %or7, label %if.then8, label %if.else11

if.then8:                                         ; preds = %or.end6
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @155, i32 0, i32 0), i32 6)
  %__str.load9 = load %string, %string* %__str1, align 8
  %end10 = extractvalue %"Range<int>" %range, 1
  call void @_EN3std12StringBuffer16indexOutOfBoundsE6string3int(%StringBuffer* %this, %string %__str.load9, i32 %end10)
  br label %if.end12

if.else11:                                        ; preds = %or.end6
  br label %if.end12

if.end12:                                         ; preds = %if.else11, %if.then8
  %characters = getelementptr inbounds %StringBuffer, %StringBuffer* %this, i32 0, i32 0
  %9 = call i8* @_EN3std4ListI4charE4dataE(%"List<char>"* %characters)
  %start13 = extractvalue %"Range<int>" %range, 0
  %10 = getelementptr inbounds i8, i8* %9, i32 %start13
  store %"Range<int>" %range, %"Range<int>"* %2, align 4
  %11 = call i32 @_EN3std5RangeI3intE4sizeE(%"Range<int>"* %2)
  call void @_EN3std6string4initEP4char3int(%string* %1, i8* %10, i32 %11)
  %.load = load %string, %string* %1, align 8
  ret %string %.load
}

define %"List<string>" @_EN3std12StringBuffer5splitE(%StringBuffer* %this) {
  %1 = call %"List<string>" @_EN3std12StringBuffer5splitE4char(%StringBuffer* %this, i8 32)
  ret %"List<string>" %1
}

define i32 @_EN3std4ListI6stringE4sizeE(%"List<string>"* %this) {
  %size = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %string* @_EN3std4ListI6stringEixE3int(%"List<string>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI6stringE16indexOutOfBoundsE3int(%"List<string>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 0
  %buffer.load = load %string*, %string** %buffer, align 8
  %2 = getelementptr inbounds %string, %string* %buffer.load, i32 %index
  ret %string* %2
}

define void @_EN3std4ListI6stringE6deinitE(%"List<string>"* %this) {
  %__iterator = alloca %"ArrayIterator<string>", align 8
  %element = alloca %string*, align 8
  %capacity = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<string>" @_EN3std4ListI6stringE8iteratorE(%"List<string>"* %this)
  store %"ArrayIterator<string>" %2, %"ArrayIterator<string>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI6stringE8hasValueE(%"ArrayIterator<string>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call %string* @_EN3std13ArrayIteratorI6stringE5valueE(%"ArrayIterator<string>"* %__iterator)
  store %string* %4, %string** %element, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI6stringE9incrementE(%"ArrayIterator<string>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 0
  %buffer.load = load %string*, %string** %buffer, align 8
  call void @_EN3std10deallocateIAU_6stringEEAU_6string(%string* %buffer.load)
  br label %if.end
}

define %"List<string>" @_EN3std12StringBuffer5splitE4char(%StringBuffer* %this, i8 %delim) {
  %tokens = alloca %"List<string>", align 8
  %prev = alloca i32, align 4
  %pos = alloca i32, align 4
  %token = alloca %string, align 8
  %1 = alloca %"Range<int>", align 8
  call void @_EN3std4ListI6stringE4initE(%"List<string>"* %tokens)
  store i32 0, i32* %prev, align 4
  store i32 0, i32* %pos, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %if.end, %0
  br i1 true, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %prev.load = load i32, i32* %prev, align 4
  %2 = call i32 @_EN3std12StringBuffer4findE4char3int(%StringBuffer* %this, i8 %delim, i32 %prev.load)
  store i32 %2, i32* %pos, align 4
  %prev.load1 = load i32, i32* %prev, align 4
  %pos.load = load i32, i32* %pos, align 4
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 %prev.load1, i32 %pos.load)
  %.load = load %"Range<int>", %"Range<int>"* %1, align 4
  %3 = call %string @_EN3std12StringBuffer6substrE5RangeI3intE(%StringBuffer* %this, %"Range<int>" %.load)
  store %string %3, %string* %token, align 8
  %token.load = load %string, %string* %token, align 8
  call void @_EN3std4ListI6stringE4pushE6string(%"List<string>"* %tokens, %string %token.load)
  %pos.load2 = load i32, i32* %pos, align 4
  %4 = add i32 %pos.load2, 1
  store i32 %4, i32* %prev, align 4
  %pos.load3 = load i32, i32* %pos, align 4
  %5 = call i32 @_EN3std12StringBuffer4sizeE(%StringBuffer* %this)
  %6 = icmp sgt i32 %pos.load3, %5
  br i1 %6, label %or.end, label %or.rhs

loop.end:                                         ; preds = %if.then, %loop.condition
  %tokens.load = load %"List<string>", %"List<string>"* %tokens, align 8
  ret %"List<string>" %tokens.load

or.rhs:                                           ; preds = %loop.body
  %prev.load4 = load i32, i32* %prev, align 4
  %7 = call i32 @_EN3std12StringBuffer4sizeE(%StringBuffer* %this)
  %8 = icmp sgt i32 %prev.load4, %7
  br label %or.end

or.end:                                           ; preds = %or.rhs, %loop.body
  %or = phi i1 [ %6, %loop.body ], [ %8, %or.rhs ]
  br i1 %or, label %if.then, label %if.else

if.then:                                          ; preds = %or.end
  br label %loop.end

if.else:                                          ; preds = %or.end
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.condition
}

define %ByteIterator @_EN3std6string5bytesE(%string* %this) {
  %1 = alloca %ByteIterator, align 8
  %this.load = load %string, %string* %this, align 8
  call void @_EN3std12ByteIterator4initE6string(%ByteIterator* %1, %string %this.load)
  %.load = load %ByteIterator, %ByteIterator* %1, align 8
  ret %ByteIterator %.load
}

define i1 @_EN3std12ByteIterator8hasValueE(%ByteIterator* %this) {
  %stream = getelementptr inbounds %ByteIterator, %ByteIterator* %this, i32 0, i32 0
  %1 = call i1 @_EN3std6string5emptyE(%string* %stream)
  %2 = xor i1 %1, true
  ret i1 %2
}

define i8 @_EN3std12ByteIterator5valueE(%ByteIterator* %this) {
  %stream = getelementptr inbounds %ByteIterator, %ByteIterator* %this, i32 0, i32 0
  %1 = call i8 @_EN3std6string5frontE(%string* %stream)
  ret i8 %1
}

declare i8 @_EN3std6stringixE3int(%string*, i32)

define void @_EN3std12ByteIterator9incrementE(%ByteIterator* %this) {
  %stream = getelementptr inbounds %ByteIterator, %ByteIterator* %this, i32 0, i32 0
  %stream1 = getelementptr inbounds %ByteIterator, %ByteIterator* %this, i32 0, i32 0
  %1 = call %string @_EN3std6string6substrE3int(%string* %stream1, i32 1)
  store %string %1, %string* %stream, align 8
  ret void
}

declare i32 @_EN3std6string4sizeE(%string*)

define %LineIterator @_EN3std6string5linesE(%string* %this) {
  %1 = alloca %LineIterator, align 8
  %this.load = load %string, %string* %this, align 8
  call void @_EN3std12LineIterator4initE6string(%LineIterator* %1, %string %this.load)
  %.load = load %LineIterator, %LineIterator* %1, align 8
  ret %LineIterator %.load
}

define i1 @_EN3std12LineIterator8hasValueE(%LineIterator* %this) {
  %stream = getelementptr inbounds %LineIterator, %LineIterator* %this, i32 0, i32 0
  %1 = call i1 @_EN3std6string5emptyE(%string* %stream)
  %2 = xor i1 %1, true
  ret i1 %2
}

define %string @_EN3std12LineIterator5valueE(%LineIterator* %this) {
  %1 = alloca %"Range<int>", align 8
  %stream = getelementptr inbounds %LineIterator, %LineIterator* %this, i32 0, i32 0
  %end = getelementptr inbounds %LineIterator, %LineIterator* %this, i32 0, i32 1
  %end.load = load i32, i32* %end, align 4
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 %end.load)
  %.load = load %"Range<int>", %"Range<int>"* %1, align 4
  %2 = call %string @_EN3std6string6substrE5RangeI3intE(%string* %stream, %"Range<int>" %.load)
  ret %string %2
}

define void @_EN3std12LineIterator9incrementE(%LineIterator* %this) {
  %stream = getelementptr inbounds %LineIterator, %LineIterator* %this, i32 0, i32 0
  %stream1 = getelementptr inbounds %LineIterator, %LineIterator* %this, i32 0, i32 0
  %end = getelementptr inbounds %LineIterator, %LineIterator* %this, i32 0, i32 1
  %end.load = load i32, i32* %end, align 4
  %1 = add i32 %end.load, 1
  %2 = call %string @_EN3std6string6substrE3int(%string* %stream1, i32 %1)
  store %string %2, %string* %stream, align 8
  %end2 = getelementptr inbounds %LineIterator, %LineIterator* %this, i32 0, i32 1
  %stream3 = getelementptr inbounds %LineIterator, %LineIterator* %this, i32 0, i32 0
  %3 = call i32 @_EN3std6string4findE4char(%string* %stream3, i8 10)
  store i32 %3, i32* %end2, align 4
  ret void
}

define %"Optional<int>" @_EN3std6string8parseIntE(%string* %this) {
  %1 = alloca %"Optional<int>", align 8
  %result = alloca i32, align 4
  %__iterator = alloca %StringIterator, align 8
  %c = alloca i8, align 1
  %2 = alloca %"Optional<int>", align 8
  %3 = alloca %"Optional<int>", align 8
  %4 = call i1 @_EN3std6string5emptyE(%string* %this)
  br i1 %4, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %1)
  %.load = load %"Optional<int>", %"Optional<int>"* %1, align 4
  ret %"Optional<int>" %.load

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  store i32 0, i32* %result, align 4
  %5 = call %StringIterator @_EN3std6string8iteratorE(%string* %this)
  store %StringIterator %5, %StringIterator* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %if.end
  %6 = call i1 @_EN3std14StringIterator8hasValueE(%StringIterator* %__iterator)
  br i1 %6, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %7 = call i8 @_EN3std14StringIterator5valueE(%StringIterator* %__iterator)
  store i8 %7, i8* %c, align 1
  %c.load = load i8, i8* %c, align 1
  %8 = icmp uge i8 %c.load, 48
  br i1 %8, label %and.rhs, label %and.end

loop.increment:                                   ; preds = %if.end9
  call void @_EN3std14StringIterator9incrementE(%StringIterator* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %result.load = load i32, i32* %result, align 4
  call void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %3, i32 %result.load)
  %.load1 = load %"Optional<int>", %"Optional<int>"* %3, align 4
  ret %"Optional<int>" %.load1

and.rhs:                                          ; preds = %loop.body
  %c.load2 = load i8, i8* %c, align 1
  %9 = icmp ule i8 %c.load2, 57
  br label %and.end

and.end:                                          ; preds = %and.rhs, %loop.body
  %and = phi i1 [ %8, %loop.body ], [ %9, %and.rhs ]
  br i1 %and, label %if.then3, label %if.else7

if.then3:                                         ; preds = %and.end
  %result.load4 = load i32, i32* %result, align 4
  %10 = mul i32 %result.load4, 10
  store i32 %10, i32* %result, align 4
  %result.load5 = load i32, i32* %result, align 4
  %c.load6 = load i8, i8* %c, align 1
  %11 = sub i8 %c.load6, 48
  %12 = zext i8 %11 to i32
  %13 = add i32 %result.load5, %12
  store i32 %13, i32* %result, align 4
  br label %if.end9

if.else7:                                         ; preds = %and.end
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %2)
  %.load8 = load %"Optional<int>", %"Optional<int>"* %2, align 4
  ret %"Optional<int>" %.load8

if.end9:                                          ; preds = %if.then3
  br label %loop.increment
}

define void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %this) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 0
  store i1 false, i1* %hasValue, align 1
  ret void
}

define i1 @_EN3stdeqI3intEEPO3intPO3int(%"Optional<int>"* %a, %"Optional<int>"* %b) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %a, i32 0, i32 0
  %hasValue.load = load i1, i1* %hasValue, align 1
  %hasValue1 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %b, i32 0, i32 0
  %hasValue.load2 = load i1, i1* %hasValue1, align 1
  %1 = icmp ne i1 %hasValue.load, %hasValue.load2
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %hasValue3 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %b, i32 0, i32 0
  %hasValue.load4 = load i1, i1* %hasValue3, align 1
  %2 = xor i1 %hasValue.load4, true
  br i1 %2, label %if.then5, label %if.else6

if.then5:                                         ; preds = %if.end
  ret i1 true

if.else6:                                         ; preds = %if.end
  br label %if.end7

if.end7:                                          ; preds = %if.else6
  %value = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %a, i32 0, i32 1
  %value.load = load i32, i32* %value, align 4
  %value8 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %b, i32 0, i32 1
  %value.load9 = load i32, i32* %value8, align 4
  %3 = icmp eq i32 %value.load, %value.load9
  ret i1 %3
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

define %StringBuffer @_EN3std6string6escapeE(%string* %this) {
  %result = alloca %StringBuffer, align 8
  %__iterator = alloca %StringIterator, align 8
  %c = alloca i8, align 1
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %__str2 = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %__str5 = alloca %string, align 8
  call void @_EN3std12StringBuffer4initE(%StringBuffer* %result)
  %1 = call %StringIterator @_EN3std6string8iteratorE(%string* %this)
  store %StringIterator %1, %StringIterator* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std14StringIterator8hasValueE(%StringIterator* %__iterator)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call i8 @_EN3std14StringIterator5valueE(%StringIterator* %__iterator)
  store i8 %3, i8* %c, align 1
  %c.load = load i8, i8* %c, align 1
  switch i8 %c.load, label %switch.default [
    i8 0, label %switch.case.0
    i8 10, label %switch.case.1
    i8 13, label %switch.case.2
    i8 9, label %switch.case.3
    i8 34, label %switch.case.4
    i8 92, label %switch.case.5
  ]

loop.increment:                                   ; preds = %switch.end
  call void @_EN3std14StringIterator9incrementE(%StringIterator* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %result.load = load %StringBuffer, %StringBuffer* %result, align 8
  ret %StringBuffer %result.load

switch.case.0:                                    ; preds = %loop.body
  %result.load6 = load %StringBuffer, %StringBuffer* %result, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @156, i32 0, i32 0), i32 2)
  %__str.load = load %string, %string* %__str, align 8
  %4 = call %StringBuffer @_EN3stdplE12StringBuffer6string(%StringBuffer %result.load6, %string %__str.load)
  store %StringBuffer %4, %StringBuffer* %result, align 8
  br label %switch.end

switch.case.1:                                    ; preds = %loop.body
  %result.load7 = load %StringBuffer, %StringBuffer* %result, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @157, i32 0, i32 0), i32 2)
  %__str.load8 = load %string, %string* %__str1, align 8
  %5 = call %StringBuffer @_EN3stdplE12StringBuffer6string(%StringBuffer %result.load7, %string %__str.load8)
  store %StringBuffer %5, %StringBuffer* %result, align 8
  br label %switch.end

switch.case.2:                                    ; preds = %loop.body
  %result.load9 = load %StringBuffer, %StringBuffer* %result, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @158, i32 0, i32 0), i32 2)
  %__str.load10 = load %string, %string* %__str2, align 8
  %6 = call %StringBuffer @_EN3stdplE12StringBuffer6string(%StringBuffer %result.load9, %string %__str.load10)
  store %StringBuffer %6, %StringBuffer* %result, align 8
  br label %switch.end

switch.case.3:                                    ; preds = %loop.body
  %result.load11 = load %StringBuffer, %StringBuffer* %result, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str3, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @159, i32 0, i32 0), i32 2)
  %__str.load12 = load %string, %string* %__str3, align 8
  %7 = call %StringBuffer @_EN3stdplE12StringBuffer6string(%StringBuffer %result.load11, %string %__str.load12)
  store %StringBuffer %7, %StringBuffer* %result, align 8
  br label %switch.end

switch.case.4:                                    ; preds = %loop.body
  %result.load13 = load %StringBuffer, %StringBuffer* %result, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str4, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @160, i32 0, i32 0), i32 2)
  %__str.load14 = load %string, %string* %__str4, align 8
  %8 = call %StringBuffer @_EN3stdplE12StringBuffer6string(%StringBuffer %result.load13, %string %__str.load14)
  store %StringBuffer %8, %StringBuffer* %result, align 8
  br label %switch.end

switch.case.5:                                    ; preds = %loop.body
  %result.load15 = load %StringBuffer, %StringBuffer* %result, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str5, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @161, i32 0, i32 0), i32 2)
  %__str.load16 = load %string, %string* %__str5, align 8
  %9 = call %StringBuffer @_EN3stdplE12StringBuffer6string(%StringBuffer %result.load15, %string %__str.load16)
  store %StringBuffer %9, %StringBuffer* %result, align 8
  br label %switch.end

switch.default:                                   ; preds = %loop.body
  %result.load17 = load %StringBuffer, %StringBuffer* %result, align 8
  %c.load18 = load i8, i8* %c, align 1
  %10 = call %StringBuffer @_EN3stdplE12StringBuffer4char(%StringBuffer %result.load17, i8 %c.load18)
  store %StringBuffer %10, %StringBuffer* %result, align 8
  br label %switch.end

switch.end:                                       ; preds = %switch.default, %switch.case.5, %switch.case.4, %switch.case.3, %switch.case.2, %switch.case.1, %switch.case.0
  br label %loop.increment
}

declare i1 @_EN3stdeqEP12StringBuffer6string(%StringBuffer*, %string)

define %StringBuffer @_EN3std6string6repeatE3int(%string* %this, i32 %count) {
  %result = alloca %StringBuffer, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %_ = alloca i32, align 4
  %2 = call i32 @_EN3std6string4sizeE(%string* %this)
  %3 = mul i32 %2, %count
  call void @_EN3std12StringBuffer4initE8capacity3int(%StringBuffer* %result, i32 %3)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 %count)
  %4 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %4, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %5 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %5, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %6 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %6, i32* %_, align 4
  %result.load = load %StringBuffer, %StringBuffer* %result, align 8
  %this.load = load %string, %string* %this, align 8
  %7 = call %StringBuffer @_EN3stdplE12StringBuffer6string(%StringBuffer %result.load, %string %this.load)
  store %StringBuffer %7, %StringBuffer* %result, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %result.load1 = load %StringBuffer, %StringBuffer* %result, align 8
  ret %StringBuffer %result.load1
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"*)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"*)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"*)

define i8* @_EN3std4ListI4charEixE3int(%"List<char>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<char>", %"List<char>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI4charE16indexOutOfBoundsE3int(%"List<char>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<char>", %"List<char>"* %this, i32 0, i32 0
  %buffer.load = load i8*, i8** %buffer, align 8
  %2 = getelementptr inbounds i8, i8* %buffer.load, i32 %index
  ret i8* %2
}

declare void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

define void @_EN3std12StringBuffer16indexOutOfBoundsE6string3int(%StringBuffer* %this, %string %function, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  %__str2 = alloca %string, align 8
  %3 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @162, i32 0, i32 0), i32 13)
  store %string %function, %string* %1, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @163, i32 0, i32 0), i32 8)
  store i32 %index, i32* %2, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @164, i32 0, i32 0), i32 27)
  %4 = call i32 @_EN3std12StringBuffer4sizeE(%StringBuffer* %this)
  store i32 %4, i32* %3, align 4
  %5 = call %never @_EN3std5abortI6string6string6string3int6string3intEEP6stringP6stringP6stringP3intP6stringP3int(%string* %__str, %string* %1, %string* %__str1, i32* %2, %string* %__str2, i32* %3)
  ret void
}

declare i8* @_EN3std4ListI4charE4dataE(%"List<char>"*)

define i32 @_EN3std5RangeI3intE4sizeE(%"Range<int>"* %this) {
  %end = getelementptr inbounds %"Range<int>", %"Range<int>"* %this, i32 0, i32 1
  %end.load = load i32, i32* %end, align 4
  %start = getelementptr inbounds %"Range<int>", %"Range<int>"* %this, i32 0, i32 0
  %start.load = load i32, i32* %start, align 4
  %1 = sub i32 %end.load, %start.load
  ret i32 %1
}

define %"ArrayIterator<string>" @_EN3std4ListI6stringE8iteratorE(%"List<string>"* %this) {
  %1 = alloca %"ArrayIterator<string>", align 8
  %2 = alloca %"ArrayRef<string>", align 8
  call void @_EN3std8ArrayRefI6stringE4initEP4ListI6stringE(%"ArrayRef<string>"* %2, %"List<string>"* %this)
  %.load = load %"ArrayRef<string>", %"ArrayRef<string>"* %2, align 8
  call void @_EN3std13ArrayIteratorI6stringE4initE8ArrayRefI6stringE(%"ArrayIterator<string>"* %1, %"ArrayRef<string>" %.load)
  %.load1 = load %"ArrayIterator<string>", %"ArrayIterator<string>"* %1, align 8
  ret %"ArrayIterator<string>" %.load1
}

define i1 @_EN3std13ArrayIteratorI6stringE8hasValueE(%"ArrayIterator<string>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<string>", %"ArrayIterator<string>"* %this, i32 0, i32 0
  %current.load = load %string*, %string** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<string>", %"ArrayIterator<string>"* %this, i32 0, i32 1
  %end.load = load %string*, %string** %end, align 8
  %1 = icmp ne %string* %current.load, %end.load
  ret i1 %1
}

define %string* @_EN3std13ArrayIteratorI6stringE5valueE(%"ArrayIterator<string>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<string>", %"ArrayIterator<string>"* %this, i32 0, i32 0
  %current.load = load %string*, %string** %current, align 8
  ret %string* %current.load
}

define void @_EN3std13ArrayIteratorI6stringE9incrementE(%"ArrayIterator<string>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<string>", %"ArrayIterator<string>"* %this, i32 0, i32 0
  %current.load = load %string*, %string** %current, align 8
  %1 = getelementptr inbounds %string, %string* %current.load, i32 1
  store %string* %1, %string** %current, align 8
  ret void
}

define void @_EN3std10deallocateIAU_6stringEEAU_6string(%string* %allocation) {
  %1 = bitcast %string* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define void @_EN3std4ListI6stringE16indexOutOfBoundsE3int(%"List<string>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @165, i32 0, i32 0), i32 11)
  store i32 %index, i32* %1, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @166, i32 0, i32 0), i32 27)
  %3 = call i32 @_EN3std4ListI6stringE4sizeE(%"List<string>"* %this)
  store i32 %3, i32* %2, align 4
  %4 = call %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %__str, i32* %1, %string* %__str1, i32* %2)
  ret void
}

define void @_EN3std4ListI6stringE4initE(%"List<string>"* %this) {
  %size = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

define void @_EN3std4ListI6stringE4pushE6string(%"List<string>"* %this, %string %element) {
  %size = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI6stringE4growE(%"List<string>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 0
  %buffer.load = load %string*, %string** %buffer, align 8
  %size1 = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 1
  %size.load2 = load i32, i32* %size1, align 4
  %2 = getelementptr inbounds %string, %string* %buffer.load, i32 %size.load2
  store %string %element, %string* %2, align 8
  %size3 = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 1
  %size.load4 = load i32, i32* %size3, align 4
  %3 = add i32 %size.load4, 1
  store i32 %3, i32* %size3, align 4
  ret void
}

define void @_EN3std12ByteIterator4initE6string(%ByteIterator* %this, %string %stream) {
  %stream1 = getelementptr inbounds %ByteIterator, %ByteIterator* %this, i32 0, i32 0
  store %string %stream, %string* %stream1, align 8
  ret void
}

define i1 @_EN3std6string5emptyE(%string* %this) {
  %characters = getelementptr inbounds %string, %string* %this, i32 0, i32 0
  %1 = call i32 @_EN3std8ArrayRefI4charE4sizeE(%"ArrayRef<char>"* %characters)
  %2 = icmp eq i32 %1, 0
  ret i1 %2
}

define i8 @_EN3std6string5frontE(%string* %this) {
  %characters = getelementptr inbounds %string, %string* %this, i32 0, i32 0
  %1 = call i8* @_EN3std8ArrayRefI4charE5frontE(%"ArrayRef<char>"* %characters)
  %.load = load i8, i8* %1, align 1
  ret i8 %.load
}

define %string @_EN3std6string6substrE3int(%string* %this, i32 %start) {
  %__str = alloca %string, align 8
  %1 = alloca %string, align 8
  %2 = icmp slt i32 %start, 0
  br i1 %2, label %or.end, label %or.rhs

or.rhs:                                           ; preds = %0
  %3 = call i32 @_EN3std6string4sizeE(%string* %this)
  %4 = icmp sgt i32 %start, %3
  br label %or.end

or.end:                                           ; preds = %or.rhs, %0
  %or = phi i1 [ %2, %0 ], [ %4, %or.rhs ]
  br i1 %or, label %if.then, label %if.else

if.then:                                          ; preds = %or.end
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @167, i32 0, i32 0), i32 6)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std6string16indexOutOfBoundsE6string3int(%string* %this, %string %__str.load, i32 %start)
  br label %if.end

if.else:                                          ; preds = %or.end
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %characters = getelementptr inbounds %string, %string* %this, i32 0, i32 0
  %5 = call i8* @_EN3std8ArrayRefI4charE4dataE(%"ArrayRef<char>"* %characters)
  %6 = getelementptr inbounds i8, i8* %5, i32 %start
  %7 = call i32 @_EN3std6string4sizeE(%string* %this)
  %8 = sub i32 %7, %start
  call void @_EN3std6string4initEP4char3int(%string* %1, i8* %6, i32 %8)
  %.load = load %string, %string* %1, align 8
  ret %string %.load
}

define void @_EN3std12LineIterator4initE6string(%LineIterator* %this, %string %stream) {
  %1 = alloca %string, align 8
  %stream1 = getelementptr inbounds %LineIterator, %LineIterator* %this, i32 0, i32 0
  store %string %stream, %string* %stream1, align 8
  %end = getelementptr inbounds %LineIterator, %LineIterator* %this, i32 0, i32 1
  store %string %stream, %string* %1, align 8
  %2 = call i32 @_EN3std6string4findE4char(%string* %1, i8 10)
  store i32 %2, i32* %end, align 4
  ret void
}

define %string @_EN3std6string6substrE5RangeI3intE(%string* %this, %"Range<int>" %range) {
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %1 = alloca %string, align 8
  %2 = alloca %"Range<int>", align 8
  %start = extractvalue %"Range<int>" %range, 0
  %3 = icmp slt i32 %start, 0
  br i1 %3, label %or.end, label %or.rhs

or.rhs:                                           ; preds = %0
  %start2 = extractvalue %"Range<int>" %range, 0
  %4 = call i32 @_EN3std6string4sizeE(%string* %this)
  %5 = icmp sgt i32 %start2, %4
  br label %or.end

or.end:                                           ; preds = %or.rhs, %0
  %or = phi i1 [ %3, %0 ], [ %5, %or.rhs ]
  br i1 %or, label %if.then, label %if.else

if.then:                                          ; preds = %or.end
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @168, i32 0, i32 0), i32 6)
  %__str.load = load %string, %string* %__str, align 8
  %start3 = extractvalue %"Range<int>" %range, 0
  call void @_EN3std6string16indexOutOfBoundsE6string3int(%string* %this, %string %__str.load, i32 %start3)
  br label %if.end

if.else:                                          ; preds = %or.end
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %end = extractvalue %"Range<int>" %range, 1
  %6 = icmp slt i32 %end, 0
  br i1 %6, label %or.end6, label %or.rhs4

or.rhs4:                                          ; preds = %if.end
  %end5 = extractvalue %"Range<int>" %range, 1
  %7 = call i32 @_EN3std6string4sizeE(%string* %this)
  %8 = icmp sgt i32 %end5, %7
  br label %or.end6

or.end6:                                          ; preds = %or.rhs4, %if.end
  %or7 = phi i1 [ %6, %if.end ], [ %8, %or.rhs4 ]
  br i1 %or7, label %if.then8, label %if.else11

if.then8:                                         ; preds = %or.end6
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @169, i32 0, i32 0), i32 6)
  %__str.load9 = load %string, %string* %__str1, align 8
  %end10 = extractvalue %"Range<int>" %range, 1
  call void @_EN3std6string16indexOutOfBoundsE6string3int(%string* %this, %string %__str.load9, i32 %end10)
  br label %if.end12

if.else11:                                        ; preds = %or.end6
  br label %if.end12

if.end12:                                         ; preds = %if.else11, %if.then8
  %characters = getelementptr inbounds %string, %string* %this, i32 0, i32 0
  %9 = call i8* @_EN3std8ArrayRefI4charE4dataE(%"ArrayRef<char>"* %characters)
  %start13 = extractvalue %"Range<int>" %range, 0
  %10 = getelementptr inbounds i8, i8* %9, i32 %start13
  store %"Range<int>" %range, %"Range<int>"* %2, align 4
  %11 = call i32 @_EN3std5RangeI3intE4sizeE(%"Range<int>"* %2)
  call void @_EN3std6string4initEP4char3int(%string* %1, i8* %10, i32 %11)
  %.load = load %string, %string* %1, align 8
  ret %string %.load
}

define i32 @_EN3std6string4findE4char(%string* %this, i8 %c) {
  %1 = call i32 @_EN3std6string4findE4char3int(%string* %this, i8 %c, i32 0)
  ret i32 %1
}

define void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %this, i32 %value) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 0
  store i1 true, i1* %hasValue, align 1
  %value1 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 1
  store i32 %value, i32* %value1, align 4
  ret void
}

declare void @_EN3std12StringBuffer4initE(%StringBuffer*)

declare %StringBuffer @_EN3stdplE12StringBuffer6string(%StringBuffer, %string)

declare %StringBuffer @_EN3stdplE12StringBuffer4char(%StringBuffer, i8)

define void @_EN3std12StringBuffer4initE8capacity3int(%StringBuffer* %this, i32 %capacity) {
  call void @_EN3std12StringBuffer4initE(%StringBuffer* %this)
  %characters = getelementptr inbounds %StringBuffer, %StringBuffer* %this, i32 0, i32 0
  call void @_EN3std4ListI4charE7reserveE3int(%"List<char>"* %characters, i32 %capacity)
  ret void
}

define void @_EN3std4ListI4charE16indexOutOfBoundsE3int(%"List<char>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca i32, align 4
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @170, i32 0, i32 0), i32 11)
  store i32 %index, i32* %1, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @171, i32 0, i32 0), i32 27)
  %3 = call i32 @_EN3std4ListI4charE4sizeE(%"List<char>"* %this)
  store i32 %3, i32* %2, align 4
  %4 = call %never @_EN3std5abortI6string3int6string3intEEP6stringP3intP6stringP3int(%string* %__str, i32* %1, %string* %__str1, i32* %2)
  ret void
}

declare %never @_EN3std5abortI6string6string6string3int6string3intEEP6stringP6stringP6stringP3intP6stringP3int(%string*, %string*, %string*, i32*, %string*, i32*)

define void @_EN3std8ArrayRefI6stringE4initEP4ListI6stringE(%"ArrayRef<string>"* %this, %"List<string>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<string>", %"ArrayRef<string>"* %this, i32 0, i32 0
  %1 = call %string* @_EN3std4ListI6stringE4dataE(%"List<string>"* %list)
  store %string* %1, %string** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<string>", %"ArrayRef<string>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI6stringE4sizeE(%"List<string>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI6stringE4initE8ArrayRefI6stringE(%"ArrayIterator<string>"* %this, %"ArrayRef<string>" %array) {
  %1 = alloca %"ArrayRef<string>", align 8
  %2 = alloca %"ArrayRef<string>", align 8
  %3 = alloca %"ArrayRef<string>", align 8
  %current = getelementptr inbounds %"ArrayIterator<string>", %"ArrayIterator<string>"* %this, i32 0, i32 0
  store %"ArrayRef<string>" %array, %"ArrayRef<string>"* %1, align 8
  %4 = call %string* @_EN3std8ArrayRefI6stringE4dataE(%"ArrayRef<string>"* %1)
  store %string* %4, %string** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<string>", %"ArrayIterator<string>"* %this, i32 0, i32 1
  store %"ArrayRef<string>" %array, %"ArrayRef<string>"* %2, align 8
  %5 = call %string* @_EN3std8ArrayRefI6stringE4dataE(%"ArrayRef<string>"* %2)
  store %"ArrayRef<string>" %array, %"ArrayRef<string>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI6stringE4sizeE(%"ArrayRef<string>"* %3)
  %7 = getelementptr inbounds %string, %string* %5, i32 %6
  store %string* %7, %string** %end, align 8
  ret void
}

declare void @free(i8*)

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

define void @_EN3std4ListI6stringE4growE(%"List<string>"* %this) {
  %capacity = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI6stringE7reserveE3int(%"List<string>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 2
  %capacity.load2 = load i32, i32* %capacity1, align 4
  %2 = mul i32 %capacity.load2, 2
  call void @_EN3std4ListI6stringE7reserveE3int(%"List<string>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare i32 @_EN3std8ArrayRefI4charE4sizeE(%"ArrayRef<char>"*)

define i8* @_EN3std8ArrayRefI4charE5frontE(%"ArrayRef<char>"* %this) {
  %__str = alloca %string, align 8
  %1 = call i1 @_EN3std8ArrayRefI4charE5emptyE(%"ArrayRef<char>"* %this)
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @172, i32 0, i32 0), i32 5)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std8ArrayRefI4charE16indexOutOfBoundsE6string3int(%"ArrayRef<char>"* %this, %string %__str.load, i32 0)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %data = getelementptr inbounds %"ArrayRef<char>", %"ArrayRef<char>"* %this, i32 0, i32 0
  %data.load = load i8*, i8** %data, align 8
  %2 = getelementptr inbounds i8, i8* %data.load, i32 0
  ret i8* %2
}

define void @_EN3std6string16indexOutOfBoundsE6string3int(%string* %this, %string %function, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  %__str2 = alloca %string, align 8
  %3 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @173, i32 0, i32 0), i32 7)
  store %string %function, %string* %1, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @174, i32 0, i32 0), i32 8)
  store i32 %index, i32* %2, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @175, i32 0, i32 0), i32 27)
  %4 = call i32 @_EN3std6string4sizeE(%string* %this)
  store i32 %4, i32* %3, align 4
  %5 = call %never @_EN3std5abortI6string6string6string3int6string3intEEP6stringP6stringP6stringP3intP6stringP3int(%string* %__str, %string* %1, %string* %__str1, i32* %2, %string* %__str2, i32* %3)
  ret void
}

declare i8* @_EN3std8ArrayRefI4charE4dataE(%"ArrayRef<char>"*)

define i32 @_EN3std6string4findE4char3int(%string* %this, i8 %c, i32 %start) {
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  %2 = call i32 @_EN3std6string4sizeE(%string* %this)
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 %start, i32 %2)
  %3 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %3, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %4 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %5 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %5, i32* %i, align 4
  %characters = getelementptr inbounds %string, %string* %this, i32 0, i32 0
  %i.load = load i32, i32* %i, align 4
  %6 = call i8* @_EN3std8ArrayRefI4charEixE3int(%"ArrayRef<char>"* %characters, i32 %i.load)
  %.load = load i8, i8* %6, align 1
  %7 = icmp eq i8 %.load, %c
  br i1 %7, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %8 = call i32 @_EN3std6string4sizeE(%string* %this)
  ret i32 %8

if.then:                                          ; preds = %loop.body
  %i.load1 = load i32, i32* %i, align 4
  ret i32 %i.load1

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.increment
}

declare void @_EN3std4ListI4charE7reserveE3int(%"List<char>"*, i32)

declare i32 @_EN3std4ListI4charE4sizeE(%"List<char>"*)

define %string* @_EN3std8ArrayRefI6stringE4dataE(%"ArrayRef<string>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<string>", %"ArrayRef<string>"* %this, i32 0, i32 0
  %data.load = load %string*, %string** %data, align 8
  ret %string* %data.load
}

define i32 @_EN3std8ArrayRefI6stringE4sizeE(%"ArrayRef<string>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<string>", %"ArrayRef<string>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define %string* @_EN3std4ListI6stringE4dataE(%"List<string>"* %this) {
  %buffer = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 0
  %buffer.load = load %string*, %string** %buffer, align 8
  ret %string* %buffer.load
}

declare %OutputFileStream @_EN3std6stderrE()

declare void @_EN3std16OutputFileStream5writeI6stringEEP6string(%OutputFileStream*, %string*)

declare void @_EN3std16OutputFileStream5writeI3intEEP3int(%OutputFileStream*, i32*)

declare void @_EN3std16OutputFileStream5writeI4charEEP4char(%OutputFileStream*, i8*)

declare %never @_EN3std12abortWrapperE()

define void @_EN3std4ListI6stringE7reserveE3int(%"List<string>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca %string*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca %string*, align 8
  %target = alloca %string*, align 8
  %capacity = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call %string* @_EN3std13allocateArrayI6stringEE3int(i32 %minimumCapacity)
  store %string* %3, %string** %newBuffer, align 8
  %size = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 1
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
  %buffer = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 0
  %buffer.load = load %string*, %string** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds %string, %string* %buffer.load, i32 %index.load
  store %string* %7, %string** %source, align 8
  %newBuffer.load = load %string*, %string** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds %string, %string* %newBuffer.load, i32 %index.load1
  store %string* %8, %string** %target, align 8
  %target.load = load %string*, %string** %target, align 8
  %source.load = load %string*, %string** %source, align 8
  %source.load.load = load %string, %string* %source.load, align 8
  store %string %source.load.load, %string* %target.load, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 0
  %buffer.load6 = load %string*, %string** %buffer5, align 8
  call void @_EN3std10deallocateIAU_6stringEEAU_6string(%string* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 0
  %newBuffer.load10 = load %string*, %string** %newBuffer, align 8
  store %string* %newBuffer.load10, %string** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<string>", %"List<string>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity11, align 4
  br label %if.end
}

define i1 @_EN3std8ArrayRefI4charE5emptyE(%"ArrayRef<char>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<char>", %"ArrayRef<char>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  %1 = icmp eq i32 %size.load, 0
  ret i1 %1
}

declare void @_EN3std8ArrayRefI4charE16indexOutOfBoundsE6string3int(%"ArrayRef<char>"*, %string, i32)

declare i8* @_EN3std8ArrayRefI4charEixE3int(%"ArrayRef<char>"*, i32)

define %string* @_EN3std13allocateArrayI6stringEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (%"ArrayRef<char>"* getelementptr (%"ArrayRef<char>", %"ArrayRef<char>"* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @176, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to %string*
  ret %string* %4
}

declare i8* @malloc(i64)
