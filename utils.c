#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


int count_pos(char* map[], const int num_of_rows, const int length, char c){
  int counter = 0;
  for(int i=0; i<num_of_rows; i++){
    for(int j=0; j<length; j++)
      if (map[i][j] == c)
        counter++;
  }
  return counter;
}


Pair* create_pos_array(char* map[], const int num_of_rows, const int length, char c, int num_of_pos){
  Pair* positions = (Pair* ) malloc(sizeof(Pair) * num_of_pos);
  
  for(int i=0; i<num_of_rows; i++){
    for(int j=0; j<length; j++)
      if (map[i][j] == c){
        num_of_pos--;
        positions[num_of_pos].y = i;
        positions[num_of_pos].x = j;
      }
  }
  return positions;
}


char** create_world_map(const int num_of_rows, const int length){
  int size = num_of_rows * (sizeof(char *) + sizeof(char) * length);
  char** map = (char** ) malloc(size);

  char* ptr = (char* )(map + num_of_rows);
  for (int i = 0; i < num_of_rows; i++){
    map[i] = ptr; 
    for (int j = 0; j < length; j++)
      map[i][j] = ' ';
    ptr += length;
  }

 return map;
}


void fill_map(char** map, Pair* positions, int size, char symbol){
  for(int i=0; i < size; i++){
    Pair p = positions[i];
    map[p.y][p.x] = symbol;
  }
}


void get_agent_observation(HarvestAgent agent, char** map, 
                           const int num_of_rows, const int length, 
                           char* obs){
  int top = agent.pos.y - 3;
  int bottom = agent.pos.y + 3;

  int local_top = MAX(-top, 0);

  int global_top = MAX(top, 0);
  int global_bottom = MIN(bottom, num_of_rows - 1);

  int left = agent.pos.x - 3;
  int right = agent.pos.x + 3;

  int local_left = MAX(-left, 0);

  int global_left = MAX(left, 0);
  int global_right = MIN(right, length - 1);

  for(;global_top <= global_bottom; global_top++){
    int j = local_left;
    for(int i=global_left; i <= global_right; i++){
      obs[local_top * 7 + j] = map[global_top][i];
      j++;
    }
    local_top++;
  }
}

/*

ACTIONS:
  0 : MOVE_LEFT
  1 : MOVE_DOWN
  2 : MOVE_RIGHT
  3 : MOVE_UP
  (порядок против часовой стрелки)
  
  4 : STAY

  5 : TURN_CLOCKWISE
  6 : TURN_COUNTERCLOCKWISE
  
  7 : FIRE


ORIENTATIONS (порядок против часовой стрелки):
  0 : UP
  1 : LEFT
  2 : DOWN
  3 : RIGHT

*/
Pair get_next_pos(Pair pos, int orientation, int action){
  if (action > 3)
    return pos;
  
  Pair next = pos;
  action = (action + orientation) & 3;
  if (action & 1)
    next.y += (action - 2);
  else
    next.x += (action - 1);

  return next;
}


