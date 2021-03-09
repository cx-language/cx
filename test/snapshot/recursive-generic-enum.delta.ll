
%E = type { i32, { [48 x i8] } }
%"S<E>" = type { %E* }

define i32 @main() {
  %e = alloca %E, align 8
  %enum = alloca %E, align 8
  %1 = alloca %"S<E>", align 8
  %enum1 = alloca %E, align 8
  %2 = alloca %"S<E>", align 8
  %tag = getelementptr inbounds %E, %E* %enum, i32 0, i32 0
  store i32 0, i32* %tag, align 4
  %tag2 = getelementptr inbounds %E, %E* %enum1, i32 0, i32 0
  store i32 0, i32* %tag2, align 4
  call void @_EN4main1SI1EE4initEOP1E(%"S<E>"* %2, %E* null)
  %.load = load %"S<E>", %"S<E>"* %2, align 8
  %3 = insertvalue { %"S<E>" } undef, %"S<E>" %.load, 0
  %associatedValue = getelementptr inbounds %E, %E* %enum1, i32 0, i32 1
  %4 = bitcast { [48 x i8] }* %associatedValue to { %"S<E>" }*
  store { %"S<E>" } %3, { %"S<E>" }* %4, align 8
  call void @_EN4main1SI1EE4initEOP1E(%"S<E>"* %1, %E* %enum1)
  %.load3 = load %"S<E>", %"S<E>"* %1, align 8
  %5 = insertvalue { %"S<E>" } undef, %"S<E>" %.load3, 0
  %associatedValue4 = getelementptr inbounds %E, %E* %enum, i32 0, i32 1
  %6 = bitcast { [48 x i8] }* %associatedValue4 to { %"S<E>" }*
  store { %"S<E>" } %5, { %"S<E>" }* %6, align 8
  %enum.load = load %E, %E* %enum, align 4
  store %E %enum.load, %E* %e, align 4
  ret i32 0
}

define void @_EN4main1SI1EE4initEOP1E(%"S<E>"* %this, %E* %e) {
  %e1 = getelementptr inbounds %"S<E>", %"S<E>"* %this, i32 0, i32 0
  store %E* %e, %E** %e1, align 8
  ret void
}
