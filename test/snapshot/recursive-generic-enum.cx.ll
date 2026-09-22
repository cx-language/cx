
%0 = type { { [12 x i32] } }
%E = type { i32, %0 }
%"S<E>" = type { ptr }

define i32 @main() {
  %e = alloca %E, align 8
  %enum = alloca %E, align 8
  %1 = alloca %"S<E>", align 8
  %enum1 = alloca %E, align 8
  %2 = alloca %"S<E>", align 8
  %tag = getelementptr inbounds %E, ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %tag2 = getelementptr inbounds %E, ptr %enum1, i32 0, i32 0
  store i32 0, ptr %tag2, align 4
  call void @_EN4main1SI1EE4initEOP1E(ptr %2, ptr null)
  %.load = load %"S<E>", ptr %2, align 8
  %3 = insertvalue { %"S<E>" } undef, %"S<E>" %.load, 0
  %associatedValue = getelementptr inbounds %E, ptr %enum1, i32 0, i32 1
  store { %"S<E>" } %3, ptr %associatedValue, align 8
  call void @_EN4main1SI1EE4initEOP1E(ptr %1, ptr %enum1)
  %.load3 = load %"S<E>", ptr %1, align 8
  %4 = insertvalue { %"S<E>" } undef, %"S<E>" %.load3, 0
  %associatedValue4 = getelementptr inbounds %E, ptr %enum, i32 0, i32 1
  store { %"S<E>" } %4, ptr %associatedValue4, align 8
  %enum.load = alloca %E, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %enum.load, ptr align 4 %enum, i64 52, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %e, ptr align 4 %enum.load, i64 52, i1 false)
  ret i32 0
}

define void @_EN4main1SI1EE4initEOP1E(ptr %this, ptr %e) {
  %e1 = alloca ptr, align 8
  store ptr %e, ptr %e1, align 8
  %e2 = getelementptr inbounds %"S<E>", ptr %this, i32 0, i32 0
  %e.load = load ptr, ptr %e1, align 8
  store ptr %e.load, ptr %e2, align 8
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
