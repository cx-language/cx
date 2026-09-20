
%"ArrayRef<int>" = type { ptr, i32 }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { ptr, i32 }
%never = type {}

@0 = private unnamed_addr constant [11 x i8] c"operator[]\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"ArrayRef.\00", align 1
@2 = private unnamed_addr constant [9 x i8] c": index \00", align 1
@3 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1

define i32 @main() {
  %three = alloca [3 x i32], align 4
  %b = alloca i32, align 4
  %ref = alloca %"ArrayRef<int>", align 8
  %bb = alloca i32, align 4
  store [3 x i32] [i32 0, i32 42, i32 0], ptr %three, align 4
  %1 = getelementptr inbounds [3 x i32], ptr %three, i32 0, i32 0
  %2 = insertvalue %"ArrayRef<int>" undef, ptr %1, 0
  %3 = insertvalue %"ArrayRef<int>" %2, i32 3, 1
  call void @_EN4main3fooE8ArrayRefI3intE(%"ArrayRef<int>" %3)
  call void @_EN4main3barEPA3_3int(ptr %three)
  store i32 3, ptr %b, align 4
  %4 = getelementptr inbounds [3 x i32], ptr %three, i32 0, i32 0
  %5 = insertvalue %"ArrayRef<int>" undef, ptr %4, 0
  %6 = insertvalue %"ArrayRef<int>" %5, i32 3, 1
  store %"ArrayRef<int>" %6, ptr %ref, align 8
  %7 = call i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %ref)
  store i32 %7, ptr %bb, align 4
  ret i32 0
}

define void @_EN4main3fooE8ArrayRefI3intE(%"ArrayRef<int>" %ints) {
  %ints1 = alloca %"ArrayRef<int>", align 8
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca %"ArrayRef<int>", align 8
  store %"ArrayRef<int>" %ints, ptr %ints1, align 8
  %1 = call ptr @_EN3std8ArrayRefI3intEixE3int(ptr %ints1, i32 1)
  %.load = load i32, ptr %1, align 4
  store i32 %.load, ptr %a, align 4
  %2 = call i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %ints1)
  store i32 %2, ptr %b, align 4
  %ints.load = load %"ArrayRef<int>", ptr %ints1, align 8
  store %"ArrayRef<int>" %ints.load, ptr %c, align 8
  ret void
}

define void @_EN4main3barEPA3_3int(ptr %ints) {
  %ints1 = alloca ptr, align 8
  %b = alloca i32, align 4
  %ref = alloca %"ArrayRef<int>", align 8
  store ptr %ints, ptr %ints1, align 8
  store i32 3, ptr %b, align 4
  %ints.load = load ptr, ptr %ints1, align 8
  %1 = getelementptr inbounds [3 x i32], ptr %ints.load, i32 0, i32 0
  %2 = insertvalue %"ArrayRef<int>" undef, ptr %1, 0
  %3 = insertvalue %"ArrayRef<int>" %2, i32 3, 1
  store %"ArrayRef<int>" %3, ptr %ref, align 8
  ret void
}

define i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

define ptr @_EN3std8ArrayRefI3intEixE3int(ptr %this, i32 %index) {
  %index1 = alloca i32, align 4
  %__str = alloca %string, align 8
  store i32 %index, ptr %index1, align 4
  %index.load = load i32, ptr %index1, align 4
  %1 = icmp slt i32 %index.load, 0
  br i1 %1, label %or.end, label %or.rhs

or.rhs:                                           ; preds = %0
  %index.load2 = load i32, ptr %index1, align 4
  %2 = call i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %this)
  %3 = icmp sge i32 %index.load2, %2
  br label %or.end

or.end:                                           ; preds = %or.rhs, %0
  %or = phi i1 [ %1, %0 ], [ %3, %or.rhs ]
  br i1 %or, label %if.then, label %if.else

if.then:                                          ; preds = %or.end
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @0, i32 10)
  %__str.load = load %string, ptr %__str, align 8
  %index.load3 = load i32, ptr %index1, align 4
  call void @_EN3std8ArrayRefI3intE16indexOutOfBoundsE6string3int(ptr %this, %string %__str.load, i32 %index.load3)
  br label %if.end

if.else:                                          ; preds = %or.end
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %data = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 0
  %data.load = load ptr, ptr %data, align 8
  %index.load4 = load i32, ptr %index1, align 4
  %4 = getelementptr inbounds i32, ptr %data.load, i32 %index.load4
  ret ptr %4
}

declare void @_EN3std6string4initEP4char3int(ptr, ptr, i32)

define void @_EN3std8ArrayRefI3intE16indexOutOfBoundsE6string3int(ptr %this, %string %function, i32 %index) {
  %function1 = alloca %string, align 8
  %index2 = alloca i32, align 4
  %__str = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %1 = alloca i32, align 4
  store %string %function, ptr %function1, align 8
  store i32 %index, ptr %index2, align 4
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @1, i32 9)
  call void @_EN3std6string4initEP4char3int(ptr %__str3, ptr @2, i32 8)
  call void @_EN3std6string4initEP4char3int(ptr %__str4, ptr @3, i32 27)
  %2 = call i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %this)
  store i32 %2, ptr %1, align 4
  %3 = call %never @_EN3std5abortI6string6string6string3int6string3intEVEP6stringP6stringP6stringP3intP6stringP3int(ptr %__str, ptr %function1, ptr %__str3, ptr %index2, ptr %__str4, ptr %1)
  ret void
}

declare %never @_EN3std5abortI6string6string6string3int6string3intEVEP6stringP6stringP6stringP3intP6stringP3int(ptr, ptr, ptr, ptr, ptr, ptr)
