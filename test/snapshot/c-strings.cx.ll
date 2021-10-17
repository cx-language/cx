
%S = type { [1024 x i8] }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }
%StringBuffer = type { %"List<char>" }
%"List<char>" = type { i8*, i32, i32 }

@0 = private unnamed_addr constant [2 x i8] c"x\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"x\00", align 1

define i32 @main() {
  %s = alloca %S, align 8
  %__str = alloca %string, align 8
  %1 = alloca %StringBuffer, align 8
  %__str1 = alloca %string, align 8
  %a = getelementptr inbounds %S, %S* %s, i32 0, i32 0
  %2 = bitcast [1024 x i8]* %a to i8*
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @0, i32 0, i32 0), i32 1)
  %__str.load = load %string, %string* %__str, align 8
  %3 = call i1 @_EN3stdeqEP4char6string(i8* %2, %string %__str.load)
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0), i32 1)
  %__str.load2 = load %string, %string* %__str1, align 8
  call void @_EN3std12StringBuffer4initE6string(%StringBuffer* %1, %string %__str.load2)
  %a3 = getelementptr inbounds %S, %S* %s, i32 0, i32 0
  %4 = bitcast [1024 x i8]* %a3 to i8*
  %5 = call i1 @_EN3stdneER12StringBufferAU_4char(%StringBuffer* %1, i8* %4)
  ret i32 0
}

declare void @_EN3std6string4initEP4char3int(%string*, i8*, i32)

declare i1 @_EN3stdeqEP4char6string(i8*, %string)

declare void @_EN3std12StringBuffer4initE6string(%StringBuffer*, %string)

declare i1 @_EN3stdneER12StringBufferAU_4char(%StringBuffer*, i8*)
