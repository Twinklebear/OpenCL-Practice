__kernel void convolve(const __global uint * const in, __constant uint * const mask,
	__global uint * const out, const int in_dim, const int mask_dim)
{
	const int2 pos = (int2)(get_global_id(0), get_global_id(1));
	uint sum = 0;
	for (int r = 0; r < mask_dim; ++r){
		//Find the location of the top-left corner of the mask in the signal
		const int idx = (pos.y + r) * in_dim + pos.x;
		for (int c = 0; c < mask_dim; ++c){
			sum += mask[r * mask_dim + c] * in[idx + c];
		}
	}
	out[pos.y * get_global_size(0) + pos.x] = sum;
}

