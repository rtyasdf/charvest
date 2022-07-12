#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


int count_pos(char* map[], const int num_of_rows, const int length, char c){
  int counter = 0;
  for(int i=0; i < num_of_rows; i++)
    for(int j=0; j < length; j++)
      if (map[i][j] == c)
        counter++;

  return counter;
}


Pair* create_pos_array(char* map[], const int num_of_rows, const int length, char c, int num_of_pos){
  Pair* positions = (Pair* ) malloc(sizeof(Pair) * num_of_pos);
  
  int idx = 0;
  for(int i=0; i < num_of_rows; i++){
    for(int j=0; j < length; j++)
      if (map[i][j] == c){
        positions[idx].y = i;
        positions[idx].x = j;
        idx++;
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

void get_char_obs_up(int global_top, int global_bottom,
                     int global_left, int global_right,
                     int local_top, int local_left, const int view_size,
                     char** map, char* obs){

  obs += local_left;

  for(;global_top <= global_bottom; global_top++){
    char* ptr = (char* )(obs + view_size * local_top);
    for(int i=global_left; i <= global_right; i++)
      *(ptr++) = map[global_top][i];
    local_top++;
  }
}


void get_char_obs_right(int global_top, int global_bottom, 
                        int global_left, int global_right,
                        int local_top, int local_left, const int view_size,
                        char** map, char* obs){

  obs += view_size * (view_size - local_left - 1);

  for(;global_top <= global_bottom; global_top++){
    char* ptr = (char* )(obs  + local_top);
    for(int i=global_left; i <= global_right; i++){
      *ptr = map[global_top][i];
      ptr -= view_size;
    }
    local_top++;
  }
}


void get_char_obs_down(int global_top, int global_bottom,
                       int global_left, int global_right,
                       int local_top, int local_left, const int view_size,
                       char** map, char* obs){

  obs += view_size * view_size - local_left - 1;

  for(;global_top <= global_bottom; global_top++){
    char* ptr = (char* )(obs - view_size * local_top);
    for(int i=global_left; i <= global_right; i++)
      *(ptr--) = map[global_top][i];
    local_top++;
  }
}


void get_char_obs_left(int global_top, int global_bottom,
                       int global_left, int global_right,
                       int local_top, int local_left, const int view_size,
                       char** map, char* obs){

  obs += view_size * (local_left + 1) - 1;

  for(;global_top <= global_bottom; global_top++){
    char* ptr = (char* )(obs - local_top);
    for(int i=global_left; i <= global_right; i++){
      *ptr = map[global_top][i];
      ptr += view_size;
    }
    local_top++;
  }
}


float char2float(char c){
  switch (c){
    case ' ':
      return 1.f;
    case '@':
      return 2.f;
    case 'F':
      return 3.f;
    case 'A':
      return 4.f;
    default:
      return (c - 97) + 5.f;
  }
}


void get_obs_up(int global_top, int global_bottom,
                int global_left, int global_right,
                int local_top, int local_left, const int view_size,
                char** map, float* obs){

  obs += local_left;

  for(;global_top <= global_bottom; global_top++){
    float* ptr = (float* )(obs + view_size * local_top);
    for(int i=global_left; i <= global_right; i++)
      *(ptr++) = char2float(map[global_top][i]);
    local_top++;
  }
}


void get_obs_right(int global_top, int global_bottom, 
                   int global_left, int global_right,
                   int local_top, int local_left, const int view_size,
                   char** map, float* obs){

  obs += view_size * (view_size - local_left - 1);

  for(;global_top <= global_bottom; global_top++){
    float* ptr = (float* )(obs  + local_top);
    for(int i=global_left; i <= global_right; i++){
      *ptr = char2float(map[global_top][i]);
      ptr -= view_size;
    }
    local_top++;
  }
}


void get_obs_down(int global_top, int global_bottom,
                  int global_left, int global_right,
                  int local_top, int local_left, const int view_size,
                  char** map, float* obs){

  obs += view_size * view_size - local_left - 1;

  for(;global_top <= global_bottom; global_top++){
    float* ptr = (float* )(obs - view_size * local_top);
    for(int i=global_left; i <= global_right; i++)
      *(ptr--) = char2float(map[global_top][i]);
    local_top++;
  }
}


void get_obs_left(int global_top, int global_bottom,
                  int global_left, int global_right,
                  int local_top, int local_left, const int view_size,
                  char** map, float* obs){

  obs += view_size * (local_left + 1) - 1;

  for(;global_top <= global_bottom; global_top++){
    float* ptr = (float* )(obs - local_top);
    for(int i=global_left; i <= global_right; i++){
      *ptr = char2float(map[global_top][i]);
      ptr += view_size;
    }
    local_top++;
  }
}


void get_agent_observation(HarvestAgent agent, char** map, 
                           const int num_of_rows, const int length, const int view_size, 
                           float* obs){
  int top = agent.pos.y - view_size;
  int bottom = agent.pos.y + view_size;

  int local_top = MAX(-top, 0);

  int global_top = MAX(top, 0);
  int global_bottom = MIN(bottom, num_of_rows - 1);

  int left = agent.pos.x - view_size;
  int right = agent.pos.x + view_size;

  int local_left = MAX(-left, 0);

  int global_left = MAX(left, 0);
  int global_right = MIN(right, length - 1);

  switch (agent.orientation){
    case 0:
      get_obs_up(global_top, global_bottom, 
                 global_left, global_right, 
                 local_top, local_left, (view_size << 1) | 1, 
                 map, obs);
      break;
    case 1:
      get_obs_right(global_top, global_bottom, 
                    global_left, global_right, 
                    local_top, local_left, (view_size << 1) | 1, 
                    map, obs);
      break;
    case 2:
      get_obs_down(global_top, global_bottom, 
                   global_left, global_right, 
                   local_top, local_left, (view_size << 1) | 1, 
                   map, obs);
      break;
    case 3:
      get_obs_left(global_top, global_bottom, 
                   global_left, global_right, 
                   local_top, local_left, (view_size << 1) | 1, 
                   map, obs);
      break;
  }
  /*
  for(;global_top <= global_bottom; global_top++){
    char* ptr = (char* )(obs + 7 * local_top + local_left);
    for(int i=global_left; i <= global_right; i++){
      *(ptr++) = map[global_top][i];
    }
    local_top++;
  }
  */
}


void get_global_map(char** map, float* full_map,
                    const int num_of_rows, const int length){

  for(int i=0; i < num_of_rows; i++)
    for(int j=0; j < length; j++)
      *(full_map++) = char2float(map[i][j]);
}
/*

ACTIONS:
  0 : MOVE_LEFT
  1 : MOVE_UP
  2 : MOVE_RIGHT
  3 : MOVE_DOWN
  (порядок по часовой стрелке)

  4 : STAY

  5 : TURN_CLOCKWISE
  6 : TURN_COUNTERCLOCKWISE
  
  7 : FIRE


ORIENTATIONS (порядок по часовой стрелке):
  0 : UP
  1 : RIGHT
  2 : DOWN
  3 : LEFT

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


