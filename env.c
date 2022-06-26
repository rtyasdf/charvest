#include<stdlib.h>
#include<stdio.h>
#include"player.c"
#include"utils.c"

typedef struct {
  int num_of_agents;

  char** world_map;

  Pair* apple_pos;
  Pair* spawn_pos;
  Pair* beam_pos;

  int apple_pos_size;
  int spawn_pos_size;
  int beam_pos_size;

  HarvestAgent** agents;
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
  
  env.world_map =  create_world_map(NUM_OF_ROWS, MAP_ROW_LENGTH);
  build_walls(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, env.world_map, '@');
  return env;
}
