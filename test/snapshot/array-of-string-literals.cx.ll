
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { ptr, i32 }

@0 = private unnamed_addr constant [4 x i8] c"foo\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"bar\00", align 1

define i32 @main() {
  %a = alloca [2 x %string], align 8
  %__str = alloca %string, align 8
  %__str1 = alloca %string, align 8
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @0, i32 3)
  %__str.load = load %string, ptr %__str, align 8
  %insert.alloca = alloca [2 x %string], align 8
  %insert.gep = getelementptr inbounds [2 x %string], ptr %insert.alloca, i32 0, i32 0
  store %string %__str.load, ptr %insert.gep, align 8
  call void @_EN3std6string4initEP4char3int(ptr %__str1, ptr @1, i32 3)
  %__str.load2 = load %string, ptr %__str1, align 8
  %insert.alloca3 = alloca [2 x %string], align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %insert.alloca3, ptr align 8 %insert.alloca, i64 32, i1 false)
  %insert.gep4 = getelementptr inbounds [2 x %string], ptr %insert.alloca3, i32 0, i32 1
  store %string %__str.load2, ptr %insert.gep4, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %a, ptr align 8 %insert.alloca3, i64 32, i1 false)
  ret i32 0
}

declare void @_EN3std6string4initEP4char3int(ptr, ptr, i32)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
