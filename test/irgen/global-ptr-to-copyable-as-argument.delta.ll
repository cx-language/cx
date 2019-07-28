
%S = type { i32 }

@sp = private global %S* null

define i32 @main() {
  %sp.load = load %S*, %S** @sp
  call void @_EN4main3fooE1pOP1S(%S* %sp.load)
  ret i32 0
}

define void @_EN4main1S4initE1i3int(%S* %this, i32 %i) {
  %i1 = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store i32 %i, i32* %i1
  ret void
}

define void @_EN4main3fooE1pOP1S(%S* %p) {
  ret void
}
