
declare i1 @a()

declare i1 @b()

declare i1 @c()

define i32 @main() {
  %and = alloca i1
  %and2 = alloca i1
  %or = alloca i1
  %or2 = alloca i1
  %1 = call i1 @a()
  br i1 %1, label %andRHS, label %andEnd

andRHS:                                           ; preds = %0
  %2 = call i1 @b()
  br label %andEnd

andEnd:                                           ; preds = %andRHS, %0
  %and1 = phi i1 [ %1, %0 ], [ %2, %andRHS ]
  store i1 %and1, i1* %and
  %3 = call i1 @a()
  br i1 %3, label %andRHS4, label %andEnd5

andRHS2:                                          ; preds = %andEnd5
  %4 = call i1 @c()
  br label %andEnd3

andEnd3:                                          ; preds = %andRHS2, %andEnd5
  %and7 = phi i1 [ %and6, %andEnd5 ], [ %4, %andRHS2 ]
  store i1 %and7, i1* %and2
  %5 = call i1 @a()
  br i1 %5, label %orEnd, label %orRHS

andRHS4:                                          ; preds = %andEnd
  %6 = call i1 @b()
  br label %andEnd5

andEnd5:                                          ; preds = %andRHS4, %andEnd
  %and6 = phi i1 [ %3, %andEnd ], [ %6, %andRHS4 ]
  br i1 %and6, label %andRHS2, label %andEnd3

orRHS:                                            ; preds = %andEnd3
  %7 = call i1 @b()
  br label %orEnd

orEnd:                                            ; preds = %orRHS, %andEnd3
  %or8 = phi i1 [ %5, %andEnd3 ], [ %7, %orRHS ]
  store i1 %or8, i1* %or
  %8 = call i1 @a()
  br i1 %8, label %orEnd12, label %orRHS11

orRHS9:                                           ; preds = %orEnd12
  %9 = call i1 @c()
  br label %orEnd10

orEnd10:                                          ; preds = %orRHS9, %orEnd12
  %or14 = phi i1 [ %or13, %orEnd12 ], [ %9, %orRHS9 ]
  store i1 %or14, i1* %or2
  ret i32 0

orRHS11:                                          ; preds = %orEnd
  %10 = call i1 @b()
  br label %orEnd12

orEnd12:                                          ; preds = %orRHS11, %orEnd
  %or13 = phi i1 [ %8, %orEnd ], [ %10, %orRHS11 ]
  br i1 %or13, label %orEnd10, label %orRHS9
}
