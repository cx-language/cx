
define i32 @main() {
  %b = alloca i1, align 1
  %c = alloca i1, align 1
  store i1 false, i1* %b, align 1
  %b.load = load i1, i1* %b, align 1
  %1 = icmp eq i1 %b.load, true
  store i1 %1, i1* %c, align 1
  ret i32 0
}
