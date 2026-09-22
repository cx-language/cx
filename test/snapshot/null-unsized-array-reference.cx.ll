
%0 = type { { %"Slice<int8>" } }
%"Optional<Slice<int8>>" = type { i32, %0 }
%"Slice<int8>" = type { ptr, i32 }

define i32 @main() {
  %a = alloca %"Optional<Slice<int8>>", align 8
  %enum = alloca %"Optional<Slice<int8>>", align 8
  %tag = getelementptr inbounds %"Optional<Slice<int8>>", ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %enum.load = alloca %"Optional<Slice<int8>>", align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %enum.load, ptr align 8 %enum, i64 24, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %a, ptr align 8 %enum.load, i64 24, i1 false)
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
