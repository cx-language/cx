
%StringIterator = type { i8*, i8* }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [4 x i8] c"abc\00", align 1

define i32 @main() {
  %__iterator = alloca %StringIterator
  %__str0 = alloca %string
  %ch = alloca i8
  call void @_EN3std6string4initE7pointerP4char6length3int(%string* %__str0, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3)
  %1 = call %StringIterator @_EN3std6string8iteratorE(%string* %__str0)
  store %StringIterator %1, %StringIterator* %__iterator
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std14StringIterator8hasValueE(%StringIterator* %__iterator)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call i8 @_EN3std14StringIterator5valueE(%StringIterator* %__iterator)
  store i8 %3, i8* %ch
  %ch.load = load i8, i8* %ch
  %4 = icmp eq i8 %ch.load, 98
  br i1 %4, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end, %if.then
  call void @_EN3std14StringIterator9incrementE(%StringIterator* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i32 0

if.then:                                          ; preds = %loop.body
  br label %loop.increment

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.increment
}

declare %StringIterator @_EN3std6string8iteratorE(%string*)

define void @_EN3std6string4initE7pointerP4char6length3int(%string* %this, i8* %pointer, i32 %length) {
  %1 = alloca %"ArrayRef<char>"
  %characters = getelementptr inbounds %string, %string* %this, i32 0, i32 0
  call void @_EN3std8ArrayRefI4charE4initE4dataP4char4size3int(%"ArrayRef<char>"* %1, i8* %pointer, i32 %length)
  %.load = load %"ArrayRef<char>", %"ArrayRef<char>"* %1
  store %"ArrayRef<char>" %.load, %"ArrayRef<char>"* %characters
  ret void
}

declare i1 @_EN3std14StringIterator8hasValueE(%StringIterator*)

declare i8 @_EN3std14StringIterator5valueE(%StringIterator*)

declare void @_EN3std14StringIterator9incrementE(%StringIterator*)

declare void @_EN3std8ArrayRefI4charE4initE4dataP4char4size3int(%"ArrayRef<char>"*, i8*, i32)
