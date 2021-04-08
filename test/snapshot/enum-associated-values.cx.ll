
%E = type { i32, { [8 x i8] } }

define i32 @main() {
  %e = alloca %E, align 8
  %enum = alloca %E, align 8
  %enum1 = alloca %E, align 8
  %enum2 = alloca %E, align 8
  %enum3 = alloca %E, align 8
  %tag = getelementptr inbounds %E, %E* %enum, i32 0, i32 0
  store i32 0, i32* %tag, align 4
  %enum.load = load %E, %E* %enum, align 4
  store %E %enum.load, %E* %e, align 4
  %tag4 = getelementptr inbounds %E, %E* %enum1, i32 0, i32 0
  store i32 1, i32* %tag4, align 4
  %associatedValue = getelementptr inbounds %E, %E* %enum1, i32 0, i32 1
  %1 = bitcast { [8 x i8] }* %associatedValue to { i1, i32 }*
  store { i1, i32 } { i1 false, i32 42 }, { i1, i32 }* %1, align 4
  %enum.load5 = load %E, %E* %enum1, align 4
  store %E %enum.load5, %E* %e, align 4
  %tag6 = getelementptr inbounds %E, %E* %enum2, i32 0, i32 0
  store i32 2, i32* %tag6, align 4
  %associatedValue7 = getelementptr inbounds %E, %E* %enum2, i32 0, i32 1
  %2 = bitcast { [8 x i8] }* %associatedValue7 to { i32 }*
  store { i32 } { i32 43 }, { i32 }* %2, align 4
  %enum.load8 = load %E, %E* %enum2, align 4
  store %E %enum.load8, %E* %e, align 4
  %e.tag = getelementptr inbounds %E, %E* %e, i32 0, i32 0
  %e.tag.load = load i32, i32* %e.tag, align 4
  switch i32 %e.tag.load, label %switch.default [
    i32 0, label %switch.case.0
    i32 1, label %switch.case.1
    i32 2, label %switch.case.2
  ]

switch.case.0:                                    ; preds = %0
  br label %switch.end

switch.case.1:                                    ; preds = %0
  %3 = getelementptr inbounds %E, %E* %e, i32 0, i32 1
  %eb = bitcast { [8 x i8] }* %3 to { i1, i32 }*
  %tag9 = getelementptr inbounds %E, %E* %enum3, i32 0, i32 0
  store i32 2, i32* %tag9, align 4
  %i = getelementptr inbounds { i1, i32 }, { i1, i32 }* %eb, i32 0, i32 1
  %i.load = load i32, i32* %i, align 4
  %4 = insertvalue { i32 } undef, i32 %i.load, 0
  %associatedValue10 = getelementptr inbounds %E, %E* %enum3, i32 0, i32 1
  %5 = bitcast { [8 x i8] }* %associatedValue10 to { i32 }*
  store { i32 } %4, { i32 }* %5, align 4
  %enum.load11 = load %E, %E* %enum3, align 4
  store %E %enum.load11, %E* %e, align 4
  %i12 = getelementptr inbounds { i1, i32 }, { i1, i32 }* %eb, i32 0, i32 1
  %i.load13 = load i32, i32* %i12, align 4
  ret i32 %i.load13

switch.case.2:                                    ; preds = %0
  %6 = getelementptr inbounds %E, %E* %e, i32 0, i32 1
  %eb14 = bitcast { [8 x i8] }* %6 to { i32 }*
  %j = getelementptr inbounds { i32 }, { i32 }* %eb14, i32 0, i32 0
  %j.load = load i32, i32* %j, align 4
  ret i32 %j.load

switch.default:                                   ; preds = %0
  br label %switch.end

switch.end:                                       ; preds = %switch.default, %switch.case.0
  %e.tag15 = getelementptr inbounds %E, %E* %e, i32 0, i32 0
  %e.tag.load16 = load i32, i32* %e.tag15, align 4
  %7 = icmp eq i32 %e.tag.load16, 0
  %e.tag17 = getelementptr inbounds %E, %E* %e, i32 0, i32 0
  %e.tag.load18 = load i32, i32* %e.tag17, align 4
  %8 = icmp eq i32 %e.tag.load18, 1
  ret i32 0
}
