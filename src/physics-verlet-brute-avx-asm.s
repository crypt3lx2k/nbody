section	.text
extern grav
extern soft
extern half

extern a0x
extern a0y
extern a1x
extern a1y

global physics_advance:function

align 32
physics_advance:
	;; f32 dt (xmm0), i64 n (rdi),
	;; f32* px (rsi), f32* py (rdx),
	;; f32* vx (rcx), f32* vy (r8),
	;; f32* m (r9)	
	push	r12
	push	r13
	push	r14
	push	r15

	test	rdi, rdi
	je	.L5

	mov	r12, [a0x]
	mov	r13, [a0y]
	mov	r14, [a1x]
	mov	r15, [a1y]

	;; __m256 e = _mm256_set1_ps(SOFTENING*SOFTENING);
	vmovss	xmm13, [soft]
	vmulss	xmm13, xmm13, xmm13
	vshufps	xmm13, xmm13, xmm13, 0x0
	vinsertf128	ymm13, ymm13, xmm13, 0x1

	;; __m256 d = _mm256_set1_ps(dt);
	vshufps	xmm14, xmm0, xmm0, 0x0
	vinsertf128	ymm14, ymm14, xmm14, 0x1

	;; __m256 h = _mm256_set1_ps(0.5f*dt);
	vmulss	xmm15, xmm0, [half]
	vshufps	xmm15, xmm15, xmm15, 0x0
	vinsertf128	ymm15, ymm15, xmm15, 0x1

	xor	rax, rax
.L1:
	vmulps	ymm0, ymm15, [r12+rax*4]
	vmulps	ymm1, ymm15, [r13+rax*4]

	vaddps	ymm0, ymm0, [rcx+rax*4]
	vaddps	ymm1, ymm1, [r8+rax*4]

	vmulps	ymm0, ymm0, ymm14
	vmulps	ymm1, ymm1, ymm14

	vaddps	ymm0, ymm0, [rsi+rax*4]
	vaddps	ymm1, ymm1, [rdx+rax*4]

	vmovaps	[rsi+rax*4], ymm0
	vmovaps	[rdx+rax*4], ymm1

	add	rax, 8
	cmp	rax, rdi
	jl	.L1

	xor	r10, r10
.L2:
	;; xi, yi
	vmovaps	ymm0, [rsi+r10*4]
	vmovaps	ymm1, [rdx+r10*4]

	;; axi, ayi
	vxorps	ymm2, ymm2, ymm2
	vxorps	ymm3, ymm3, ymm3

	xor	r11, r11
.L3:
	;; xj, yj, mj
	vbroadcastss	ymm4, [rsi+r11*4]
	vbroadcastss	ymm5, [rdx+r11*4]
	vbroadcastss	ymm12, [r9+r11*4]

	;; rx, ry
	vsubps	ymm4, ymm4, ymm0
	vsubps	ymm5, ymm5, ymm1

	;; s = (rx^2 + ry^2)
	vmulps	ymm6, ymm4, ymm4
	vmulps	ymm7, ymm5, ymm5
	vaddps	ymm11, ymm6, ymm7

	;; s = s+e
	vaddps	ymm11, ymm11, ymm13

	;; s = s*s*s
	vmulps	ymm10, ymm11, ymm11
	vmulps	ymm11, ymm11, ymm10

	;; s = 1/sqrt(s)
	vrsqrtps	ymm11, ymm11

	vmulps	ymm4, ymm4, ymm11
	vmulps	ymm5, ymm5, ymm11

	vmulps	ymm4, ymm4, ymm12
	vmulps	ymm5, ymm5, ymm12

	vaddps	ymm2, ymm2, ymm4
	vaddps	ymm3, ymm3, ymm5

	add	r11, 1
	cmp	r11, rdi
	jl	.L3

	vmovaps	[r14+r10*4], ymm2
	vmovaps	[r15+r10*4], ymm3

	add	r10, 8
	cmp	r10, rdi
	jl	.L2

	xor	rax, rax
.L4:
	vmovaps	ymm0, [r12+rax*4]
	vmovaps	ymm1, [r13+rax*4]

	vaddps	ymm0, ymm0, [r14+rax*4]
	vaddps	ymm1, ymm1, [r15+rax*4]

	vmulps	ymm0, ymm0, ymm15
	vmulps	ymm1, ymm1, ymm15

	vaddps	ymm0, ymm0, [rcx+rax*4]
	vaddps	ymm1, ymm1, [r8+rax*4]

	vmovaps	[rcx+rax*4], ymm0
	vmovaps	[r8+rax*4], ymm1

	add	rax, 8
	cmp	rax, rdi
	jl	.L4

	mov	[a0x], r14
	mov	[a0y], r15
	mov	[a1x], r12
	mov	[a1y], r13
.L5:
	pop	r15
	pop	r14
	pop	r13
	pop	r12

	vzeroupper
	ret
