
%E = type { i32, [8 x i8] }

define i32 @main() {
  %e = alloca %E
  %enum = alloca %E
  %enum1 = alloca %E
  %enum3 = alloca %E
  %enum6 = alloca %E
  %tag = getelementptr inbounds %E, %E* %enum, i32 0, i32 0
  store i32 0, i32* %tag
  %enum.load = load %E, %E* %enum
  store %E %enum.load, %E* %e
  %tag2 = getelementptr inbounds %E, %E* %enum1, i32 0, i32 0
  store i32 1, i32* %tag2
  %associatedValue = getelementptr inbounds %E, %E* %enum1, i32 0, i32 1
  %1 = bitcast [8 x i8]* %associatedValue to { i1, i32 }*
  store { i1, i32 } { i1 false, i32 42 }, { i1, i32 }* %1
  %enum1.load = load %E, %E* %enum1
  store %E %enum1.load, %E* %e
  %tag4 = getelementptr inbounds %E, %E* %enum3, i32 0, i32 0
  store i32 2, i32* %tag4
  %associatedValue5 = getelementptr inbounds %E, %E* %enum3, i32 0, i32 1
  %2 = bitcast [8 x i8]* %associatedValue5 to { i32 }*
  store { i32 } { i32 43 }, { i32 }* %2
  %enum3.load = load %E, %E* %enum3
  store %E %enum3.load, %E* %e
  %e.tag = getelementptr inbounds %E, %E* %e, i32 0, i32 0
  %e.tag.load = load i32, i32* %e.tag
  switch i32 %e.tag.load, label %switch.default [
    i32 0, label %switch.case.0
    i32 1, label %switch.case.1
    i32 2, label %switch.case.2
  ]

switch.case.0:                                    ; preds = %0
  br label %switch.end

switch.case.1:                                    ; preds = %0
  %3 = getelementptr inbounds %E, %E* %e, i32 0, i32 1
  %eb = bitcast [8 x i8]* %3 to { i1, i32 }*
  %tag7 = getelementptr inbounds %E, %E* %enum6, i32 0, i32 0
  store i32 2, i32* %tag7
  %i = getelementptr inbounds { i1, i32 }, { i1, i32 }* %eb, i32 0, i32 1
  %i.load = load i32, i32* %i
  %4 = insertvalue { i32 } undef, i32 %i.load, 0
  %associatedValue8 = getelementptr inbounds %E, %E* %enum6, i32 0, i32 1
  %5 = bitcast [8 x i8]* %associatedValue8 to { i32 }*
  store { i32 } %4, { i32 }* %5
  %enum6.load = load %E, %E* %enum6
  store %E %enum6.load, %E* %e
  %i9 = getelementptr inbounds { i1, i32 }, { i1, i32 }* %eb, i32 0, i32 1
  %i9.load = load i32, i32* %i9
  ret i32 %i9.load

switch.case.2:                                    ; preds = %0
  %6 = getelementptr inbounds %E, %E* %e, i32 0, i32 1
  %eb10 = bitcast [8 x i8]* %6 to { i32 }*
  %j = getelementptr inbounds { i32 }, { i32 }* %eb10, i32 0, i32 0
  %j.load = load i32, i32* %j
  ret i32 %j.load

switch.default:                                   ; preds = %0
  br label %switch.end

switch.end:                                       ; preds = %switch.default, %switch.case.0
  %e.tag11 = getelementptr inbounds %E, %E* %e, i32 0, i32 0
  %e.tag11.load = load i32, i32* %e.tag11
  %7 = icmp eq i32 %e.tag11.load, 0
  %e.tag12 = getelementptr inbounds %E, %E* %e, i32 0, i32 0
  %e.tag12.load = load i32, i32* %e.tag12
  %8 = icmp eq i32 %e.tag12.load, 1
  ret i32 0
}
