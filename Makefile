simd_torture: simd_torture.c
	gcc simd_torture.c -O2 -mavx512f -o simd_torture
