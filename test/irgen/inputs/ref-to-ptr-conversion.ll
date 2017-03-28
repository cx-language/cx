define void @foo([2 x i32]* %ints) {
  %ptr = alloca [2 x i32]*
  store [2 x i32]* %ints, [2 x i32]** %ptr
  ret void
}
