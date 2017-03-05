define i32 @main() {
  %i = alloca i32
  store i32 0, i32* %i
  %1 = load i32, i32* %i
  %2 = add i32 %1, 1
  store i32 %2, i32* %i
  %3 = load i32, i32* %i
  %4 = sub i32 %3, 1
  store i32 %4, i32* %i
  %5 = load i32, i32* %i
  %6 = mul i32 %5, 1
  store i32 %6, i32* %i
  %7 = load i32, i32* %i
  %8 = sdiv i32 %7, 1
  store i32 %8, i32* %i
  %9 = load i32, i32* %i
  %10 = and i32 %9, 1
  store i32 %10, i32* %i
  %11 = load i32, i32* %i
  %12 = or i32 %11, 1
  store i32 %12, i32* %i
  %13 = load i32, i32* %i
  %14 = xor i32 %13, 1
  store i32 %14, i32* %i
  %15 = load i32, i32* %i
  %16 = shl i32 %15, 1
  store i32 %16, i32* %i
  %17 = load i32, i32* %i
  %18 = ashr i32 %17, 1
  store i32 %18, i32* %i
  ret i32 0
}
