
%S = type {}
%C = type {}

define i32 @main() {
  %s = alloca %S, align 8
  %c = alloca %C, align 8
  ret i32 0
}
