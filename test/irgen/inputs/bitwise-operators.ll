define i32 @and(i32 %a, i32 %b) {
  %1 = and i32 %a, %b
  ret i32 %1
}

define i32 @or(i32 %a, i32 %b) {
  %1 = or i32 %a, %b
  ret i32 %1
}

define i32 @xor(i32 %a, i32 %b) {
  %1 = xor i32 %a, %b
  ret i32 %1
}

define i32 @shl(i32 %a, i32 %b) {
  %1 = shl i32 %a, %b
  ret i32 %1
}

define i32 @ashr(i32 %a, i32 %b) {
  %1 = ashr i32 %a, %b
  ret i32 %1
}

define i32 @lshr(i32 %a, i32 %b) {
  %1 = lshr i32 %a, %b
  ret i32 %1
}

define i32 @compl(i32 %a) {
  %1 = xor i32 %a, -1
  ret i32 %1
}
