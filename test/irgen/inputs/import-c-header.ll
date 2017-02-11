%Foo = type { i32, i8* }

@0 = private unnamed_addr constant [8 x i8] c"foo bar\00"
@MAGIC_NUMBER = private constant i32 5
@1 = private unnamed_addr constant [4 x i8] c"foo\00"

define i32 @main() {
  %1 = call i32 @puts(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @0, i32 0, i32 0))
  %f = alloca %Foo
  %MAGIC_NUMBER = load i32, i32* @MAGIC_NUMBER
  %2 = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  store i32 %MAGIC_NUMBER, i32* %2
  %3 = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 1
  store i8* getelementptr inbounds ([4 x i8], [4 x i8]* @1, i32 0, i32 0), i8** %3
  %bar = alloca i32
  %4 = call i32 @getBar(%Foo* %f)
  store i32 %4, i32* %bar
  ret i32 0
}

declare i32 @puts(i8*)

declare i32 @getBar(%Foo*)
