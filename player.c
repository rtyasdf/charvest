#include <stdlib.h>

typedef struct{
  char id;
  int pos_x;
  int pos_y;
  char orientation;
  int reward;
} HarvestAgent;


HarvestAgent* create_agent(char id, int pos_x, int pos_y, char orientation){
  HarvestAgent* agent = (HarvestAgent* ) malloc(sizeof(HarvestAgent));

  agent -> id = id;
  agent -> pos_x = pos_x;
  agent -> pos_y = pos_y;
  agent -> orientation = orientation;
  agent -> reward = 0;
  
  return agent;
}


char consume(HarvestAgent* agent, char c){
  if (c == 'A'){
    agent -> reward++;
    return ' ';
  }
  return c;
}
