
define i1 @_EN4mainltEP1XP1X(ptr %a, ptr %b) {
  %a1 = alloca ptr, align 8
  %b2 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  store ptr %b, ptr %b2, align 8
  ret i1 true
}

define void @_EN4main2fxEP1XP1XP4voidP4void(ptr %a, ptr %b, ptr %v1, ptr %v2) {
  %a1 = alloca ptr, align 8
  %b2 = alloca ptr, align 8
  %v13 = alloca ptr, align 8
  %v24 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  store ptr %b, ptr %b2, align 8
  store ptr %v1, ptr %v13, align 8
  store ptr %v2, ptr %v24, align 8
  %a.load = load ptr, ptr %a1, align 8
  %b.load = load ptr, ptr %b2, align 8
  %1 = icmp eq ptr %a.load, %b.load
  %a.load5 = load ptr, ptr %a1, align 8
  %b.load6 = load ptr, ptr %b2, align 8
  %2 = icmp ult ptr %a.load5, %b.load6
  %a.load7 = load ptr, ptr %a1, align 8
  %b.load8 = load ptr, ptr %b2, align 8
  %3 = call i1 @_EN4mainltEP1XP1X(ptr %a.load7, ptr %b.load8)
  %v1.load = load ptr, ptr %v13, align 8
  %v2.load = load ptr, ptr %v24, align 8
  %4 = icmp ne ptr %v1.load, %v2.load
  %v1.load9 = load ptr, ptr %v13, align 8
  %v2.load10 = load ptr, ptr %v24, align 8
  %5 = icmp uge ptr %v1.load9, %v2.load10
  ret void
}
