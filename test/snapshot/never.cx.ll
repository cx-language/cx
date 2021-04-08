
%never = type {}

define %never @_EN4main1fE() {
  %1 = call %never @_EN4main1fE()
  unreachable
}

define %never @_EN4main1gE() {
  %1 = call %never @abort()
  unreachable
}

declare %never @abort()

define %never @_EN4main1hE() {
  unreachable
}

define void @_EN4main1iE() {
  %1 = call %never @_EN4main1fE()
  ret void
}

define i32 @_EN4main1iE3int(i32 %x) {
  switch i32 %x, label %switch.default [
    i32 0, label %switch.case.0
  ]

switch.case.0:                                    ; preds = %0
  ret i32 1

switch.default:                                   ; preds = %0
  %1 = call %never @_EN4main1fE()
  br label %switch.end

switch.end:                                       ; preds = %switch.default
  unreachable
}
