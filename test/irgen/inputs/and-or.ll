define void @and() {
  %a = alloca i1
  store i1 true, i1* %a
  %b = alloca i1
  store i1 false, i1* %b
  %c = alloca i1
  %a1 = load i1, i1* %a
  br i1 %a1, label %andRHS, label %andEnd

andRHS:                                           ; preds = %0
  %b2 = load i1, i1* %b
  br label %andEnd

andEnd:                                           ; preds = %andRHS, %0
  %and = phi i1 [ %a1, %0 ], [ %b2, %andRHS ]
  store i1 %and, i1* %c
  ret void
}

define void @or() {
  %a = alloca i1
  store i1 true, i1* %a
  %b = alloca i1
  store i1 false, i1* %b
  %c = alloca i1
  %a1 = load i1, i1* %a
  br i1 %a1, label %orEnd, label %orRHS

orRHS:                                            ; preds = %0
  %b2 = load i1, i1* %b
  br label %orEnd

orEnd:                                            ; preds = %orRHS, %0
  %or = phi i1 [ %a1, %0 ], [ %b2, %orRHS ]
  store i1 %or, i1* %c
  ret void
}
