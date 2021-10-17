
%X = type { i32 }

define i1 @_EN4mainltEP1XP1X(%X* %a, %X* %b) {
  ret i1 true
}

define void @_EN4main2fxEP1XP1XP4voidP4void(%X* %a, %X* %b, i8* %v1, i8* %v2) {
  %1 = icmp eq %X* %a, %b
  %2 = icmp ult %X* %a, %b
  %3 = call i1 @_EN4mainltEP1XP1X(%X* %a, %X* %b)
  %4 = icmp ne i8* %v1, %v2
  %5 = icmp uge i8* %v1, %v2
  ret void
}
