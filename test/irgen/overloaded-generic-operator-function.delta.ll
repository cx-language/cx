
define void @_EN4maineqE1a1S1b1S({} %a, {} %b) {
  ret void
}

define void @_EN4main1fE1s1S({} %s) {
  call void @_EN4maineqE1a1S1b1S({} %s, {} %s)
  ret void
}
