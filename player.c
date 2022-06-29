typedef struct{
  int x;
  int y;
} Pair;


typedef struct{
  char id;
  Pair pos;
  char orientation;
  int reward;
} HarvestAgent;


HarvestAgent create_agent(char id, int pos_x, int pos_y, char orientation){
  HarvestAgent agent;

  agent.id = id;
  agent.pos.x = pos_x;
  agent.pos.y = pos_y;
  agent.orientation = orientation;
  agent.reward = 0;
  
  return agent;
}


void consume(HarvestAgent* agent, char c){
  if (c == 'A')
    agent -> reward++;
}
