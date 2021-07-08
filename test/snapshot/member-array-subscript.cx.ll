
%C = type { %"ArrayRef<int>" }
%"ArrayRef<int>" = type { i32*, i32 }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }
%never = type {}

@0 = private unnamed_addr constant [11 x i8] c"operator[]\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"ArrayRef.\00", align 1
@2 = private unnamed_addr constant [9 x i8] c": index \00", align 1
@3 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1

define i32 @main() {
  %1 = alloca %C, align 8
  %2 = alloca [5 x i32], align 4
  store [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4], [5 x i32]* %2, align 4
  %3 = getelementptr inbounds [5 x i32], [5 x i32]* %2, i32 0, i32 0
  %4 = insertvalue %"ArrayRef<int>" undef, i32* %3, 0
  %5 = insertvalue %"ArrayRef<int>" %4, i32 5, 1
  call void @_EN4main1C4initE8ArrayRefI3intE(%C* %1, %"ArrayRef<int>" %5)
  call void @_EN4main1C3fooE(%C* %1)
  ret i32 0
}

define void @_EN4main1C4initE8ArrayRefI3intE(%C* %this, %"ArrayRef<int>" %a) {
  %a1 = getelementptr inbounds %C, %C* %this, i32 0, i32 0
  store %"ArrayRef<int>" %a, %"ArrayRef<int>"* %a1, align 8
  ret void
}

define void @_EN4main1C3fooE(%C* %this) {
  %a = getelementptr inbounds %C, %C* %this, i32 0, i32 0
  %1 = call i32* @_EN3std8ArrayRefI3intEixE3int(%"ArrayRef<int>"* %a, i32 4)
  ret void
}

define i32* @_EN3std8ArrayRefI3intEixE3int(%"ArrayRef<int>"* %this, i32 %index) {
  %__str = alloca %string, align 8
  %1 = icmp slt i32 %index, 0
  br i1 %1, label %or.end, label %or.rhs

or.rhs:                                           ; preds = %0
  %2 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %this)
  %3 = icmp sge i32 %index, %2
  br label %or.end

or.end:                                           ; preds = %or.rhs, %0
  %or = phi i1 [ %1, %0 ], [ %3, %or.rhs ]
  br i1 %or, label %if.then, label %if.else

if.then:                                          ; preds = %or.end
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @0, i32 0, i32 0), i32 10)
  %__str.load = load %string, %string* %__str, align 8
  call void @_EN3std8ArrayRefI3intE16indexOutOfBoundsE6string3int(%"ArrayRef<int>"* %this, %string %__str.load, i32 %index)
  br label %if.end

if.else:                                          ; preds = %or.end
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %data = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  %data.load = load i32*, i32** %data, align 8
  %4 = getelementptr inbounds i32, i32* %data.load, i32 %index
  ret i32* %4
}

define i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

declare void @_EN3std6string4initEP4char3int(%string*, i8*, i32)

define void @_EN3std8ArrayRefI3intE16indexOutOfBoundsE6string3int(%"ArrayRef<int>"* %this, %string %function, i32 %index) {
  %__str = alloca %string, align 8
  %1 = alloca %string, align 8
  %__str1 = alloca %string, align 8
  %2 = alloca i32, align 4
  %__str2 = alloca %string, align 8
  %3 = alloca i32, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @1, i32 0, i32 0), i32 9)
  store %string %function, %string* %1, align 8
  call void @_EN3std6string4initEP4char3int(%string* %__str1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @2, i32 0, i32 0), i32 8)
  store i32 %index, i32* %2, align 4
  call void @_EN3std6string4initEP4char3int(%string* %__str2, i8* getelementptr inbounds ([28 x i8], [28 x i8]* @3, i32 0, i32 0), i32 27)
  %4 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %this)
  store i32 %4, i32* %3, align 4
  %5 = call %never @_EN3std5abortI6string6string6string3int6string3intEEP6stringP6stringP6stringP3intP6stringP3int(%string* %__str, %string* %1, %string* %__str1, i32* %2, %string* %__str2, i32* %3)
  ret void
}

declare %never @_EN3std5abortI6string6string6string3int6string3intEEP6stringP6stringP6stringP3intP6stringP3int(%string*, %string*, %string*, i32*, %string*, i32*)
