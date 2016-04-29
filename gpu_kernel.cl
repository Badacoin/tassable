__kernel void gpu (__global int *src, __global int *dst)
{
    int col = get_global_id(0) + 1;
    int row = get_global_id(1) + 1;

    int center = row * DIM + col;
    int left = src[center - 1] / 4;
    int middle = src[center] % 4;
    int right = src[center + 1] / 4;
    int up = src[center - DIM] / 4;
    int down = src[center + DIM] / 4;

    dst[center] = left + middle + right + up + down;
}