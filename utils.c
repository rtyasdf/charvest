typedef struct{
  int x;
  int y;
} Pair;


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
        positions[num_of_pos].x = i;
        positions[num_of_pos].y = j;
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


void build_walls(char* build_map[], const int num_of_rows, const int length, char** world_map, char wall_symbol){
  for(int i=0; i<num_of_rows; i++)
    for(int j=0; j<length; j++)
      if (build_map[i][j] == wall_symbol)
        world_map[i][j] = wall_symbol;
}
