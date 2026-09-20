
%B = type {}

@0 = private unnamed_addr constant [57 x i8] c"integer overflow at interface-method-definition.cx:5:26\0A\00", align 1

define i32 @main() {
  %b = alloca %B, align 8
  %x = alloca i32, align 4
  call void @_EN4main1B4initE(ptr %b)
  %1 = call i32 @_EN4main1B1gE(ptr %b)
  store i32 %1, ptr %x, align 4
  ret i32 0
}

define void @_EN4main1B4initE(ptr %this) {
  ret void
}

define i32 @_EN4main1B1gE(ptr %this) {
  %1 = call i32 @_EN4main1B1fE(ptr %this)
  %2 = sext i32 %1 to i128
  %3 = mul i128 %2, 2
  %4 = trunc i128 %3 to i32
  %5 = sext i32 %4 to i128
  %6 = icmp ne i128 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %0
  ret i32 %4
}

define i32 @_EN4main1B1fE(ptr %this) {
  ret i32 21
}

declare void @_EN3std10assertFailEP4char(ptr)
