mov r7, 42
mov r2, 32

write:
st [r7+r2], r2
sub r2, 1
bne r2, write

mov r2, 32
mov r1, rz

read:
ld r3, [r7+r2]
add r1, r3
sub r2, 1
bne r2, read

hlt
