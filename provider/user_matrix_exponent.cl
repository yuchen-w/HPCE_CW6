__kernel void MatrixMul(__global const uint *a, __global const uint *b, __global uint *res)
{
	uint r = get_global_id(0);
	uint c = get_global_id(1);
	uint n = get_global_size(0);
	uint temp = 0;

	for (unsigned i = 0; i<n; i++){
		temp = (uint)(temp + ((a[r*n + i] * b[i*n + r]) % 2147483647)) % 2147483647;
	}
	res[r*n + c] = temp;
}

