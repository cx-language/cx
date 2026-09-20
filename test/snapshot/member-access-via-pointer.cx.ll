
%Foo = type { i32 }

define void @_EN4main3getEP3Foo(ptr %f) {
  %f1 = alloca ptr, align 8
  %m = alloca i32, align 4
  store ptr %f, ptr %f1, align 8
  %f.load = load ptr, ptr %f1, align 8
  %i = getelementptr inbounds %Foo, ptr %f.load, i32 0, i32 0
  %i.load = load i32, ptr %i, align 4
  store i32 %i.load, ptr %m, align 4
  ret void
}

define i32 @main() {
  %f = alloca %Foo, align 8
  %rf = alloca ptr, align 8
  %n = alloca i32, align 4
  store ptr %f, ptr %rf, align 8
  %rf.load = load ptr, ptr %rf, align 8
  %i = getelementptr inbounds %Foo, ptr %rf.load, i32 0, i32 0
  %i.load = load i32, ptr %i, align 4
  store i32 %i.load, ptr %n, align 4
  %rf.load1 = load ptr, ptr %rf, align 8
  call void @_EN4main3getEP3Foo(ptr %rf.load1)
  ret i32 0
}
