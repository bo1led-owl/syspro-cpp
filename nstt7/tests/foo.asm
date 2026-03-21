mov r2, 5
mov r1, 1

loop:
mul r1, r2
sub r2, 1
bne r2, loop

hlt
