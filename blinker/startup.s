

.globl _start
_start:

    li $16,1
    li $17,2

    li $sp,0x1000
    .set    noreorder
    jal notmain
    li $18,3
    .set    reorder
hang:
    j hang
    nop

.globl PUT32
PUT32:
    .set    noreorder
    jr $ra
    sw $a1,0($a0)
    .set    reorder

.globl GET32
GET32:
    .set    noreorder
    jr $ra
    lw $v0,0($a0)
    .set    reorder

.globl dummy
dummy:
    jr $ra
    nop
