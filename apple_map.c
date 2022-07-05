typedef struct {
  // HarvestMap heritage
  Pair* pos;
  int size;
  char apple_symbol;

  // For fast respawn
  int** id;
  int** neighbors;
  int* alive;
  
  // Stack
  int* dead;
  int* new_head;
  int* old_head;
} AppleMap;


int** create_id_map(char* build_map[], const int num_of_rows, const int length, char apple_char){
  int size = num_of_rows * (sizeof(int *) + sizeof(int) * length);
  int** id_map = (int** ) malloc(size);

  int count = 0;
  int* ptr = (int* )(id_map + num_of_rows);
  
  for (int i = 0; i < num_of_rows; i++){
    id_map[i] = ptr; 
    for (int j = 0; j < length; j++)
      if (build_map[i][j] == apple_char)
        id_map[i][j] = count++;
      else
        id_map[i][j] = -1;
    ptr += length;
  }

  return id_map;
}


int** create_neighbors(int n_apples, const int R){
  int square = 0;
  for(int i=-R; i <= R; i++)
    for(int j=-R; j <= R; j++)
      if (i * i + j * j <= R * R)
        square++;

  int size = n_apples * (sizeof(int *) + sizeof(int) * (square + 1));
  int** neighbors = (int** ) malloc(size);

  int* ptr = (int* )(neighbors + n_apples);
  
  for (int i = 0; i < n_apples; i++){
    neighbors[i] = ptr;
    ptr += (square + 1);
  }

  return neighbors;
}


void fill_neighbors(char* map[], int** neighbors, int** id, 
                    const int num_of_rows, const int length, 
                    const int R, char c){
  int count = 0;
  for(int i=0; i < num_of_rows; i++)
    for(int j=0; j < length; j++){
      if (map[i][j] != c)
        continue;

      int size = 0;
      for(int u=-R; u <= R; u++){
        int y = i + u;
        if (y < 0 || y >= num_of_rows)
          continue;

        for(int v=-R; v <= R; v++){
          int x = j + v;
          if (x < 0 || x >= length || (v == 0 && u == 0))
            continue; 

          if (u * u + v * v <= R * R && map[y][x] == 'A')
            neighbors[count][++size] = id[y][x];
        }
      }
      neighbors[count++][0] = size;
    }
}


int probs[] = {0, 327, 1310, 3276};  // будет работать только с 0xffff

int choose(int n_alive){
  int p = random() & 0xffff;
  return p < probs[MIN(n_alive, 3)] ? 1 : 0;
}


void swap(int* p, int* q){
  int temp = *p;
  *p = *q;
  *q = temp;
}


void kill_apple(AppleMap* a_map, int y, int x){
  int index = (a_map -> id)[y][x];

  // кладём на стэк
  *(a_map -> new_head) = index;
  (a_map -> new_head)++; 
}


void spawn_apples(char** map, AppleMap* a_map){
  // 0) для краткости
  int* alive = a_map -> alive;
  int** neighbors = a_map -> neighbors;
  
  // 1) сообщаем соседям только что съеденных яблоках о смерти
  int* head = a_map -> old_head;
  for(; head != a_map -> new_head; head++)
    for(int i=1; i <= neighbors[*head][0]; i++){
      int id = neighbors[*head][i];  // индекс соседа съеденного яблока
      alive[id]--;
    }

  // 2) проходим по стэку и кидаем монетку 
  // [также проверяем, чтобы на воскрешаемой ячейке никого из агентов не было]
  int* sep = head;
  int* ptr = a_map -> dead;
  while (ptr != sep){
    if (choose(a_map -> alive[*ptr])){
      Pair p = (a_map -> pos)[*ptr];
      if (map[p.y][p.x] == ' '){
        swap(ptr, --sep);
        continue;
      }
    }
    ptr++;
  }

  // 3) выставляем яблоки на карту
  while (head != sep){
    head--;
    Pair p = (a_map -> pos)[*head];
    map[p.y][p.x] = a_map -> apple_symbol;
    
    // обновляем информацию об окрестности у соседей
    for(int i=1; i <= neighbors[*head][0]; i++){
      int id = neighbors[*head][i];  // индекс соседа воскрешенного яблока
      alive[id]++;
    }
  }

  a_map -> old_head = head;
  a_map -> new_head = head;
}


AppleMap* create_apple_map(char* build_map[], const int num_of_rows, const int length, char c){
  int dummy;
  AppleMap* map = (AppleMap* ) malloc(sizeof(AppleMap));
  map -> apple_symbol = c;

  map -> size = count_pos(build_map, num_of_rows, length, c);
  map -> pos = create_pos_array(build_map, num_of_rows, length, c, map -> size);
  
  map -> id = create_id_map(build_map, num_of_rows, length, c);

  const int R = 2;
  map -> neighbors = create_neighbors(map -> size, R);
  fill_neighbors(build_map, map -> neighbors, map -> id, num_of_rows, length, R, c);

  map -> alive = (int* ) malloc(sizeof(int) * map -> size);
  for(int i=0; i < map->size; i++)
    map -> alive[i] = map -> neighbors[i][0];
  
  map -> dead = (int* ) malloc(sizeof(int) * (map -> size + 1));
  map -> old_head = map -> dead;
  map -> new_head = map -> dead;

  return map;
}

