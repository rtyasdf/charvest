#include<stdlib.h>
#include<stdio.h>
#include"utils.c"

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
    
    env.agents[i].pos_x = p.x;
    env.agents[i].pos_y = p.y;
    env.agents[i].orientation = random() & 3;
    env.world_map[p.y][p.x] = env.agents[i].id + 97;
  }

  // * записать наблюдения для каждого агента
  for(int i=0; i < env.num_of_agents; i++)
    get_agent_observation(env.agents[i], env.world_map, 
                          NUM_OF_ROWS, MAP_ROW_LENGTH, 
                          (char* )(obs + 7 * 7 * i));
}
