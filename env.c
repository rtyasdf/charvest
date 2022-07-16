#include<stdlib.h>
#include<stdio.h>
#include"player.c"
#include"utils.c"
#include"apple_map.c"
#include"resolution.c"
#include"fire_utils.c"

typedef struct {
  int num_of_agents;

  char** world_map;

  AppleMap* apple_map;

  Pair* spawn_pos;
  Pair* wall_pos;

  int spawn_pos_size;
  int wall_pos_size;

  HarvestAgent* agents;
  char* dead_agents;
} HarvestEnv;

const int VIEW_SIZE = 7;
const int DIAMETER = (VIEW_SIZE << 1) | 1;
const int RELOAD_TIME = 10;
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



void spawn_agent(HarvestEnv env, HarvestAgent* agent){
  int index = random() % env.spawn_pos_size;
  Pair p = env.spawn_pos[index];

  while (env.world_map[p.y][p.x] != ' '){
    index = random() % env.spawn_pos_size;
    p = env.spawn_pos[index];
  }

  agent -> pos = p;
  agent -> orientation = random() & 3;
  env.world_map[p.y][p.x] = agent -> id + 97;
}


HarvestEnv create_env(int num_of_agents){
  HarvestEnv env;
  env.num_of_agents = num_of_agents;
  
  env.spawn_pos_size = count_pos(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, 'P');
  env.spawn_pos = create_pos_array(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, 'P', env.spawn_pos_size);
  
  env.apple_map = create_apple_map(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, 'A');

  env.wall_pos_size = count_pos(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, '@');
  env.wall_pos = create_pos_array(HARVEST_MAP, NUM_OF_ROWS, MAP_ROW_LENGTH, '@', env.wall_pos_size);
  
  env.world_map =  create_world_map(NUM_OF_ROWS, MAP_ROW_LENGTH);
  fill_map(env.world_map, env.wall_pos, env.wall_pos_size, '@');

  env.agents = (HarvestAgent* ) malloc(sizeof(HarvestAgent) * num_of_agents);
  for(char i=0; i < num_of_agents; i++)
    env.agents[i] = create_agent(i, 0, 0, 0, RELOAD_TIME);

  env.dead_agents = (char* ) malloc(sizeof(char) * (num_of_agents + 1));
  return env;
}


void reset(HarvestEnv env, float* full_map, float* obs,
           float* positions, float* orientations, float* able_to_shoot){
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
  fill_map(env.world_map, env.apple_map -> pos, env.apple_map -> size, 'A');
  
  // * назначить агентам позиции
  //    (и отрисовать их на world_map)
  // * каждому агенту обновить cooldown
  for(int i=0; i < env.num_of_agents; i++){
    HarvestAgent* ptr = &env.agents[i];

    spawn_agent(env, ptr);
    ptr -> cooldown = RELOAD_TIME;

    positions[2 * i] = ptr -> pos.x;
    positions[2 * i + 1] = ptr -> pos.y;
    orientations[i] = ptr -> orientation;
    able_to_shoot[i] = 1;
  }

  // * записать всю карту во float'ах
  get_global_map(env.world_map, full_map, NUM_OF_ROWS, MAP_ROW_LENGTH);

  // * записать наблюдения для каждого агента
  for(int i=0; i < env.num_of_agents; i++)
    get_agent_observation(env.agents[i], env.world_map, 
                          NUM_OF_ROWS, MAP_ROW_LENGTH, VIEW_SIZE,
                          (float* )(obs + DIAMETER * DIAMETER * i));

  reset_apple_map(env.apple_map);
}


// step
//    вход: массив целых чисел (действия принятые агентами)
//    выход: наблюдения агентов (a.k.a. снимки карты), награды, dones (может вообще не возвращать?), info (тот же вопрос)
//  1) расчёт следующих позиций агентов [с учётом поворотов]
//  2) отлов шагов в стену
//  3) conflict resolution
//  4) смена позиций агентов
//  5) поедание яблок и собираем награды
//  6) поворот агентов которые выбрали TURN
//  7) обновляем счётчик "шагов подряд без выстрелов" для каждого агента
//  8) выращиваем новые яблоки
//  9) "рендер" выстрелов
// 10) получаем снимки и всю карту во float'ах
// 11) убираем выстрелы, возвращаем яблоки под ними
// 12) возрождаем пораженных агентов

void step(HarvestEnv env, int* actions, float* full_map, float* obs, 
          float* positions, float* orientations, float* able_to_shoot, float* rewards){

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

  // 4), 5), 6) and 7)
  for(char i=0; i < env.num_of_agents; i++){
    //   если change == 1:  // по построению это автоматически означает, что агент собирался двигаться И будет двигаться
    //     если на нынешней позиции стоит буква этого агента
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
      //consume(&env.agents[i], env.world_map[np.y][np.x]);
      if (env.world_map[np.y][np.x] == env.apple_map -> apple_symbol){
        rewards[i] = 1;
        kill_apple(env.apple_map, np.y, np.x); 
      }

      env.world_map[np.y][np.x] = 97 + i;
    }
    else if (actions[i] == 5){
      env.agents[i].orientation++;
      env.agents[i].orientation &= 3;
    }
    else if (actions[i] == 6){
      env.agents[i].orientation += 3;
      env.agents[i].orientation &= 3;
    }
    env.agents[i].cooldown++;
  }

  // 8)
  spawn_apples(env.world_map, env.apple_map);

  // 9)
  int* h_id = env.apple_map -> hide;
  char* dead = env.dead_agents;
  for(int i=0; i < env.num_of_agents; i++){
    if (actions[i] != 7  ||  env.agents[i].cooldown < RELOAD_TIME)
      continue;

    if (env.agents[i].orientation & 1)
      fire_horizontal(env.world_map, MAP_ROW_LENGTH,
                      env.agents[i], env.apple_map->id, 
                      &dead, &h_id);
    else
      fire_vertical(env.world_map, NUM_OF_ROWS,
                    env.agents[i], env.apple_map->id, 
                    &dead, &h_id);
  }

  // 10)
  get_global_map(env.world_map, full_map, NUM_OF_ROWS, MAP_ROW_LENGTH);

  for(int i=0; i < env.num_of_agents; i++){
    get_agent_observation(env.agents[i], env.world_map,
                          NUM_OF_ROWS, MAP_ROW_LENGTH, VIEW_SIZE,
                          (float* )(obs + DIAMETER * DIAMETER * i));
   
    positions[2 * i] = env.agents[i].pos.x;
    positions[2 * i + 1] = env.agents[i].pos.y;
    orientations[i] = env.agents[i].orientation;
    able_to_shoot[i] = env.agents[i].cooldown < RELOAD_TIME ? 0 : 1;
  }

  // 11)
  for(char i=0; i < env.num_of_agents; i++){
    if (actions[i] != 7  ||  env.agents[i].cooldown < RELOAD_TIME)
      continue;

    env.agents[i].cooldown = 0;
    able_to_shoot[i] = 0;

    if (env.agents[i].orientation & 1)
      clean_horizontal(env.world_map, MAP_ROW_LENGTH, env.agents[i]);
    else
      clean_vertical(env.world_map, NUM_OF_ROWS, env.agents[i]);
  }

  while (h_id != env.apple_map -> hide){
    h_id--;
    Pair p = (env.apple_map -> pos)[*h_id];
    env.world_map[p.y][p.x] = 'A';
  }

  // 12)
  while (dead != env.dead_agents){
    dead--;
    spawn_agent(env, &env.agents[*dead]);
  }
}

