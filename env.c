#include<stdlib.h>
#include<stdio.h>
#include"player.c"
#include"utils.c"
#include"resolution.c"

typedef struct {
  int num_of_agents;

  char** world_map;

  Pair* apple_pos;
  Pair* spawn_pos;
  Pair* beam_pos;
  Pair* wall_pos;

  int apple_pos_size;
  int spawn_pos_size;
  int beam_pos_size;
  int wall_pos_size;

  HarvestAgent* agents;
} HarvestEnv;

const int NUM_OF_ROWS = 16;
const int MAP_ROW_LENGTH = 38;
char* HARVEST_MAP[16] = {"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
                         "@ P   P      A    P AAAAA    P  A P  @",
                         "@  P     A P AA    P    AAA    A  A  @",
                         "@     A AAA  AAA    A    A AA AAAA   @",
                         "@ A  AAA A    A  A AAA  A  A   A A   @",
                         "@AAA  A A    A  AAA A  AAA        A P@",
                         "@ A A  AAA  AAA  A A    A AA   AA AA @",
                         "@  A A  AAA    A A  AAA    AAA  A    @",
                         "@   AAA  A      AAA  A    AAAA       @",
                         "@ P  A       A  A AAA    A  A      P @",
                         "@A  AAA  A  A  AAA A    AAAA     P   @",
                         "@    A A   AAA  A A      A AA   A  P @",
                         "@     AAA   A A  AAA      AA   AAA P @",
                         "@ A    A     AAA  A  P          A    @",
                         "@       P     A         P  P P     P @",
                         "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"};


HarvestEnv create_env(int num_of_agents){
  HarvestEnv env;
  env.num_of_agents = num_of_agents;
  
  env.spawn_pos_size = count_pos(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, 'P');
  env.spawn_pos = create_pos_array(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, 'P', env.spawn_pos_size);
  
  env.apple_pos_size = count_pos(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, 'A');
  env.apple_pos = create_pos_array(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, 'A', env.apple_pos_size);
  
  env.wall_pos_size = count_pos(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, '@');
  env.wall_pos = create_pos_array(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, '@', env.wall_pos_size);
  
  env.world_map =  create_world_map(NUM_OF_ROWS, MAP_ROW_LENGTH);
  fill_map(env.world_map, env.wall_pos, env.wall_pos_size, '@');

  env.agents = (HarvestAgent* ) malloc(sizeof(HarvestAgent) * num_of_agents);
  for(int i=0; i<num_of_agents; i++)
    env.agents[i] = create_agent(i, 0, 0, 0);

  return env;
}


void reset(HarvestEnv env, char* obs){
  /*
  1. Несмотря на то, что python'овский float на деле есть double,
       pytorch ожидает настоящий C-шный float, 
       поэтому лучше заранее объявить массив obs извне типом np.float32
  2. Почему было принято решение передавать массив с наблюдениями как аргумент,
       вместо того, чтобы возвращать его?
       Потому что, если возвращать, то перед этим придется вызывать malloc,
        а потом эту память нужно ещё и очистить,
        и видимо это придётся делать, как дополнительный вызов какой-то функции
        уже в Python'овском коде после того как нейронки прогнали эти наблюдения.
        Запарно => лучше так.
  */
  
  // * очистить карту (world_map)
  for (int i = 0; i < NUM_OF_ROWS; i++)
    for (int j = 0; j < MAP_ROW_LENGTH; j++)
      env.world_map[i][j] = ' ';

  // * поставить стены
  fill_map(env.world_map, env.wall_pos, env.wall_pos_size, '@');

  // * поставить яблоки
  fill_map(env.world_map, env.apple_pos, env.apple_pos_size, 'A');
  
  // * назначить агентам позиции
  //    (и отрисовать их на world_map)
  int index = random() % env.spawn_pos_size;
  Pair p = env.spawn_pos[index];

  for(int i=0; i < env.num_of_agents; i++){
    while (env.world_map[p.y][p.x] != ' '){
      index = random() % env.spawn_pos_size;
      p = env.spawn_pos[index];
    }
    
    env.agents[i].pos = p;
    env.agents[i].orientation = random() & 3;
    env.world_map[p.y][p.x] = env.agents[i].id + 97;
  }

  // * записать наблюдения для каждого агента
  for(int i=0; i < env.num_of_agents; i++)
    get_agent_observation(env.agents[i], env.world_map, 
                          NUM_OF_ROWS, MAP_ROW_LENGTH, 
                          (char* )(obs + 7 * 7 * i));
}


// step
//    вход: массив целых чисел (действия принятые агентами)
//    выход: наблюдения агентов (a.k.a. снимки карты), награды, dones (может вообще не возвращать?), info (тот же вопрос)
//  1) расчёт следующих позиций агентов [здесь нужно учитывать повороты]
//  2) ?? отлавливать шаги в стену ??
//  3) conflict resolution
//  4) ?? поменять позиции агентов ??
//  5) обработать поедание яблок
//  6) повернуть агентов которые выбрали TURN
//  7) ?? выстрелы ??
//  8) выращиваем новые яблоки
//  9) получаем снимки
// 10) собираем награды

void step(HarvestEnv env, int* actions, char* obs, int* rewards){

  // 1) and 2) 
  Pair next_pos[env.num_of_agents];
  for(int i=0; i < env.num_of_agents; i++){
    next_pos[i] = get_next_pos(env.agents[i].pos, env.agents[i].orientation, actions[i]);

    int y = next_pos[i].y;
    int x = next_pos[i].x;
    if (env.world_map[y][x] == '@')
      next_pos[i] = env.agents[i].pos;
  }

  // 3)
  int change[2 * env.num_of_agents];
  for(int i=0; i < 2 * env.num_of_agents; i++)
    change[i] = 0;
  
  update_moves(env.world_map, env.num_of_agents, (Pair* )next_pos, (int* )change); 


  // 4) and 5)
  for(char i=0; i < env.num_of_agents; i++){
    //   если change == 1:  // по построению это автоматически означает, что агент собирался двигаться И будет двигаться
    //     если на нынешней позиции стоит буква агента
    //       поставить там пробел
    //     заменить поля агента новой позицией
    //     обработать следующую позицию (типа съесть яблоко если оно там есть)
    //     поставить агента на карту в соответствии с позицией

    if (change[i] == 1){
      Pair p = env.agents[i].pos;
      if (env.world_map[p.y][p.x] == 97 + i)
        env.world_map[p.y][p.x] = ' ';
      

      Pair np = next_pos[i];
      env.agents[i].pos = np;

      // съесть яблоко в новой локации
      consume(&env.agents[i], env.world_map[np.y][np.x]);

      env.world_map[np.y][np.x] = 97 + i;
    }
  }
}
