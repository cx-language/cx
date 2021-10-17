
%X = type { i32 }

define void @_EN4main2fxEP1XP1X(%X* %a, %X* %b) {
  %1 = icmp eq %X* %a, %b
  %2 = icmp ult %X* %a, %b
  ret void
}
