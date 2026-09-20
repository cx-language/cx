
define float @_EN4main1fE5float(float %b) {
  %b1 = alloca float, align 4
  store float %b, ptr %b1, align 4
  %b.load = load float, ptr %b1, align 4
  %1 = fmul float %b.load, 8.000000e+00
  ret float %1
}
