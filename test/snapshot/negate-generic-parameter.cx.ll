
define i32 @main() {
  %1 = call float @_EN4main1fI5floatEE5float(float 0.000000e+00)
  ret i32 0
}

define float @_EN4main1fI5floatEE5float(float %a) {
  %a1 = alloca float, align 4
  store float %a, ptr %a1, align 4
  %a.load = load float, ptr %a1, align 4
  %1 = fneg float %a.load
  ret float %1
}
