
%"Optional<ArrayRef<int8>>" = type { i32, <{ { %"ArrayRef<int8>" } }> }
%"ArrayRef<int8>" = type { ptr, i32 }

define i32 @main() {
  %a = alloca %"Optional<ArrayRef<int8>>", align 8
  %enum = alloca %"Optional<ArrayRef<int8>>", align 8
  %tag = getelementptr inbounds %"Optional<ArrayRef<int8>>", ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %enum.load = load %"Optional<ArrayRef<int8>>", ptr %enum, align 4
  store %"Optional<ArrayRef<int8>>" %enum.load, ptr %a, align 4
  ret i32 0
}
