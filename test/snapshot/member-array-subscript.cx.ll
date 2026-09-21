
%C = type { %"ArrayRef<int>" }
%"ArrayRef<int>" = type { ptr, i32 }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { ptr, i32 }
%never = type {}

@0 = private unnamed_addr constant [11 x i8] c"operator[]\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"ArrayRef.\00", align 1
@2 = private unnamed_addr constant [9 x i8] c": index \00", align 1
@3 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1

define i32 @main() {
  %1 = alloca %C, align 8
  %2 = alloca [5 x i32], align 4
  store [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4], ptr %2, align 4
  %3 = getelementptr inbounds [5 x i32], ptr %2, i32 0, i32 0
  %4 = insertvalue %"ArrayRef<int>" undef, ptr %3, 0
  %5 = insertvalue %"ArrayRef<int>" %4, i32 5, 1
  call void @_EN4main1C4initE8ArrayRefI3intE(ptr %1, %"ArrayRef<int>" %5)
  call void @_EN4main1C3fooE(ptr %1)
  ret i32 0
}

define void @_EN4main1C4initE8ArrayRefI3intE(ptr %this, %"ArrayRef<int>" %a) {
  %a1 = alloca %"ArrayRef<int>", align 8
  store %"ArrayRef<int>" %a, ptr %a1, align 8
  %a2 = getelementptr inbounds %C, ptr %this, i32 0, i32 0
  %a.load = load %"ArrayRef<int>", ptr %a1, align 8
  store %"ArrayRef<int>" %a.load, ptr %a2, align 8
  ret void
}

define void @_EN4main1C3fooE(ptr %this) {
  %a = getelementptr inbounds %C, ptr %this, i32 0, i32 0
  %1 = call ptr @_EN3std8ArrayRefI3intEixE3int(ptr %a, i32 4)
  ret void
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

define i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
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
  %3 = call %never @_EN3std5abortI6string6string6string3int6string3intEVER6stringR6stringR6stringR3intR6stringR3int(ptr %__str, ptr %function1, ptr %__str3, ptr %index2, ptr %__str4, ptr %1)
  ret void
}

declare %never @_EN3std5abortI6string6string6string3int6string3intEVER6stringR6stringR6stringR3intR6stringR3int(ptr, ptr, ptr, ptr, ptr, ptr)
