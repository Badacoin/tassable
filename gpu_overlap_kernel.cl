__kernel void gpu_overlap (__global int *src, __global int *dst)
{
    int y = get_global_id(0) - 2 * get_group_id(0) * BORDER;
    int x = get_global_id(1) - 2 * get_group_id(1) * BORDER;
    int y_local = get_local_id(0) + 1;
    int x_local = get_local_id(1) + 1;
    
    __local int tile[TILE2 + 2 * BORDER + 2][TILE + 2 * BORDER + 2];
    int center = y * DIM + x;
    tile[x_local][y_local] = src[center];
    barrier(CLK_LOCAL_MEM_FENCE);

    int counter = 0;

    while(counter < BORDER) {
	int left = tile[x_local-1][y_local] / 4;
	int middle = tile[x_local][y_local] % 4;
	int right = tile[x_local+1][y_local] / 4;
	int up = tile[x_local][y_local-1] / 4;
	int down = tile[x_local][y_local+1] / 4;
	barrier(CLK_LOCAL_MEM_FENCE);
        tile[x_local][y_local] = left + middle + right + up + down;
	barrier(CLK_LOCAL_MEM_FENCE);
        counter++;	
    }

    int x_shift = x_local - BORDER - 1;
    int y_shift = y_local - BORDER - 1;
    
    if ((x_shift >= 0) && (x_shift < TILE2) &&
	(y_shift >= 0) && (y_shift < TILE)) {
	dst[center] = tile[x_local][y_local];
    }
}
