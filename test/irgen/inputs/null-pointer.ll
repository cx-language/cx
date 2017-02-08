define i32* @foo() {
  ret i32* null
}

define i32 @main() {
  %isNull = alloca i1
  %1 = call i32* @foo()
  %2 = icmp eq i32* %1, null
  store i1 %2, i1* %isNull
  %isNonNull = alloca i1
  %3 = call i32* @foo()
  %4 = icmp ne i32* %3, null
  store i1 %4, i1* %isNonNull
  %ptr = alloca i32*
  %5 = call i32* @foo()
  store i32* %5, i32** %ptr
  %ptr2 = alloca i1*
  store i1* null, i1** %ptr2
  ret i32 0
}
