
%0 = type <{ { i32 } }>
%"Optional<int>" = type { i32, %0 }

define %"Optional<int>" @_EN4main1fE() {
  %i = alloca i32, align 4
  %enum = alloca %"Optional<int>", align 8
  store i32 1, ptr %i, align 4
  %tag = getelementptr inbounds %"Optional<int>", ptr %enum, i32 0, i32 0
  store i32 1, ptr %tag, align 4
  %i.load = load i32, ptr %i, align 4
  %1 = insertvalue { i32 } undef, i32 %i.load, 0
  %associatedValue = getelementptr inbounds %"Optional<int>", ptr %enum, i32 0, i32 1
  store { i32 } %1, ptr %associatedValue, align 4
  %enum.load = load %"Optional<int>", ptr %enum, align 4
  ret %"Optional<int>" %enum.load
}
