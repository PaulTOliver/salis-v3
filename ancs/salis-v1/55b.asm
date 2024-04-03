; Project: Salis
; Author:  Paul Oliver
; Email:   contact@pauloliver.dev

; Based on the original 55.anc ancestor from salis-v1:
; https://git.pauloliver.dev/salis-v1/tree/bin/genomes/55.anc
; This organism replicates bidirectionally.

; begin template
lokb

; measure gene
adrb
keyb
adrf
keyb
nop1
incn
nop1
subn
nop1
nop1

; alloc gene
lokc
notn
nop3
pshn
nop1
pshn
nop3
ifnz
nop3
jmpf
keyd
allb
nop1
nop2
jmpf
keye
lokd
allf
nop1
nop2

; copy gene
loke
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
keye

; split gene
splt
popn
nop3
popn
nop1
jmpb
keyc

; end template
lokb
