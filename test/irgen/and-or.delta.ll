
declare i1 @a()

declare i1 @b()

declare i1 @c()

define i32 @main() {
  %and = alloca i1
  %and2 = alloca i1
  %or = alloca i1
  %or2 = alloca i1
  %1 = call i1 @a()
  br i1 %1, label %and.rhs, label %and.end

and.rhs:                                          ; preds = %0
  %2 = call i1 @b()
  br label %and.end

and.end:                                          ; preds = %and.rhs, %0
  %and1 = phi i1 [ %1, %0 ], [ %2, %and.rhs ]
  store i1 %and1, i1* %and
  %3 = call i1 @a()
  br i1 %3, label %and.rhs4, label %and.end5

and.rhs2:                                         ; preds = %and.end5
  %4 = call i1 @c()
  br label %and.end3

and.end3:                                         ; preds = %and.rhs2, %and.end5
  %and7 = phi i1 [ %and6, %and.end5 ], [ %4, %and.rhs2 ]
  store i1 %and7, i1* %and2
  %5 = call i1 @a()
  br i1 %5, label %or.end, label %or.rhs

and.rhs4:                                         ; preds = %and.end
  %6 = call i1 @b()
  br label %and.end5

and.end5:                                         ; preds = %and.rhs4, %and.end
  %and6 = phi i1 [ %3, %and.end ], [ %6, %and.rhs4 ]
  br i1 %and6, label %and.rhs2, label %and.end3

or.rhs:                                           ; preds = %and.end3
  %7 = call i1 @b()
  br label %or.end

or.end:                                           ; preds = %or.rhs, %and.end3
  %or8 = phi i1 [ %5, %and.end3 ], [ %7, %or.rhs ]
  store i1 %or8, i1* %or
  %8 = call i1 @a()
  br i1 %8, label %or.end12, label %or.rhs11

or.rhs9:                                          ; preds = %or.end12
  %9 = call i1 @c()
  br label %or.end10

or.end10:                                         ; preds = %or.rhs9, %or.end12
  %or14 = phi i1 [ %or13, %or.end12 ], [ %9, %or.rhs9 ]
  store i1 %or14, i1* %or2
  ret i32 0

or.rhs11:                                         ; preds = %or.end
  %10 = call i1 @b()
  br label %or.end12

or.end12:                                         ; preds = %or.rhs11, %or.end
  %or13 = phi i1 [ %8, %or.end ], [ %10, %or.rhs11 ]
  br i1 %or13, label %or.end10, label %or.rhs9
}
