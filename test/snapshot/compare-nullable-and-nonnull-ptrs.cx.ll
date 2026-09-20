
define void @_EN4main1fEP3intOP3int(ptr %foo, ptr %bar) {
  %foo1 = alloca ptr, align 8
  %bar2 = alloca ptr, align 8
  store ptr %foo, ptr %foo1, align 8
  store ptr %bar, ptr %bar2, align 8
  %foo.load = load ptr, ptr %foo1, align 8
  %bar.load = load ptr, ptr %bar2, align 8
  %1 = icmp eq ptr %foo.load, %bar.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}
