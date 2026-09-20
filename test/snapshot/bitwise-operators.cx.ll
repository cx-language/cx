
define i32 @main(i32 %a, i32 %b, i32 %c, i32 %d) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  %c3 = alloca i32, align 4
  %d4 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  store i32 %c, ptr %c3, align 4
  store i32 %d, ptr %d4, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = and i32 %a.load, %b.load
  %a.load5 = load i32, ptr %a1, align 4
  %b.load6 = load i32, ptr %b2, align 4
  %2 = or i32 %a.load5, %b.load6
  %a.load7 = load i32, ptr %a1, align 4
  %b.load8 = load i32, ptr %b2, align 4
  %3 = xor i32 %a.load7, %b.load8
  %a.load9 = load i32, ptr %a1, align 4
  %b.load10 = load i32, ptr %b2, align 4
  %4 = shl i32 %a.load9, %b.load10
  %a.load11 = load i32, ptr %a1, align 4
  %b.load12 = load i32, ptr %b2, align 4
  %5 = ashr i32 %a.load11, %b.load12
  %c.load = load i32, ptr %c3, align 4
  %d.load = load i32, ptr %d4, align 4
  %6 = lshr i32 %c.load, %d.load
  %a.load13 = load i32, ptr %a1, align 4
  %7 = xor i32 %a.load13, -1
  ret i32 0
}
