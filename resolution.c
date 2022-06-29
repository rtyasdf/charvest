void resolve(char graph[][4], char len[], int* change, char index){
  change[index] = 1;
  if (len[index] == 0)
    return;
  
  char child = random() % len[index];
  resolve(graph, len, change, graph[index][child]);
}


void update_moves(char** map, int num_of_agents, Pair* next_pos, int* change){
  int double_size = 2 * num_of_agents;

  char graph[double_size][4];
  char length[double_size];
  for(int i=0; i < double_size ; i++)
    length[i] = 0;

  char head = 0;
  char backup[num_of_agents];

  // Первый проход (конструируем граф)
  for(int i=0; i < num_of_agents; i++){
    Pair np = next_pos[i];
    char ns = map[np.y][np.x];
    char index;
    if (ns > 96){  // клетка, на которой сейчас стоит другой агент
      index = ns - 97;
      if (i == index) // не допускаем ссылку на самого себя
        continue;
    }
    else if (ns > 31){  // пустая клетка, которую здесь помечаю
      backup[head] = ns;
      map[np.y][np.x] = head;
      index = num_of_agents + head;
      head++;
    }
    else{  // клетка, которая уже была кем-то помечена
      index = num_of_agents + ns; // смещение потому что клетка назначения без агента
    }
    graph[index][length[index]] = i;
    length[index]++;
  }


  for(; head > 0; head--)
    resolve(graph, length, change, num_of_agents + head - 1);


  // Восстанавливаем исходные значения ячеек
  for(int i=0; i < num_of_agents; i++){
    Pair np = next_pos[i];
    char ns = map[np.y][np.x];
    if (ns < 32)  // клетка, которую изменили при первом проходе
      map[np.y][np.x] = backup[ns];
  }
}
