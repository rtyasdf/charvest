from ctypes import *
import numpy as np
import os
from time import perf_counter as pc


dir_path = os.path.dirname(__file__)
lib = CDLL(dir_path + "/env.so")

char_p = POINTER(c_char)
char_pp = POINTER(char_p)
int_p = POINTER(c_int)
int_pp = POINTER(int_p)
float_p = POINTER(c_float)

class Pair(Structure):
  _fields_ = [("x", c_int),
              ("y", c_int)]

pair_p = POINTER(Pair)


class HarvestAgent(Structure):
  _fields_ = [("id", c_char),
              ("pos", Pair),
              ("orientation", c_char),
              ("reward", c_int)]

agent_p = POINTER(HarvestAgent)


class AppleMap(Structure):
  _fields_ = [("pos", pair_p),
              ("size", c_int),
              ("apple_symbol", c_char),
              ("id", int_pp),
              ("neighbors", int_pp),
              ("alive", int_p),
              ("dead", int_p),
              ("new_head", int_p),
              ("old_head", int_p),
              ("hide", int_p)]


class HarvestEnv(Structure):
  _fields_ = [("num_of_agents", c_int),
              ("world_map", char_pp),
              ("apple_map", POINTER(AppleMap)),
              ("spawn_pos", pair_p),
              ("wall_pos", pair_p),
              ("spawn_pos_size", c_int),
              ("wall_pos_size", c_int),
              ("agents", agent_p),
              ("dead_agents", char_p)]


Create = lib.create_env
Create.restype = HarvestEnv
Create.argtypes = [c_int]


Reset = lib.reset
Reset.argtypes = [HarvestEnv, float_p]


Step = lib.step
Step.argtypes = [HarvestEnv, int_p, float_p, float_p]


class Env:
  def __init__(self, n_agents: int):
    self.c_env = Create(n_agents)

  def reset(self, obs):
    Reset(self.c_env, obs.ctypes.data_as(float_p))

  def step(self, actions, obs, rewards):
    Step(self.c_env, actions.ctypes.data_as(int_p), obs.ctypes.data_as(float_p), rewards.ctypes.data_as(float_p))



if __name__ == "__main__":
  n_agents = 5

  e = Env(n_agents)

  observation = np.zeros((n_agents * 15 * 15, ), dtype=np.float32)
  fake_rewards = np.zeros((n_agents, ), dtype=np.float32)
  actions = np.zeros((n_agents, ), dtype=np.int32)

  T = pc()
  for i in range(1000):
    e.reset(observation)    
    for _ in range(1000):
      e.step(actions, observation, fake_rewards)
      y = observation.reshape((n_agents, 15, 15))  # NO COPY

  for n in range(n_agents):
    print(f"\tAgent {n}:")
    print(y[n])
    print("\n\n")

  print(f"1000 episodes takes {pc() - T : .2f} seconds")
