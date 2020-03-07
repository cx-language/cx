
define i32 @main() {
  %1 = call float @_EN4main1fI5floatEE1a5float(float 0.000000e+00)
  ret i32 0
}

define float @_EN4main1fI5floatEE1a5float(float %a) {
  %1 = fsub float -0.000000e+00, %a
  ret float %1
}
