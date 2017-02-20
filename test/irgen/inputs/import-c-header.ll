%Foo = type { i32, i8* }

@0 = private unnamed_addr constant [8 x i8] c"foo bar\00"
@MAGIC_NUMBER = private constant i32 5
@ANSWER = private constant i32 42
@1 = private unnamed_addr constant [4 x i8] c"foo\00"

define i32 @main() {
  %f = alloca %Foo
  %bar = alloca i32
  %1 = call i32 @puts(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @0, i32 0, i32 0))
  %2 = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  %MAGIC_NUMBER = load i32, i32* @MAGIC_NUMBER
  %ANSWER = load i32, i32* @ANSWER
  %3 = add i32 %MAGIC_NUMBER, %ANSWER
  store i32 %3, i32* %2
  %4 = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 1
  store i8* getelementptr inbounds ([4 x i8], [4 x i8]* @1, i32 0, i32 0), i8** %4
  %5 = call i32 @getBar(%Foo* %f)
  store i32 %5, i32* %bar
  ret i32 0
}

declare i32 @puts(i8*)

declare i32 @getBar(%Foo*)
