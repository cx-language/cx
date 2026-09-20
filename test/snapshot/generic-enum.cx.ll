
%0 = type <{ { i32 } }>
%"Opt<int>" = type { i32, %0 }

define i32 @main() {
  %a = alloca %"Opt<int>", align 8
  %enum = alloca %"Opt<int>", align 8
  %b = alloca %"Opt<int>", align 8
  %enum1 = alloca %"Opt<int>", align 8
  %enum2 = alloca %"Opt<int>", align 8
  %enum3 = alloca %"Opt<int>", align 8
  %tag = getelementptr inbounds %"Opt<int>", ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %associatedValue = getelementptr inbounds %"Opt<int>", ptr %enum, i32 0, i32 1
  store { i32 } { i32 1 }, ptr %associatedValue, align 4
  %enum.load = load %"Opt<int>", ptr %enum, align 4
  store %"Opt<int>" %enum.load, ptr %a, align 4
  %tag4 = getelementptr inbounds %"Opt<int>", ptr %enum1, i32 0, i32 0
  store i32 1, ptr %tag4, align 4
  %enum.load5 = load %"Opt<int>", ptr %enum1, align 4
  store %"Opt<int>" %enum.load5, ptr %b, align 4
  %a.tag = getelementptr inbounds %"Opt<int>", ptr %a, i32 0, i32 0
  %a.tag.load = load i32, ptr %a.tag, align 4
  switch i32 %a.tag.load, label %switch.default [
    i32 0, label %switch.case.0
    i32 1, label %switch.case.1
  ]

switch.case.0:                                    ; preds = %0
  %1 = getelementptr inbounds %"Opt<int>", ptr %a, i32 0, i32 1
  %tag6 = getelementptr inbounds %"Opt<int>", ptr %enum2, i32 0, i32 0
  store i32 0, ptr %tag6, align 4
  %value = getelementptr inbounds { i32 }, ptr %1, i32 0, i32 0
  %value.load = load i32, ptr %value, align 4
  %2 = add i32 %value.load, 1
  %3 = insertvalue { i32 } undef, i32 %2, 0
  %associatedValue7 = getelementptr inbounds %"Opt<int>", ptr %enum2, i32 0, i32 1
  store { i32 } %3, ptr %associatedValue7, align 4
  %enum.load8 = load %"Opt<int>", ptr %enum2, align 4
  store %"Opt<int>" %enum.load8, ptr %b, align 4
  br label %switch.end

switch.case.1:                                    ; preds = %0
  %tag9 = getelementptr inbounds %"Opt<int>", ptr %enum3, i32 0, i32 0
  store i32 1, ptr %tag9, align 4
  %enum.load10 = load %"Opt<int>", ptr %enum3, align 4
  store %"Opt<int>" %enum.load10, ptr %b, align 4
  br label %switch.end

switch.default:                                   ; preds = %0
  br label %switch.end

switch.end:                                       ; preds = %switch.default, %switch.case.1, %switch.case.0
  %b.tag = getelementptr inbounds %"Opt<int>", ptr %b, i32 0, i32 0
  %b.tag.load = load i32, ptr %b.tag, align 4
  switch i32 %b.tag.load, label %switch.default15 [
    i32 0, label %switch.case.011
    i32 1, label %switch.case.114
  ]

switch.case.011:                                  ; preds = %switch.end
  %4 = getelementptr inbounds %"Opt<int>", ptr %b, i32 0, i32 1
  %value12 = getelementptr inbounds { i32 }, ptr %4, i32 0, i32 0
  %value.load13 = load i32, ptr %value12, align 4
  ret i32 %value.load13

switch.case.114:                                  ; preds = %switch.end
  ret i32 0

switch.default15:                                 ; preds = %switch.end
  br label %switch.end16

switch.end16:                                     ; preds = %switch.default15
  ret i32 0
}
