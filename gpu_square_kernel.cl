__kernel void gpu_square (__global int *src, __global int *dst)
{
    int col = get_global_id(0) + 1;
    int row = get_global_id(1) + 1;
    int y = get_local_id(0) + 1;
    int x = get_local_id(1) + 1;
    int center = row * DIM + col;
    int offset1 = (TILE / 2) * (DIM + 1);
    int offset2 = offset1 - TILE;

    __local int tile[2 * TILE + 2][2 * TILE + 2];
    __local int dual_tile[2 * TILE + 2][2 * TILE + 2];

    int x_min = x - TILE / 2;
    int x_plus = x + TILE / 2;
    int y_min = y - TILE / 2;
    int y_plus = y + TILE / 2;
    
    tile[x_min][y_min] = src[center - offset1];
    tile[x_min][y_plus] = src[center - offset2];
    tile[x_plus][y_min] = src[center + offset2];
    tile[x_plus][y_plus] = src[center + offset1];
    barrier(CLK_LOCAL_MEM_FENCE);

    int ***temp;
    int left, middle, right, up, down, x_t, y_t;
	
    for (int i = 0 ; i < TILE / 4 ; i++) {
        
	x_t = x_min;
	y_t = y_min;
	left = tile[x_t][y_t-1] / 4;
	middle = tile[x_t][y_t] % 4;
	right = tile[x_t][y_t+1] / 4;
	up = tile[x_t-1][y_t] / 4;
	down = tile[x_t+1][y_t] / 4;
	dual_tile[x_t][y_t] = left + middle + right + up + down;
        
	y_t = y_plus;
	left = tile[x_t][y_t-1] / 4;
	middle = tile[x_t][y_t] % 4;
	right = tile[x_t][y_t+1] / 4;
	up = tile[x_t-1][y_t] / 4;
	down = tile[x_t+1][y_t] / 4;
	dual_tile[x_t][y_t] = left + middle + right + up + down;

	x_t = x_plus;
	left = tile[x_t][y_t-1] / 4;
	middle = tile[x_t][y_t] % 4;
	right = tile[x_t][y_t+1] / 4;
	up = tile[x_t-1][y_t] / 4;
	down = tile[x_t+1][y_t] / 4;
	dual_tile[x_t][y_t] = left + middle + right + up + down;
    
	y_t = y_min;
	left = tile[x_t][y_t-1] / 4;
	middle = tile[x_t][y_t] % 4;
	right = tile[x_t][y_t+1] / 4;
	up = tile[x_t-1][y_t] / 4;
	down = tile[x_t+1][y_t] / 4;
	dual_tile[x_t][y_t] = left + middle + right + up + down;
	
	barrier(CLK_LOCAL_MEM_FENCE);

	left = dual_tile[x_t][y_t-1] / 4;
	middle = dual_tile[x_t][y_t] % 4;
	right = dual_tile[x_t][y_t+1] / 4;
	up = dual_tile[x_t-1][y_t] / 4;
	down = dual_tile[x_t+1][y_t] / 4;
	tile[x_t][y_t] = left + middle + right + up + down;
        
	x_t = x_min;
	left = dual_tile[x_t][y_t-1] / 4;
	middle = dual_tile[x_t][y_t] % 4;
	right = dual_tile[x_t][y_t+1] / 4;
	up = dual_tile[x_t-1][y_t] / 4;
	down = dual_tile[x_t+1][y_t] / 4;
	tile[x_t][y_t] = left + middle + right + up + down;
    
	y_t = y_plus;
	left = dual_tile[x_t][y_t-1] / 4;
	middle = dual_tile[x_t][y_t] % 4;
	right = dual_tile[x_t][y_t+1] / 4;
	up = dual_tile[x_t-1][y_t] / 4;
	down = dual_tile[x_t+1][y_t] / 4;
	tile[x_t][y_t] = left + middle + right + up + down;
    
	x_t = x_plus;
	left = dual_tile[x_t][y_t-1] / 4;
	middle = dual_tile[x_t][y_t] % 4;
	right = dual_tile[x_t][y_t+1] / 4;
	up = dual_tile[x_t-1][y_t] / 4;
	down = dual_tile[x_t+1][y_t] / 4;
	tile[x_t][y_t] = left + middle + right + up + down;
    
	barrier(CLK_LOCAL_MEM_FENCE);
    }

    dst[center] = tile[x][y];
}
