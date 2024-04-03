; Project: Salis
; Author:  Paul Oliver
; Email:   contact@pauloliver.dev

; Based on the original 55.anc ancestor from salis-v1:
; https://git.pauloliver.dev/salis-v1/tree/bin/genomes/55.anc
; This organism replicates bidirectionally.

; begin template
loka

; measure gene
adrb
keya
adrf
keya
nop1
incn
nop1
subn
nop1
nop1

; alloc gene
lokb
notn
nop3
pshn
nop1
pshn
nop3
ifnz
nop3
jmpf
keyc
allb
nop1
nop2
jmpf
keyd
lokc
allf
nop1
nop2

; copy gene
lokd
load
nop0
nop3
wrte
nop2
nop3
incn
incn
nop2
decn
nop1
ifnz
nop1
jmpb
keyd

; split gene
splt
popn
nop3
popn
nop1
jmpb
keyb

; end template
loka
