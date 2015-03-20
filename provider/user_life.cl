__kernel void update_cl(__global int *curr, __global int *next) {

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint n = get_global_size(0);

	int neighbours = 0;
	int ox = 0;
	int oy = 0;

	//dx = -1, dy = -1

	ox = (n + x - 1) % n; // handle wrap-around
	oy = (n + y - 1) % n;

	if (curr[oy*n + ox])
		neighbours++;

	//dx = -1, dy = 0
	ox = (n + x - 1) % n; // handle wrap-around
	oy = (n + y) % n;

	if (curr[oy*n + ox])
		neighbours++;

	//dx = -1, dy = 1
	ox = (n + x - 1) % n; // handle wrap-around
	oy = (n + y + 1) % n;

	if (curr[oy*n + ox])
		neighbours++;

	//dx = 0, dy = -1
	ox = (n + x) % n; // handle wrap-around
	oy = (n + y - 1) % n;

	if (curr[oy*n + ox])
		neighbours++;

	//dx = 0, dy = 0
	ox = (n + x) % n; // handle wrap-around
	oy = (n + y) % n;

	//no if loop because !(dx == 0 && dy == 0) = 0

	//dx = 0, dy = 1
	ox = (n + x) % n; // handle wrap-around
	oy = (n + y + 1) % n;

	if (curr[oy*n + ox])
		neighbours++;

	//dx = 1, dy = -1

	ox = (n + x + 1) % n; // handle wrap-around
	oy = (n + y - 1) % n;

	if (curr[oy*n + ox])
		neighbours++;

	//dx = 1, dy = 0
	ox = (n + x + 1) % n; // handle wrap-around
	oy = (n + y) % n;

	if (curr[oy*n + ox])
		neighbours++;

	//dx = 1, dy = 1
	ox = (n + x + 1) % n; // handle wrap-around
	oy = (n + y + 1) % n;

	if (curr[oy*n + ox]) 
		neighbours++;




	//for (int dx = -1; dx <= +1; dx++){
	//	for (int dy = -1; dy <= +1; dy++){
	//		int ox = (n + x + dx) % n; // handle wrap-around
	//		int oy = (n + y + dy) % n;

	//		if (curr.at(oy*n + ox) && !(dx == 0 && dy == 0))
	//		if ((curr[oy*n + ox]==1) && !(dx == 0 && dy == 0))
	//			neighbours++;
	//	}
	//}

	if (curr[n*y + x]==1){
		// alive
		if (neighbours<2){
			next[y*n + x] = 0;
		}
		else if (neighbours>3){
			next[y*n + x] = 0;
		}
		else{
			next[y*n + x] = 1;
		}
	}
	else{
		// dead
		if (neighbours == 3){
			next[y*n + x] = 1;
		}
		else{
			next[y*n + x] = 0;
		}
	}
}