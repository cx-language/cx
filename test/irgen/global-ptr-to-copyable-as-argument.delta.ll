
%S = type { i32 }

@sp = private global %S* null

define i32 @main() {
  %sp = load %S*, %S** @sp
  call void @_EN4main3fooE1pOP1S(%S* %sp)
  ret i32 0
}

define void @_EN4main3fooE1pOP1S(%S* %p) {
  ret void
}
