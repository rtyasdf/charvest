void fire_horizontal(char** map, const int length, 
                     HarvestAgent agent, int** apple_id, 
                     char** dead, int** apples){
  
  int sign = 2 - agent.orientation;
  
  for(int i=0; i < 3; i++){
    int y = agent.pos.y - 1 + i;
    int x = agent.pos.x + sign * (i & 1);

    for(int j=0; j < 5; j++){
      char s = map[y][x];
      
      if (s == '@')
        break;
      else if (s == 'A'){
        **apples = apple_id[y][x];
        (*apples)++;
      }
      else if (s > 96){
        **dead = s - 97;
        (*dead)++; 
      }
      
      map[y][x] = 'F';
      x += sign;
    }
    if (x == 0 || x == length)
      continue;
  }
}


void fire_vertical(char** map, const int num_of_rows, 
                   HarvestAgent agent, int** apple_id, 
                   char** dead, int** apples){
  
  int sign = agent.orientation - 1;
  int y = agent.pos.y;
  for(int i=0; i < 6; i++){
    int x = agent.pos.x - 1;
    for(int j=0; j < 3; j++){
      if ((i == 5 && (j & 1) == 0) || (i == 0 && (j & 1))){
        x++;
        continue;
      }

      char s = map[y][x];
      if (s == '@'){
        x++;
        continue;
      }
      else if (s == 'A'){
        **apples = apple_id[y][x];
        (*apples)++;
      }
      else if (s > 96){
        **dead = s - 97;
        (*dead)++; 
      }
      
      map[y][x] = 'F';
      x++;
    }
    y += sign;
    if (y == 0 || y == num_of_rows)
      break;
  }
}


void clean_horizontal(char** map, const int length, HarvestAgent agent){
  
  int sign = 2 - agent.orientation;
  
  for(int i=0; i < 3; i++){
    int y = agent.pos.y - 1 + i;
    int x = agent.pos.x + sign * (i & 1);

    for(int j=0; j < 5; j++){
      char s = map[y][x];
      
      if (s == '@')
        break;

      map[y][x] = ' ';
      x += sign;
    }
    if (x == 0 || x == length)
      continue;
  }
}


void clean_vertical(char** map, const int num_of_rows, HarvestAgent agent){
  
  int sign = agent.orientation - 1;
  int y = agent.pos.y;
  for(int i=0; i < 6; i++){
    int x = agent.pos.x - 1;
    
    for(int j=0; j < 3; j++){
      if (i == 5 && (j & 1) == 0 || i == 0 && (j & 1)){
        x++;
        continue;
      }

      char s = map[y][x];
      if (s == '@'){
        x++;
        continue;
      }
      
      map[y][x] = ' ';
      x++;
    }
    y += sign;
    if (y == 0 || y == num_of_rows)
      break;
  }
}
