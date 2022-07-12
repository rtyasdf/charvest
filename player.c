typedef struct{
  int x;
  int y;
} Pair;


typedef struct{
  char id;
  Pair pos;
  char orientation;
  int cooldown;
} HarvestAgent;


HarvestAgent create_agent(char id, int pos_x, int pos_y, char orientation, int cooldown){
  HarvestAgent agent;

  agent.id = id;
  agent.pos.x = pos_x;
  agent.pos.y = pos_y;
  agent.orientation = orientation;
  agent.cooldown = cooldown;
  
  return agent;
}
