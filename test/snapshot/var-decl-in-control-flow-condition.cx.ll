
define void @_EN4main1fE() {
  %c = alloca i32*, align 8
  %c1 = alloca i32*, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.body, %0
  %1 = call i32* @_EN4main1hE()
  store i32* %1, i32** %c, align 8
  %c.load = load i32*, i32** %c, align 8
  %2 = icmp ne i32* %c.load, null
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %c.load2 = load i32*, i32** %c, align 8
  call void @_EN4main1jEP3int(i32* %c.load2)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %3 = call i32* @_EN4main1hE()
  store i32* %3, i32** %c1, align 8
  %c.load3 = load i32*, i32** %c1, align 8
  %4 = icmp ne i32* %c.load3, null
  br i1 %4, label %if.then, label %if.else

if.then:                                          ; preds = %loop.end
  %c.load4 = load i32*, i32** %c1, align 8
  call void @_EN4main1jEP3int(i32* %c.load4)
  br label %if.end

if.else:                                          ; preds = %loop.end
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define i32* @_EN4main1hE() {
  ret i32* null
}

define void @_EN4main1jEP3int(i32* %i) {
  ret void
}
