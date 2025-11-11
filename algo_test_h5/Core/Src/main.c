#include "main.h"

//	General purpose variables:
#define maze_x_len 3
#define maze_y_len 3
#define intersection_stack_max_len 15
#define bfs_q_max_len 15
#define path_len 15
// destination coordinates...?


struct Cell{
	uint8_t wallLeft;
	uint8_t wallRight;
	uint8_t wallFront;
	uint8_t wallBack;
	uint8_t visited;

	uint8_t bfs_visited;
	int8_t bfs_parent[2];

	int8_t cost;

};

void Cell(struct Cell* cell){
	cell->wallLeft = 0;
	cell->wallRight = 0;
	cell->wallFront = 0;
	cell->wallBack = 0;
	cell->visited = 0;

	cell->bfs_visited = 0;
	cell->cost = 0;
}

struct Maze{
	struct Cell cells[maze_x_len][maze_y_len];

	int8_t curr_cell[2];
	int8_t back_cell[2];
	int8_t front_cell[2];
	int8_t left_cell[2];
	int8_t right_cell[2];

	char orient;

	int8_t intersection_stack[intersection_stack_max_len][2];
	int8_t ISLen;

	uint8_t bfs_q[bfs_q_max_len][2];
	int8_t q_r,q_f;
	int8_t bfs_loc[2];

	int8_t n_ways;
	int8_t n_visited;

	int8_t prev_removed[2];

	int8_t exploration_done;

	int8_t dest[2];

	int8_t path[path_len][2];
	int8_t plen;

};

// Function prototypes
void Cell(struct Cell* cell);
void Maze(struct Maze* maze, int8_t destx, int8_t desty);
void Maze_SIM(struct Maze* maze);
void update_adjacents(struct Maze* maze);
void update_walls(struct Maze* maze, struct Maze* sim_maze);
void move_forward(struct Maze* maze, struct Maze* sim_maze);
void turn_left(struct Maze* maze, struct Maze* sim_maze);
void turn_right(struct Maze* maze, struct Maze* sim_maze);
void enq(struct Maze* maze);
void deq(struct Maze* maze);
void to_prev_intersection(struct Maze* maze, struct Maze* sim_maze);
void DFS_explore(struct Maze* maze, struct Maze* sim_maze);
void flood_fill(struct Maze* maze, struct Maze* sim_maze);

void Maze(struct Maze* maze,int8_t destx,int8_t desty){
    maze->curr_cell[0] = 0;
    maze->curr_cell[1] = 0;

    maze->ISLen = 0;

    maze->q_r = -1;  // Should start at -1, not 0
    maze->q_f = -1;  // Should start at -1, not 0

    maze->n_ways = 0;
    maze->n_visited = 0;

    maze->prev_removed[0] = -1;
    maze->prev_removed[1] = -1;

    maze->exploration_done = 0;

    maze->dest[0] = destx;
    maze->dest[1] = desty;

    maze->orient = 'N';

    //cells constructor:
    for(int i=0;i<maze_x_len;i++){
        for(int j=0;j<maze_y_len;j++){
            Cell(&maze->cells[i][j]);
        }
    }

    // Mark starting cell as visited and initialize adjacent cells
    maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].visited = 1;
    update_adjacents(maze);

    //	Adding starting point to intersection stack
    maze->intersection_stack[maze->ISLen][0] = maze->curr_cell[0];
    maze->intersection_stack[maze->ISLen][1] = maze->curr_cell[1];
    maze->ISLen++;

    maze->plen = 0;
};

//	Simulator Functions
void Maze_SIM(struct Maze* maze){
	maze->cells[0][0].wallLeft = 1;
	maze->cells[0][0].wallFront = 0;
	maze->cells[0][0].wallRight = 1;
	maze->cells[0][0].wallBack = 1;

	maze->cells[0][1].wallLeft = 1;
	maze->cells[0][1].wallFront = 1;
	maze->cells[0][1].wallRight = 0;
	maze->cells[0][1].wallBack = 0;

	maze->cells[0][2].wallLeft = 1;
	maze->cells[0][2].wallFront = 1;
	maze->cells[0][2].wallRight = 0;
	maze->cells[0][2].wallBack = 1;

	maze->cells[1][0].wallLeft = 1;
	maze->cells[1][0].wallFront = 1;
	maze->cells[1][0].wallRight = 1;
	maze->cells[1][0].wallBack = 1;

	maze->cells[1][1].wallLeft = 0;
	maze->cells[1][1].wallFront = 1;
	maze->cells[1][1].wallRight = 0;
	maze->cells[1][1].wallBack = 1;

	maze->cells[1][2].wallLeft = 0;
	maze->cells[1][2].wallFront = 1;
	maze->cells[1][2].wallRight = 0;
	maze->cells[1][2].wallBack = 1;

	maze->cells[2][0].wallLeft = 1;
	maze->cells[2][0].wallFront = 0;
	maze->cells[2][0].wallRight = 1;
	maze->cells[2][0].wallBack = 1;

	maze->cells[2][1].wallLeft = 0;
	maze->cells[2][1].wallFront = 0;
	maze->cells[2][1].wallRight = 1;
	maze->cells[2][1].wallBack = 0;

	maze->cells[2][2].wallLeft = 0;
	maze->cells[2][2].wallFront = 1;
	maze->cells[2][2].wallRight = 1;
	maze->cells[2][2].wallBack = 0;

}


void SIM_move_forward(struct Maze* maze){
	if(maze->orient=='N') maze->curr_cell[1]++;
	else if(maze->orient=='S') maze->curr_cell[1]--;
	else if(maze->orient=='E') maze->curr_cell[0]++;
	else if(maze->orient=='W') maze->curr_cell[0]--;
}

void SIM_turn_right(struct Maze* maze){
	if(maze->orient=='N') maze->orient='E';
	else if(maze->orient=='E') maze->orient='S';
	else if(maze->orient=='S') maze->orient='W';
	else if(maze->orient=='W') maze->orient='N';
}

void SIM_turn_left(struct Maze* maze){
	if(maze->orient=='N') maze->orient='W';
	else if(maze->orient=='E') maze->orient='N';
	else if(maze->orient=='S') maze->orient='E';
	else if(maze->orient=='W') maze->orient='S';
}

uint8_t SIM_wall_left(struct Maze* maze){
	if(maze->orient=='N' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallLeft==1) return 1;
	else if(maze->orient=='E' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallFront==1) return 1;
	else if(maze->orient=='S' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallRight==1) return 1;
	else if(maze->orient=='W' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallBack==1) return 1;
	return 0;
}
uint8_t SIM_wall_right(struct Maze* maze){
	if(maze->orient=='N' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallRight==1) return 1;
	else if(maze->orient=='E' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallBack==1) return 1;
	else if(maze->orient=='S' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallLeft==1) return 1;
	else if(maze->orient=='W' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallFront==1) return 1;
	return 0;
}
uint8_t SIM_wall_front(struct Maze* maze){
	if(maze->orient=='N' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallFront==1) return 1;
	else if(maze->orient=='E' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallRight==1) return 1;
	else if(maze->orient=='S' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallBack==1) return 1;
	else if(maze->orient=='W' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallLeft==1) return 1;
	return 0;
}
uint8_t SIM_wall_back(struct Maze* maze){
	if(maze->orient=='N' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallBack==1) return 1;
	else if(maze->orient=='E' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallLeft==1) return 1;
	else if(maze->orient=='S' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallFront==1) return 1;
	else if(maze->orient=='W' && maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallRight==1) return 1;
	return 0;
}


//	Algorithm functions:
void update_walls(struct Maze* maze,struct Maze* sim_maze){
    if (maze->orient == 'N')
    {
        // multiple sensors can report walls; check each independently
        if (SIM_wall_left(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallLeft = 1;
            // symmetric update: neighbor to the west has right wall
            if (maze->curr_cell[0] > 0)
                maze->cells[maze->curr_cell[0] - 1][maze->curr_cell[1]].wallRight = 1;
        }
        if (SIM_wall_right(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallRight = 1;
            // symmetric update: neighbor to the east has left wall
            if (maze->curr_cell[0] < maze_x_len-1)
                maze->cells[maze->curr_cell[0] + 1][maze->curr_cell[1]].wallLeft = 1;
        }
        if (SIM_wall_front(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallFront = 1;
            // symmetric update: neighbor to the north has back wall
            if (maze->curr_cell[1] < maze_y_len-1)
                maze->cells[maze->curr_cell[0]][maze->curr_cell[1] + 1].wallBack = 1;
        }
        if (SIM_wall_back(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallBack = 1;
            // symmetric update: neighbor to the south has front wall
            if (maze->curr_cell[1] > 0)
                maze->cells[maze->curr_cell[0]][maze->curr_cell[1] - 1].wallFront = 1;
        }
    }
    else if (maze->orient == 'E')
    {
        if (SIM_wall_left(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallFront = 1;
            if (maze->curr_cell[1] < maze_y_len-1)
                maze->cells[maze->curr_cell[0]][maze->curr_cell[1] + 1].wallBack = 1;
        }
        if (SIM_wall_right(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallBack = 1;
            if (maze->curr_cell[1] > 0)
                maze->cells[maze->curr_cell[0]][maze->curr_cell[1] - 1].wallFront = 1;
        }
        if (SIM_wall_front(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallRight = 1;
            if (maze->curr_cell[0] < maze_x_len-1)
                maze->cells[maze->curr_cell[0] + 1][maze->curr_cell[1]].wallLeft = 1;
        }
        if (SIM_wall_back(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallLeft = 1;
            if (maze->curr_cell[0] > 0)
                maze->cells[maze->curr_cell[0] - 1][maze->curr_cell[1]].wallRight = 1;
        }
    }
    else if (maze->orient == 'S')
    {
        if (SIM_wall_left(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallRight = 1;
            if (maze->curr_cell[0] < maze_x_len-1)
                maze->cells[maze->curr_cell[0] + 1][maze->curr_cell[1]].wallLeft = 1;
        }
        if (SIM_wall_right(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallLeft = 1;
            if (maze->curr_cell[0] > 0)
                maze->cells[maze->curr_cell[0] - 1][maze->curr_cell[1]].wallRight = 1;
        }
        if (SIM_wall_front(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallBack = 1;
            if (maze->curr_cell[1] > 0)
                maze->cells[maze->curr_cell[0]][maze->curr_cell[1] - 1].wallFront = 1;
        }
        if (SIM_wall_back(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallFront = 1;
            if (maze->curr_cell[1] < maze_y_len-1)
                maze->cells[maze->curr_cell[0]][maze->curr_cell[1] + 1].wallBack = 1;
        }
    }
    else if (maze->orient == 'W')
    {
        if (SIM_wall_left(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallBack = 1;
            if (maze->curr_cell[1] > 0)
                maze->cells[maze->curr_cell[0]][maze->curr_cell[1] - 1].wallFront = 1;
        }
        if (SIM_wall_right(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallFront = 1;
            if (maze->curr_cell[1] < maze_y_len - 1)
                maze->cells[maze->curr_cell[0]][maze->curr_cell[1] + 1].wallBack = 1;
        }
        if (SIM_wall_front(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallLeft = 1;
            if (maze->curr_cell[0] > 0)
                maze->cells[maze->curr_cell[0] - 1][maze->curr_cell[1]].wallRight = 1;
        }
        if (SIM_wall_back(sim_maze)==1)
        {
            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].wallRight = 1;
            if (maze->curr_cell[0] < maze_x_len - 1)
                maze->cells[maze->curr_cell[0] + 1][maze->curr_cell[1]].wallLeft = 1;
        }
    }
    else
    {
        // //log("Error updating walls!");
    }
}

void update_adjacents(struct Maze* maze){
    if (maze->orient == 'N')
    {
        maze->left_cell[0] = maze->curr_cell[0] - 1;
        maze->left_cell[1] = maze->curr_cell[1];

        maze->right_cell[0] = maze->curr_cell[0] + 1;
        maze->right_cell[1] = maze->curr_cell[1];

        maze->front_cell[0] = maze->curr_cell[0];
        maze->front_cell[1] = maze->curr_cell[1] + 1;

        maze->back_cell[0] = maze->curr_cell[0];
        maze->back_cell[1] = maze->curr_cell[1] - 1;
    }
    else if (maze->orient == 'E')
    {
        maze->left_cell[0] = maze->curr_cell[0];
        maze->left_cell[1] = maze->curr_cell[1] + 1;

        maze->right_cell[0] = maze->curr_cell[0];
        maze->right_cell[1] = maze->curr_cell[1] - 1;

        maze->front_cell[0] = maze->curr_cell[0] + 1;
        maze->front_cell[1] = maze->curr_cell[1];

        maze->back_cell[0] = maze->curr_cell[0] - 1;
        maze->back_cell[1] = maze->curr_cell[1];
    }
    else if (maze->orient == 'S')
    {
        maze->left_cell[0] = maze->curr_cell[0] + 1;
        maze->left_cell[1] = maze->curr_cell[1];

        maze->right_cell[0] = maze->curr_cell[0] - 1;
        maze->right_cell[1] = maze->curr_cell[1];

        maze->front_cell[0] = maze->curr_cell[0];
        maze->front_cell[1] = maze->curr_cell[1] - 1;

        maze->back_cell[0] = maze->curr_cell[0];
        maze->back_cell[1] = maze->curr_cell[1] + 1;
    }
    else if (maze->orient == 'W')
    {
        maze->left_cell[0] = maze->curr_cell[0];
        maze->left_cell[1] = maze->curr_cell[1] - 1;

        maze->right_cell[0] = maze->curr_cell[0];
        maze->right_cell[1] = maze->curr_cell[1] + 1;

        maze->front_cell[0] = maze->curr_cell[0] - 1;
        maze->front_cell[1] = maze->curr_cell[1];

        maze->back_cell[0] = maze->curr_cell[0] + 1;
        maze->back_cell[1] = maze->curr_cell[1];
    }
    else
    {
        // //log("Error updating adjacent cells!");
    }
}

void move_forward(struct Maze* maze,struct Maze* sim_maze){
    SIM_move_forward(sim_maze);
    // Change virtual position first, then update adjacent cells
    if (maze->orient == 'N')
    {
        maze->curr_cell[1] = maze->curr_cell[1] + 1;
    }
    else if (maze->orient == 'E')
    {
        maze->curr_cell[0] = maze->curr_cell[0] + 1;
    }
    else if (maze->orient == 'S')
    {
        maze->curr_cell[1] = maze->curr_cell[1] - 1;
    }
    else if (maze->orient == 'W')
    {
        maze->curr_cell[0] = maze->curr_cell[0] - 1;
    }
    else
    {
        //log("Error changing position");
    }

    update_adjacents(maze);
}

void turn_left(struct Maze* maze, struct Maze* sim_maze){
	SIM_turn_left(sim_maze);
	    // Change orient first, then update adjacent cells
	    if (maze->orient == 'N')
	    {
	        maze->orient = 'W';
	    }
	    else if (maze->orient == 'E')
	    {
	        maze->orient = 'N';
	    }
	    else if (maze->orient == 'S')
	    {
	        maze->orient = 'E';
	    }
	    else if (maze->orient == 'W')
	    {
	        maze->orient = 'S';
	    }
	    else
	    {
	        // //log("Error turning left!");
	    }

	    update_adjacents(maze);
}

void turn_right(struct Maze *maze,struct Maze* sim_maze)
{
    SIM_turn_right(sim_maze);
    // Change orient first, then update adjacent cells
    if (maze->orient == 'N')
    {
        maze->orient = 'E';
    }
    else if (maze->orient == 'E')
    {
        maze->orient = 'S';
    }
    else if (maze->orient == 'S')
    {
        maze->orient = 'W';
    }
    else if (maze->orient == 'W')
    {
        maze->orient = 'N';
    }
    else
    {
        // //log("Error turning right!");
    }

    update_adjacents(maze);
}

//	Queue Functions
void enq(struct Maze *maze)
{
    if (maze->q_f == -1)
        maze->q_f = 0;
    if (maze->q_r < bfs_q_max_len - 1) {
        maze->q_r++;
        maze->bfs_q[maze->q_r][0] = maze->bfs_loc[0];
        maze->bfs_q[maze->q_r][1] = maze->bfs_loc[1];
    }
    // //log("Added to queue: "+std::to_string(bfs_loc[0])+","+std::to_string(bfs_loc[1]));
}

void deq(struct Maze *maze)
{
    if (maze->q_f <= maze->q_r && maze->q_f >= 0) {
        maze->bfs_loc[0] = maze->bfs_q[maze->q_f][0];
        maze->bfs_loc[1] = maze->bfs_q[maze->q_f][1];
        maze->q_f++;
    }
}

void to_prev_intersection(struct Maze *maze,struct Maze *sim_maze)
{

    for (uint8_t i = 0; i < maze_x_len; i++)
        for (uint8_t j = 0; j < maze_y_len; j++)
        {
            maze->cells[i][j].bfs_visited = 0;
            maze->cells[i][j].bfs_parent[0] = -1;
            maze->cells[i][j].bfs_parent[1] = -1;
        }

    maze->q_f = -1;
    maze->q_r = -1;

    // If the top of intersection stack is the current cell, pop it to go to the previous one
    if (maze->ISLen <= 0)
    {
        maze->exploration_done = 1;
        return;
    }
    if (maze->intersection_stack[maze->ISLen - 1][0] == maze->curr_cell[0] &&
        maze->intersection_stack[maze->ISLen - 1][1] == maze->curr_cell[1])
    {
        // pop current intersection since we're already there
        maze->ISLen = maze->ISLen - 1;
    }
    if (maze->ISLen <= 0)
    {
        //log("No previous intersection to go to");
        maze->exploration_done = 1;
        return;
    }

    // start BFS from current cell
    maze->bfs_loc[0] = maze->curr_cell[0];
    maze->bfs_loc[1] = maze->curr_cell[1];

    // mark start
    maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].bfs_visited = 1;
    maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].bfs_parent[0] = -1;
    maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].bfs_parent[1] = -1;

    enq(maze);

    while (maze->q_r - maze->q_f + 1 > 0)
    {
        deq(maze);

        if (maze->bfs_loc[0] == maze->intersection_stack[maze->ISLen - 1][0] &&
            maze->bfs_loc[1] == maze->intersection_stack[maze->ISLen - 1][1])
        {
            // build path from current cell to intersection using bfs_parent
            int8_t path[path_len][2];
            int8_t plen = 0;

            // start from target (intersection) and walk parents back to start
            int8_t tx = maze->intersection_stack[maze->ISLen - 1][0];
            int8_t ty = maze->intersection_stack[maze->ISLen - 1][1];
            int8_t aborted = 0;
            while (!(tx == maze->curr_cell[0] && ty == maze->curr_cell[1]))
            {
                path[plen][0] = tx;
                path[plen][1] = ty;
                plen++;
                int8_t px = maze->cells[tx][ty].bfs_parent[0];
                int8_t py = maze->cells[tx][ty].bfs_parent[1];
                // safety: if parent not set, abort
                if (px == -1 && py == -1)
                {
                    //log("bfs parent missing, aborting");
                    aborted = 1;
                    break;
                }
                tx = px;
                ty = py;
                // avoid overflow
                if (plen >= path_len)
                {
                    aborted = 1;
                    break;
                }
            }

            if (aborted)
            {
                // cleanup and return without moving
                for (int i = 0; i < maze_x_len; i++)
                    for (int j = 0; j < maze_y_len; j++)
                        maze->cells[i][j].bfs_visited = 0;
                maze->q_f = -1;
                maze->q_r = -1;
                return;
            }

            // finally add the starting cell
            path[plen][0] = maze->curr_cell[0];
            path[plen][1] = maze->curr_cell[1];
            plen++;

            // path currently: [target, ..., start] -> reverse to [start, ..., target]
            for (int8_t i = 0; i < plen / 2; i++)
            {
                int tx0 = path[i][0];
                int ty0 = path[i][1];
                path[i][0] = path[plen - 1 - i][0];
                path[i][1] = path[plen - 1 - i][1];
                path[plen - 1 - i][0] = tx0;
                path[plen - 1 - i][1] = ty0;
            }

            // follow path from current -> intersection
            for (int8_t i = 1; i < plen; i++)
            {
                int8_t sx = path[i - 1][0];
                int8_t sy = path[i - 1][1];
                int8_t dx = path[i][0];
                int8_t dy = path[i][1];

                // determine required absolute direction
                char need = 'N';
                if (dx == sx && dy == sy + 1)
                    need = 'N';
                else if (dx == sx + 1 && dy == sy)
                    need = 'E';
                else if (dx == sx && dy == sy - 1)
                    need = 'S';
                else if (dx == sx - 1 && dy == sy)
                    need = 'W';

                // verify that planned step is not blocked by a wall in the map
                int8_t blocked = 0;
                if (need == 'N' && maze->cells[sx][sy].wallFront)
                    blocked = 1;
                if (need == 'E' && maze->cells[sx][sy].wallRight)
                    blocked = 1;
                if (need == 'S' && maze->cells[sx][sy].wallBack)
                    blocked = 1;
                if (need == 'W' && maze->cells[sx][sy].wallLeft)
                    blocked = 1;
                if (blocked==1)
                {
                    // cleanup and abort
                    for (int8_t ii = 0; ii < maze_x_len; ii++)
                        for (int8_t jj = 0; jj < maze_y_len; jj++)
                            maze->cells[ii][jj].bfs_visited = 0;
                    maze->q_f = -1;
                    maze->q_r = -1;
                    return;
                }

                // turn to required direction using minimal turns
                if (maze->orient == need)
                {
                    // nothing
                }
                else if ((maze->orient == 'N' && need == 'S') || (maze->orient == 'S' && need == 'N') ||
                         (maze->orient == 'E' && need == 'W') || (maze->orient == 'W' && need == 'E'))
                {
                    turn_left(maze,sim_maze);
                    turn_left(maze,sim_maze);
                }
                else if ((maze->orient == 'N' && need == 'W') || (maze->orient == 'W' && need == 'S') ||
                         (maze->orient == 'S' && need == 'E') || (maze->orient == 'E' && need == 'N'))
                {
                    turn_left(maze,sim_maze);
                }
                else
                {
                    turn_right(maze,sim_maze);
                }

                // move forward one step
                move_forward(maze,sim_maze);
            }

            // cleanup bfs_visited flags and queue pointers
            for (int8_t i = 0; i < maze_x_len; i++)
                for (int8_t j = 0; j < maze_y_len; j++)
                    maze->cells[i][j].bfs_visited = 0;

            maze->q_f = -1;
            maze->q_r = -1;

            // pop this intersection from stack
            maze->n_ways = 0;
            maze->n_visited = 0;
            if (SIM_wall_front(sim_maze)==0)
            {
                maze->n_ways++;
                if (maze->front_cell[0] >= 0 && maze->front_cell[0] < maze_x_len &&
                    maze->front_cell[1] >= 0 && maze->front_cell[1] < maze_y_len &&
                    maze->cells[maze->front_cell[0]][maze->front_cell[1]].visited)
                    maze->n_visited++;
            }
            if (SIM_wall_left(sim_maze)==0)
            {
                maze->n_ways++;
                if (maze->left_cell[0] >= 0 && maze->left_cell[0] < maze_x_len &&
                    maze->left_cell[1] >= 0 && maze->left_cell[1] < maze_y_len &&
                    maze->cells[maze->left_cell[0]][maze->left_cell[1]].visited)
                    maze->n_visited++;
            }
            if (SIM_wall_right(sim_maze)==0)
            {
                maze->n_ways++;
                if (maze->right_cell[0] >= 0 && maze->right_cell[0] < maze_x_len &&
                    maze->right_cell[1] >= 0 && maze->right_cell[1] < maze_y_len &&
                    maze->cells[maze->right_cell[0]][maze->right_cell[1]].visited)
                    maze->n_visited++;
            }
            if (maze->n_visited == maze->n_ways)
            {
                maze->prev_removed[0] = maze->intersection_stack[maze->ISLen - 1][0];
                maze->prev_removed[1] = maze->intersection_stack[maze->ISLen - 1][1];
                maze->ISLen--;
            }
            return;
        }

        if (maze->bfs_loc[0] > 0 && !maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].wallLeft &&
            !maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].bfs_visited &&
            maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].visited)
        {
            maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].bfs_visited = 1;
            maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].bfs_parent[0] = maze->bfs_loc[0];
            maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].bfs_parent[1] = maze->bfs_loc[1];
            maze->bfs_loc[0]--;
            enq(maze);
            maze->bfs_loc[0]++;
        }
        if (maze->bfs_loc[1] < maze_y_len-1 && !maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].wallFront &&
            !maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].bfs_visited &&
            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].visited)
        {
            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].bfs_visited = 1;
            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].bfs_parent[0] = maze->bfs_loc[0];
            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].bfs_parent[1] = maze->bfs_loc[1];
            maze->bfs_loc[1]++;
            enq(maze);
            maze->bfs_loc[1]--;
        }
        if (maze->bfs_loc[0] < maze_x_len-1 && !maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].wallRight &&
            !maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].bfs_visited &&
            maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].visited)
        {
            maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].bfs_visited = 1;
            maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].bfs_parent[0] = maze->bfs_loc[0];
            maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].bfs_parent[1] = maze->bfs_loc[1];
            maze->bfs_loc[0]++;
            enq(maze);
            maze->bfs_loc[0]--;
        }
        if (maze->bfs_loc[1] > 0 && !maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].wallBack &&
            !maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].bfs_visited &&
            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].visited)
        {
            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].bfs_visited = 1;
            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].bfs_parent[0] = maze->bfs_loc[0];
            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].bfs_parent[1] = maze->bfs_loc[1];
            maze->bfs_loc[1]--;
            enq(maze);
            maze->bfs_loc[1]++;
        }
    }
}

void DFS_explore(struct Maze* maze, struct Maze* sim_maze){
	while (!maze->exploration_done)
	    {
	        if (!maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].visited)
	        {
	            maze->cells[maze->curr_cell[0]][maze->curr_cell[1]].visited = 1;
	        }
	        update_adjacents(maze);
	        update_walls(maze,sim_maze);

	        //  Move ahead if no other route:
	        if (SIM_wall_front(sim_maze)==0 && SIM_wall_left(sim_maze)==1 && SIM_wall_right(sim_maze)==1)
	        {
	            move_forward(maze,sim_maze);
	        }
	        else if (SIM_wall_left(sim_maze)==1 && SIM_wall_front(sim_maze)==1 && SIM_wall_right(sim_maze)==0)
	        {
	            turn_right(maze,sim_maze);
	            move_forward(maze,sim_maze);
	        }
	        else if (SIM_wall_left(sim_maze)==0 && SIM_wall_front(sim_maze)==1 && SIM_wall_right(sim_maze)==1)
	        {
	            turn_left(maze,sim_maze);
	            move_forward(maze,sim_maze);
	        }
	        //  Dead End
	        else if (SIM_wall_front(sim_maze)==1 && SIM_wall_left(sim_maze)==1 && SIM_wall_right(sim_maze)==1)
	        {
	            // Go back to previous intersection
	            to_prev_intersection(maze,sim_maze);
	        }
	        //  Intersection found
	        else
	        {
	            if (maze->ISLen > 0 && maze->ISLen < intersection_stack_max_len && // Add bounds check
	                (!(maze->intersection_stack[maze->ISLen - 1][0] == maze->curr_cell[0] &&
	                   maze->intersection_stack[maze->ISLen - 1][1] == maze->curr_cell[1])) &&
	                (!(maze->curr_cell[0] == maze->prev_removed[0] && maze->curr_cell[1] == maze->prev_removed[1])))
	            {
	                maze->intersection_stack[maze->ISLen][0] = maze->curr_cell[0];
	                maze->intersection_stack[maze->ISLen][1] = maze->curr_cell[1];
	                maze->ISLen++;
	            }

	            if (SIM_wall_left(sim_maze)==0 && maze->left_cell[0] >= 0 && maze->left_cell[0] < maze_x_len &&
	                maze->left_cell[1] >= 0 && maze->left_cell[1] < maze_y_len &&
	                maze->cells[maze->left_cell[0]][maze->left_cell[1]].visited==0)
	            {
	                turn_left(maze,sim_maze);
	                move_forward(maze,sim_maze);
	                continue;
	            }
	            else if (SIM_wall_front(sim_maze)==0 && maze->front_cell[0] >= 0 && maze->front_cell[0] < maze_x_len &&
	                     maze->front_cell[1] >= 0 && maze->front_cell[1] < maze_y_len &&
	                     maze->cells[maze->front_cell[0]][maze->front_cell[1]].visited==0)
	            {
	                move_forward(maze,sim_maze);
	                continue;
	            }
	            else if (SIM_wall_right(sim_maze)==0 && maze->right_cell[0] >= 0 && maze->right_cell[0] < maze_x_len &&
	                     maze->right_cell[1] >= 0 && maze->right_cell[1] < maze_y_len &&
	                     maze->cells[maze->right_cell[0]][maze->right_cell[1]].visited==0)
	            {
	                turn_right(maze,sim_maze);
	                move_forward(maze,sim_maze);
	                continue;
	            }
	            else
	            {
	                // Go back to previous intersection
	                to_prev_intersection(maze,sim_maze);
	            }
	        }
	    }
}

void flood_fill(struct Maze* maze, struct Maze* sim_maze){
	   // prepare BFS: clear previous bfs flags/parents and reset queue
	    for (int8_t i = 0; i < maze_x_len; i++)
	        for (int8_t j = 0; j < maze_y_len; j++)
	        {
	            maze->cells[i][j].bfs_visited = 0;
	        }

	    maze->q_f = -1;
	    maze->q_r = -1;

	    maze->bfs_loc[0] = maze->dest[0];
	    maze->bfs_loc[1] = maze->dest[1];

	    maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].bfs_visited = 1;
	    maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].cost = 0;

	    enq(maze);

	    while (maze->q_r - maze->q_f + 1 > 0)
	    {
	        deq(maze);

	        if (maze->bfs_loc[0] > 0 && maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].wallLeft==0 &&
	            maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].bfs_visited==0 &&
	            maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].visited==1)
	        {
	            maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].bfs_visited = 1;
	            maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].bfs_parent[0] = maze->bfs_loc[0];
	            maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].bfs_parent[1] = maze->bfs_loc[1];
	            maze->cells[maze->bfs_loc[0] - 1][maze->bfs_loc[1]].cost = maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].cost + 1;
	            maze->bfs_loc[0]--;

	            enq(maze);
	            maze->bfs_loc[0]++;
	        }
	        if (maze->bfs_loc[1] < maze_y_len-1 && maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].wallFront==0 &&
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].bfs_visited==0 &&
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].visited==1)
	        {
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].bfs_visited = 1;
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].bfs_parent[0] = maze->bfs_loc[0];
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].bfs_parent[1] = maze->bfs_loc[1];
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] + 1].cost = maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].cost + 1;
	            maze->bfs_loc[1]++;

	            enq(maze);
	            maze->bfs_loc[1]--;
	        }
	        if (maze->bfs_loc[0] < maze_x_len-1 && maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].wallRight==0 &&
	            maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].bfs_visited==0 &&
	            maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].visited==1)
	        {
	            maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].bfs_visited = 1;
	            maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].bfs_parent[0] = maze->bfs_loc[0];
	            maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].bfs_parent[1] = maze->bfs_loc[1];
	            maze->cells[maze->bfs_loc[0] + 1][maze->bfs_loc[1]].cost = maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].cost + 1;
	            maze->bfs_loc[0]++;

	            enq(maze);
	            maze->bfs_loc[0]--;
	        }
	        if (maze->bfs_loc[1] > 0 && maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].wallBack==0 &&
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].bfs_visited==0 &&
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].visited==1)
	        {
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].bfs_visited = 1;
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].bfs_parent[0] = maze->bfs_loc[0];
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].bfs_parent[1] = maze->bfs_loc[1];
	            maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1] - 1].cost = maze->cells[maze->bfs_loc[0]][maze->bfs_loc[1]].cost + 1;
	            maze->bfs_loc[1]--;

	            enq(maze);
	            maze->bfs_loc[1]++;
	        }
	    }

	    //  Create the shortest path
	    // log("Starting pathfinding from " + std::to_string(curr_cell[0]) + "," + std::to_string(curr_cell[1]) + " to " + std::to_string(dest[0]) + "," + std::to_string(dest[1]));

	    // Build path by following lowest cost neighbors

	    // Start from current position
	    int8_t tx = maze->curr_cell[0];
	    int8_t ty = maze->curr_cell[1];

	    // Add current cell to path
	    maze->path[maze->plen][0] = tx;
	    maze->path[maze->plen][1] = ty;
	    maze->plen++;

	    // Follow path of decreasing costs until we reach destination
	    while (!(tx == maze->dest[0] && ty == maze->dest[1]))
	    {
	        int min_cost = 999;
	        int8_t next_x = tx;
	        int8_t next_y = ty;
	        int8_t found_next = 0;

	        // Check all four neighbors for the one with minimum cost
	        // Left neighbor
	        if (tx > 0 && maze->cells[tx][ty].wallLeft==0 && maze->cells[tx - 1][ty].visited==1 && maze->cells[tx - 1][ty].cost < min_cost)
	        {
	            min_cost = maze->cells[tx - 1][ty].cost;
	            next_x = tx - 1;
	            next_y = ty;
	            found_next = 1;
	        }
	        // Front neighbor (North)
	        if (ty < maze_y_len-1 && maze->cells[tx][ty].wallFront==0 && maze->cells[tx][ty + 1].visited==1 && maze->cells[tx][ty + 1].cost < min_cost)
	        {
	            min_cost = maze->cells[tx][ty + 1].cost;
	            next_x = tx;
	            next_y = ty + 1;
	            found_next = 1;
	        }
	        // Right neighbor
	        if (tx < maze_x_len-1 && maze->cells[tx][ty].wallRight==0 && maze->cells[tx + 1][ty].visited==1 && maze->cells[tx + 1][ty].cost < min_cost)
	        {
	            min_cost = maze->cells[tx + 1][ty].cost;
	            next_x = tx + 1;
	            next_y = ty;
	            found_next = 1;
	        }
	        // Back neighbor (South)
	        if (ty > 0 && maze->cells[tx][ty].wallBack==0 && maze->cells[tx][ty - 1].visited==1 && maze->cells[tx][ty - 1].cost < min_cost)
	        {
	            min_cost = maze->cells[tx][ty - 1].cost;
	            next_x = tx;
	            next_y = ty - 1;
	            found_next = 1;
	        }

	        if (found_next==0)
	        {
	            // log("No path found to destination!");
	            return;
	        }

	        // Move to next cell
	        tx = next_x;
	        ty = next_y;
	        maze->path[maze->plen][0] = tx;
	        maze->path[maze->plen][1] = ty;
	        maze->plen++;

	        // Safety check
	        if (maze->plen >= path_len)
	        {
	            // log("Path too long, aborting");
	            return;
	        }
	    }

	    // Execute the path using same logic as to_prev_intersection
//        for (int8_t i = 1; i < plen; i++)
//        {
//            int8_t sx = path[i - 1][0];
//            int8_t sy = path[i - 1][1];
//            int8_t dx = path[i][0];
//            int8_t dy = path[i][1];
//
//            // determine required absolute direction
//            char need = 'N';
//            if (dx == sx && dy == sy + 1)
//                need = 'N';
//            else if (dx == sx + 1 && dy == sy)
//                need = 'E';
//            else if (dx == sx && dy == sy - 1)
//                need = 'S';
//            else if (dx == sx - 1 && dy == sy)
//                need = 'W';
//
//            // verify that planned step is not blocked by a wall in the map
//            int8_t blocked = 0;
//            if (need == 'N' && maze->cells[sx][sy].wallFront)
//                blocked = 1;
//            if (need == 'E' && maze->cells[sx][sy].wallRight)
//                blocked = 1;
//            if (need == 'S' && maze->cells[sx][sy].wallBack)
//                blocked = 1;
//            if (need == 'W' && maze->cells[sx][sy].wallLeft)
//                blocked = 1;
//            if (blocked==1)
//            {
//                // cleanup and abort
//                for (int8_t ii = 0; ii < 16; ii++)
//                    for (int8_t jj = 0; jj < 16; jj++)
//                        maze->cells[ii][jj].bfs_visited = 0;
//                maze->q_f = -1;
//                maze->q_r = -1;
//                return;
//            }
//
//            // turn to required direction using minimal turns
//            if (maze->orient == need)
//            {
//                // nothing
//            }
//            else if ((maze->orient == 'N' && need == 'S') || (maze->orient == 'S' && need == 'N') ||
//                     (maze->orient == 'E' && need == 'W') || (maze->orient == 'W' && need == 'E'))
//            {
//                turn_left(maze,sim_maze);
//                turn_left(maze,sim_maze);
//            }
//            else if ((maze->orient == 'N' && need == 'W') || (maze->orient == 'W' && need == 'S') ||
//                     (maze->orient == 'S' && need == 'E') || (maze->orient == 'E' && need == 'N'))
//            {
//                turn_left(maze,sim_maze);
//            }
//            else
//            {
//                turn_right(maze,sim_maze);
//            }
//
//            // move forward one step
//            move_forward(maze,sim_maze);
//        }

}


void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  struct Maze sim_maze;
  Maze(&sim_maze,0,2);
//  Maze_SIM(&sim_maze);
  Maze_SIM(&sim_maze);

  struct Maze maze;
  Maze(&maze,0,2);

  int a = 1;
  DFS_explore(&maze,&sim_maze);
  a++;
  a++;
  flood_fill(&maze,&sim_maze);
  while (1)
  {
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV2;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the programming delay
  */
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
