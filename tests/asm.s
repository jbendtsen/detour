.global dummyBegin, sayHello, dummyEnd

dummyBegin:
	.ascii "Hello"

sayHello:
	push $0
	mov $title, %eax
	sub $sayHello, %eax
	add $0x20000000, %eax
	push %eax
	mov $msg, %eax
	sub $sayHello, %eax
	add $0x20000000, %eax
	push %eax
	push $0
	mov $0x48c1c4, %eax
	call *(%eax)
	mov $0x420640, %eax
	jmp *%eax

title:
	.ascii "Hello!\0"
msg:
	.ascii "This was called from a function that was installed into a custom memory page in this process.\0"

dummyEnd:
	.ascii "Hello"