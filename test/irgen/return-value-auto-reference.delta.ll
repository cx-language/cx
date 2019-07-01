
%A = type { [2 x i1] }
%B = type { i8 }

@i = private global i32 42

define i32* @_EN4main3fooE() {
  ret i32* @i
}

define i1* @_EN4main1A2ffE(%A* %this) {
  %b = getelementptr inbounds %A, %A* %this, i32 0, i32 0
  %1 = getelementptr [2 x i1], [2 x i1]* %b, i32 0, i32 1
  ret i1* %1
}

define i8* @_EN4main1B2ggE(%B* %this) {
  %a = getelementptr inbounds %B, %B* %this, i32 0, i32 0
  ret i8* %a
}
