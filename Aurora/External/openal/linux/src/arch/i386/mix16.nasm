	;; short *dst, short *src, int len
	;;	
	global mix16_1
mix16_1:	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx

	mov eax, [ebp+8]    ; dst
	mov ebx, [ebp+12]   ; src
	mov ecx, [ebp+16]   ; len

.loop
	cmp ecx, 4
	jl .twocomp

	;; load data
	movq mm0, [eax]
	movq mm1, [ebx]

	;; add
	paddsw mm0, mm1

	;; write back
	movq [eax], mm0

	;; increment dst, src
	add eax, 8
	add ebx, 8

	;; decrement len
	sub ecx, 4
	jmp .loop

;; now do the rest for %2
.twocomp
	cmp ecx, 2
	jl .rest

	;; load data
	movd mm0, [eax]
	movd mm1, [ebx]

	;; add
	paddsw mm0, mm1

	;; write back
	movd [eax], mm0

	;; increment dst
	add eax, 4
	add ebx, 4

	;; decrement len
	sub ecx, 2
	jmp .twocomp

;; now for %1
.rest
	cmp ecx, 0
	jle .done

	mov cx, [eax]
	add cx, word [ebx]

	cmp ecx, 32767       ;; max value
	jg .movemax
	cmp ecx, -32768  ;; min value
	jl .movemin
	mov [eax], cx
.done
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret

.movemin
	mov word [eax], -32768

	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp
	
	emms
	ret

.movemax
	mov word [eax], 32767

	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret









	;; short *dst, short **srcs, int len
	;;	
	global mix16_2
mix16_2:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx

	mov eax, [ebp+8]    ; dst
	mov ecx, [ebp+16]   ; len
	mov ebx, [ebp+12]   ; srcs
.loop
	cmp ecx, 4
	jl .twocomp

	;; load data
	

	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movq mm0, [eax]   ;; mov dst[0..3] into mm0
	movq mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movq mm2, [edx]   ;; mov srcs[1][0..3] into mm2

	;; add
	paddsw mm0, mm1
	paddsw mm0, mm2

	;; write back
	movq [eax], mm0

	;; increment dst, src[0, 1]
	add word [ebx],   8
	add eax, 8
	add word [ebx+4], 8

	;; decrement len
	sub ecx, 4
	jmp .loop

;; now do the rest for %2
.twocomp
	cmp ecx, 2
	jl .rest

	;; load data
	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movd mm0, [eax]   ;; mov dst[0..3] into mm0
	movd mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movd mm2, [edx]   ;; mov srcs[1][0..3] into mm2

	;; add
	paddsw mm0, mm1
	paddsw mm0, mm2

	;; write back
	movd [eax], mm0

	;; increment dst
	add word [ebx], 4
	add eax, 4
	add word [ebx+4], 4

	;; decrement len
	sub ecx, 2
	jmp .twocomp

;; now for %1
.rest
	cmp ecx, 0
	jle .done

	mov cx, [eax]
	add cx, word [ebx]
	add cx, word [ebx+4]

	cmp ecx, 32767       ;; max value
	jg .movemax
	cmp ecx, -32768  ;; min value
	jl .movemin
	mov [eax], cx
.done
	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret

.movemin
	mov word [eax], -32768

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp
	
	emms
	ret

.movemax
	mov word [eax], 32767

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret




	;; short *dst, short **srcs, int len
	;;	
	global mix16_3
mix16_3:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx

	mov eax, [ebp+8]    ; dst
	mov ecx, [ebp+16]   ; len
	mov ebx, [ebp+12]   ; srcs
.loop
	cmp ecx, 4
	jl .twocomp

	;; load data
	

	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movq mm0, [eax]   ;; mov dst[0..3] into mm0
	movq mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movq mm2, [edx]   ;; mov srcs[1][0..3] into mm2
	mov edx, [ebx+8]  ;; mov srcs[2] into edx
	movq mm3, [edx]   ;; mov srcs[2][0..3] into mm3

	;; add
	paddsw mm0, mm1
	paddsw mm0, mm2
	paddsw mm0, mm3

	;; write back
	movq [eax], mm0

	;;
	;; Sorry for the code unclarity here.  It's
	;; very simple but the references are interleaved
	;;
	add word [ebx],   8 ;; increment src[0]
	add eax, 8          ;; increment dst
	add word [ebx+4], 8 ;; increment src[1]
	sub ecx, 4          ;; decrement len
	add word [ebx+8], 8 ;; increment src[2]

	jmp .loop

;; now do the rest for %2
.twocomp
	cmp ecx, 2
	jl .rest

	;; load data
	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movd mm0, [eax]   ;; mov dst[0..3] into mm0
	movd mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movd mm2, [edx]   ;; mov srcs[1][0..3] into mm2
	mov edx, [ebx+8]  ;; mov srcs[2] into edx
	movd mm3, [edx]   ;; mov srcs[2][0..3] into mm3

	;; add
	paddsw mm0, mm1
	paddsw mm0, mm2
	paddsw mm0, mm3

	;; write back
	movd [eax], mm0

	;; increment dst
	add word [ebx], 4   ;; increment src[0]
	add eax, 4          ;; increment dst
	add word [ebx+4], 4 ;; increment src[1]
	sub ecx, 2          ;; decrement len
	add word [ebx+8], 4 ;; increment src[2]
	
	jmp .twocomp

;; now for %1
.rest
	cmp ecx, 0
	jle .done

	mov cx, [eax]
	add cx, word [ebx]
	add cx, word [ebx+4]
	add cx, word [ebx+8]

	cmp ecx, 32767       ;; max value
	jg .movemax
	cmp ecx, -32768  ;; min value
	jl .movemin
	mov [eax], cx
.done
	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret

.movemin
	mov word [eax], -32768

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp
	
	emms
	ret

.movemax
	mov word [eax], 32767

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret




	;; short *dst, short **srcs, int len
	;;	
	global mix16_4
mix16_4:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx

	mov eax, [ebp+8]    ; dst
	mov ecx, [ebp+16]   ; len
	mov ebx, [ebp+12]   ; srcs
.loop
	cmp ecx, 4
	jl .twocomp

	;; load data

	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movq mm0, [eax]   ;; mov dst[0..3] into mm0
	movq mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movq mm2, [edx]   ;; mov srcs[1][0..3] into mm2
	mov edx, [ebx+8]  ;; mov srcs[2] into edx
	movq mm3, [edx]   ;; mov srcs[2][0..3] into mm3
	mov edx, [ebx+12]  ;; mov srcs[3] into edx
	movq mm4, [edx]   ;; mov srcs[3][0..3] into mm4

	;; add
	paddsw mm1, mm2
	paddsw mm3, mm4
	paddsw mm0, mm1
	paddsw mm0, mm3

	;; write back
	movq [eax], mm0

	;;
	;; Sorry for the code unclarity here.  It's
	;; very simple but the references are interleaved
	;;
	;; FIXME: we could do an mmx add with a register like:
	;; mmx: 0x0004000400040004 for src[0-3]
	add word [ebx],   8 ;; increment src[0]
	add eax, 8          ;; increment dst
	add word [ebx+4], 8 ;; increment src[1]
	sub ecx, 4          ;; decrement len
	add word [ebx+8], 8 ;; increment src[2]
	add word [ebx+12], 8 ;; increment src[3]

	jmp .loop

;; now do the rest for %2
.twocomp
	cmp ecx, 2
	jl .rest

	;; load data
	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movd mm0, [eax]   ;; mov dst[0..3] into mm0
	movd mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movd mm2, [edx]   ;; mov srcs[1][0..3] into mm2
	mov edx, [ebx+8]  ;; mov srcs[2] into edx
	movd mm3, [edx]   ;; mov srcs[2][0..3] into mm3
	mov edx, [ebx+12]  ;; mov srcs[3] into edx
	movd mm4, [edx]   ;; mov srcs[3][0..3] into mm4

	;; add
	paddsw mm1, mm2
	paddsw mm3, mm4
	paddsw mm0, mm1
	paddsw mm0, mm3

	;; write back
	movd [eax], mm0

	;; increment dst
	;; FIXME: we could do an mmx add with a register like:
	;; mmx: 0x0004000400040004 for src[0-3]
	add word [ebx], 4   ;; increment src[0]
	add eax, 4          ;; increment dst
	add word [ebx+4], 4 ;; increment src[1]
	sub ecx, 2          ;; decrement len
	add word [ebx+8], 4 ;; increment src[2]
	add word [ebx+12], 4 ;; increment src[3]
	
	jmp .twocomp

;; now for %1
.rest
	cmp ecx, 0
	jle .done

	mov cx, [eax]
	add cx, word [ebx]
	add cx, word [ebx+4]
	add cx, word [ebx+8]
	add cx, word [ebx+12]

	cmp ecx, 32767       ;; max value
	jg .movemax
	cmp ecx, -32768  ;; min value
	jl .movemin
	mov [eax], cx
.done
	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret

.movemin
	mov word [eax], -32768

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp
	
	emms
	ret

.movemax
	mov word [eax], 32767

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret




	;; short *dst, short **srcs, int len
	;;	
	global mix16_5
mix16_5:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx

	mov eax, [ebp+8]    ; dst
	mov ecx, [ebp+16]   ; len
	mov ebx, [ebp+12]   ; srcs
.loop
	cmp ecx, 4
	jl .twocomp

	;; load data

	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movq mm0, [eax]   ;; mov dst[0..3] into mm0
	movq mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movq mm2, [edx]   ;; mov srcs[1][0..3] into mm2
	mov edx, [ebx+8]  ;; mov srcs[2] into edx
	movq mm3, [edx]   ;; mov srcs[2][0..3] into mm3
	mov edx, [ebx+12]  ;; mov srcs[3] into edx
	movq mm4, [edx]   ;; mov srcs[3][0..3] into mm4
	mov edx, [ebx+16]  ;; mov srcs[4] into edx
	movq mm5, [edx]    ;; mov srcs[4][0..3] into mm5

	;; add
	paddsw mm1, mm2
	paddsw mm3, mm4
	paddsw mm0, mm5
	paddsw mm0, mm1
	paddsw mm0, mm3

	;; write back
	movq [eax], mm0

	;;
	;; Sorry for the code unclarity here.  It's
	;; very simple but the references are interleaved
	;;
	;; FIXME: we could do an mmx add with a register like:
	;; mmx: 0x0004000400040004 for src[0-3]
	add word [ebx],   8 ;; increment src[0]
	add eax, 8          ;; increment dst
	add word [ebx+4], 8 ;; increment src[1]
	sub ecx, 4          ;; decrement len
	add word [ebx+8], 8 ;; increment src[2]
	add word [ebx+12], 8 ;; increment src[3]
	add word [ebx+16], 8 ;; increment src[4]

	jmp .loop

;; now do the rest for %2
.twocomp
	cmp ecx, 2
	jl .rest

	;; load data
	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movd mm0, [eax]   ;; mov dst[0..3] into mm0
	movd mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movd mm2, [edx]   ;; mov srcs[1][0..3] into mm2
	mov edx, [ebx+8]  ;; mov srcs[2] into edx
	movd mm3, [edx]   ;; mov srcs[2][0..3] into mm3
	mov edx, [ebx+12]  ;; mov srcs[3] into edx
	movd mm4, [edx]   ;; mov srcs[3][0..3] into mm4
	mov edx, [ebx+16]  ;; mov srcs[4] into edx
	movd mm5, [edx]   ;; mov srcs[4][0..3] into mm5

	;; add
	paddsw mm1, mm2
	paddsw mm3, mm4
	paddsw mm0, mm5
	paddsw mm0, mm1
	paddsw mm0, mm3

	;; write back
	movd [eax], mm0

	;; increment dst
	;; FIXME: we could do an mmx add with a register like:
	;; mmx: 0x0004000400040004 for src[0-3]
	add word [ebx], 4   ;; increment src[0]
	add eax, 4          ;; increment dst
	add word [ebx+4], 4 ;; increment src[1]
	sub ecx, 2          ;; decrement len
	add word [ebx+8], 4 ;; increment src[2]
	add word [ebx+12], 4 ;; increment src[3]
	add word [ebx+16], 4 ;; increment src[4]
	
	jmp .twocomp

;; now for %1
.rest
	cmp ecx, 0
	jle .done

	mov cx, [eax]
	add cx, word [ebx]
	add cx, word [ebx+4]
	add cx, word [ebx+8]
	add cx, word [ebx+12]
	add cx, word [ebx+16]

	cmp ecx, 32767       ;; max value
	jg .movemax
	cmp ecx, -32768  ;; min value
	jl .movemin
	mov [eax], cx
.done
	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret

.movemin
	mov word [eax], -32768

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp
	
	emms
	ret

.movemax
	mov word [eax], 32767

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret


	;; short *dst, short **srcs, int len
	;;	
	global mix16_6
mix16_6:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx

	mov eax, [ebp+8]    ; dst
	mov ecx, [ebp+16]   ; len
	mov ebx, [ebp+12]   ; srcs
.loop
	cmp ecx, 4
	jl .twocomp

	;; load data

	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movq mm0, [eax]   ;; mov dst[0..3] into mm0
	movq mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movq mm2, [edx]   ;; mov srcs[1][0..3] into mm2
	mov edx, [ebx+8]  ;; mov srcs[2] into edx
	movq mm3, [edx]   ;; mov srcs[2][0..3] into mm3
	mov edx, [ebx+12]  ;; mov srcs[3] into edx
	movq mm4, [edx]   ;; mov srcs[3][0..3] into mm4
	mov edx, [ebx+16]  ;; mov srcs[4] into edx
	movq mm5, [edx]    ;; mov srcs[4][0..3] into mm5
	mov edx, [ebx+20]  ;; mov srcs[5] into edx
	movq mm6, [edx]    ;; mov srcs[5][0..3] into mm6

	;; add
	paddsw mm1, mm2
	paddsw mm3, mm4
	paddsw mm5, mm6
	paddsw mm0, mm1
	paddsw mm0, mm3
	paddsw mm0, mm5

	;; write back
	movq [eax], mm0

	;;
	;; Sorry for the code unclarity here.  It's
	;; very simple but the references are interleaved
	;;
	;; FIXME: we could do an mmx add with a register like:
	;; mmx: 0x0004000400040004 for src[0-3]
	add word [ebx],   8 ;; increment src[0]
	add eax, 8          ;; increment dst
	add word [ebx+4], 8 ;; increment src[1]
	sub ecx, 4          ;; decrement len
	add word [ebx+8], 8 ;; increment src[2]
	add word [ebx+12], 8 ;; increment src[3]
	add word [ebx+16], 8 ;; increment src[4]
	add word [ebx+20], 8 ;; increment src[5]

	jmp .loop

;; now do the rest for %2
.twocomp
	cmp ecx, 2
	jl .rest

	;; load data
	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movd mm0, [eax]   ;; mov dst[0..3] into mm0
	movd mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movd mm2, [edx]   ;; mov srcs[1][0..3] into mm2
	mov edx, [ebx+8]  ;; mov srcs[2] into edx
	movd mm3, [edx]   ;; mov srcs[2][0..3] into mm3
	mov edx, [ebx+12]  ;; mov srcs[3] into edx
	movd mm4, [edx]   ;; mov srcs[3][0..3] into mm4
	mov edx, [ebx+16]  ;; mov srcs[4] into edx
	movd mm5, [edx]   ;; mov srcs[4][0..3] into mm5
	mov edx, [ebx+20]  ;; mov srcs[5] into edx
	movd mm6, [edx]   ;; mov srcs[5][0..3] into mm6

	;; add
	paddsw mm1, mm2
	paddsw mm3, mm4
	paddsw mm5, mm6
	paddsw mm0, mm1
	paddsw mm0, mm3
	paddsw mm0, mm5

	;; write back
	movd [eax], mm0

	;; increment dst
	;; FIXME: we could do an mmx add with a register like:
	;; mmx: 0x0004000400040004 for src[0-3]
	add word [ebx], 4   ;; increment src[0]
	add eax, 4          ;; increment dst
	add word [ebx+4], 4 ;; increment src[1]
	sub ecx, 2          ;; decrement len
	add word [ebx+8], 4 ;; increment src[2]
	add word [ebx+12], 4 ;; increment src[3]
	add word [ebx+16], 4 ;; increment src[4]
	add word [ebx+20], 4 ;; increment src[5]
	
	jmp .twocomp

;; now for %1
.rest
	cmp ecx, 0
	jle .done

	mov cx, [eax]
	add cx, word [ebx]
	add cx, word [ebx+4]
	add cx, word [ebx+8]
	add cx, word [ebx+12]
	add cx, word [ebx+16]
	add cx, word [ebx+20]

	cmp ecx, 32767       ;; max value
	jg .movemax
	cmp ecx, -32768  ;; min value
	jl .movemin
	mov [eax], cx
.done
	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret

.movemin
	mov word [eax], -32768

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp
	
	emms
	ret

.movemax
	mov word [eax], 32767

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret

	;; short *dst, short **srcs, int len
	;;	
	global mix16_7
mix16_7:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx

	mov eax, [ebp+8]    ; dst
	mov ecx, [ebp+16]   ; len
	mov ebx, [ebp+12]   ; srcs
.loop
	cmp ecx, 4
	jl .twocomp

	;; load data

	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movq mm0, [eax]   ;; mov dst[0..3] into mm0
	movq mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movq mm2, [edx]   ;; mov srcs[1][0..3] into mm2
	mov edx, [ebx+8]  ;; mov srcs[2] into edx
	movq mm3, [edx]   ;; mov srcs[2][0..3] into mm3
	mov edx, [ebx+12]  ;; mov srcs[3] into edx
	movq mm4, [edx]   ;; mov srcs[3][0..3] into mm4
	mov edx, [ebx+16]  ;; mov srcs[4] into edx
	movq mm5, [edx]    ;; mov srcs[4][0..3] into mm5
	mov edx, [ebx+20]  ;; mov srcs[5] into edx
	movq mm6, [edx]    ;; mov srcs[5][0..3] into mm6
	mov edx, [ebx+24]  ;; mov srcs[6] into edx
	movq mm7, [edx]    ;; mov srcs[6][0..3] into mm7

	;; add
	paddsw mm1, mm2
	paddsw mm3, mm4
	paddsw mm5, mm6
	paddsw mm1, mm3
	paddsw mm5, mm7
	paddsw mm0, mm1
	paddsw mm0, mm5

	;; write back
	movq [eax], mm0

	;;
	;; Sorry for the code unclarity here.  It's
	;; very simple but the references are interleaved
	;;
	;; FIXME: we could do an mmx add with a register like:
	;; mmx: 0x0004000400040004 for src[0-3]
	add word [ebx],   8 ;; increment src[0]
	add eax, 8          ;; increment dst
	add word [ebx+4], 8 ;; increment src[1]
	sub ecx, 4          ;; decrement len
	add word [ebx+8], 8 ;; increment src[2]
	add word [ebx+12], 8 ;; increment src[3]
	add word [ebx+16], 8 ;; increment src[4]
	add word [ebx+20], 8 ;; increment src[5]
	add word [ebx+24], 8 ;; increment src[6]

	jmp .loop

;; now do the rest for %2
.twocomp
	cmp ecx, 2
	jl .rest

	;; load data
	mov edx, [ebx]	  ;; mov srcs[0] into edx
	movd mm0, [eax]   ;; mov dst[0..3] into mm0
	movd mm1, [edx]   ;; mov srcs[0][0..3] into mm1
	mov edx, [ebx+4]  ;; mov srcs[1] into edx
	movd mm2, [edx]   ;; mov srcs[1][0..3] into mm2
	mov edx, [ebx+8]  ;; mov srcs[2] into edx
	movd mm3, [edx]   ;; mov srcs[2][0..3] into mm3
	mov edx, [ebx+12]  ;; mov srcs[3] into edx
	movd mm4, [edx]   ;; mov srcs[3][0..3] into mm4
	mov edx, [ebx+16]  ;; mov srcs[4] into edx
	movd mm5, [edx]   ;; mov srcs[4][0..3] into mm5
	mov edx, [ebx+20]  ;; mov srcs[5] into edx
	movd mm6, [edx]   ;; mov srcs[5][0..3] into mm6
	mov edx, [ebx+24]  ;; mov srcs[6] into edx
	movd mm7, [edx]   ;; mov srcs[6][0..3] into mm7

	;; add
	paddsw mm1, mm2
	paddsw mm3, mm4

	paddsw mm5, mm6

	paddsw mm1, mm3 ;; mm1 = (orig) mm1 + mm2 + mm3 + mm4
	paddsw mm5, mm7 ;; mm5 = (orig) mm5 + mm6 + mm7

	paddsw mm0, mm1 ;; mm0 = mm0 + (orig) mm1 + mm2 + mm3 + mm4
	paddsw mm0, mm5 ;; mm0 = (orig) mm0 + (orig) mm1 + mm2 + mm3 + mm4 +
			;;       (orig) mm5 + mm6 + mm7

	;; write back
	movd [eax], mm0

	;; increment dst
	;; FIXME: we could do an mmx add with a register like:
	;; mmx: 0x0004000400040004 for src[0-3]
	add word [ebx], 4   ;; increment src[0]
	add eax, 4          ;; increment dst
	add word [ebx+4], 4 ;; increment src[1]
	sub ecx, 2          ;; decrement len
	add word [ebx+8], 4 ;; increment src[2]
	add word [ebx+12], 4 ;; increment src[3]
	add word [ebx+16], 4 ;; increment src[4]
	add word [ebx+20], 4 ;; increment src[5]
	add word [ebx+24], 4 ;; increment src[6]
	
	jmp .twocomp

;; now for %1
.rest
	cmp ecx, 0
	jle .done

	mov cx, [eax]
	add cx, word [ebx]
	add cx, word [ebx+4]
	add cx, word [ebx+8]
	add cx, word [ebx+12]
	add cx, word [ebx+16]
	add cx, word [ebx+20]
	add cx, word [ebx+24]

	cmp ecx, 32767       ;; max value
	jg .movemax
	cmp ecx, -32768  ;; min value
	jl .movemin
	mov [eax], cx
.done
	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret

.movemin
	mov word [eax], -32768

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp
	
	emms
	ret

.movemax
	mov word [eax], 32767

	pop edx
	pop ecx
	pop ebx
	pop eax
	mov esp, ebp
	pop ebp

	emms
	ret
