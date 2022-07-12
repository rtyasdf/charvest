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
  def __init__(self, n_agents: int, n_steps: int=1000):
    self.c_env = Create(n_agents)
    self.n_steps = n_steps
    self.obs = np.zeros((n_steps + 1, n_agents, 15, 15), dtype=np.float32)
    self.full_map = np.zeros((n_steps + 1, 16, 38), dtype=np.float32)
    self.reward = np.zeros((n_steps + 1, n_agents), dtype=np.float32)

  def reset(self):
    self.t = 0

    self.obs.fill(0)
    self.full_map.fill(0)
    self.reward.fill(0)

    Reset(self.c_env, 
          self.full_map[self.t].ravel().ctypes.data_as(float_p), 
          self.obs[self.t].ravel().ctypes.data_as(float_p))

    return self.full_map[self.t], self.obs[self.t]

  def step(self, actions):
    self.t += 1
    Step(self.c_env, actions.ctypes.data_as(int_p),
         self.full_map[self.t].ravel().ctypes.data_as(float_p),
         self.obs[self.t].ravel().ctypes.data_as(float_p),
         self.reward[self.t].ctypes.data_as(float_p))

    return self.full_map[self.t], self.obs[self.t], self.reward[self.t], self.t == self.n_steps



if __name__ == "__main__":
  n_agents = 5
  n_episodes = 1000

  e = Env(n_agents)

  actions = np.zeros((n_agents, ), dtype=np.int32)

  T = pc()
  for i in range(n_episodes):
    fmap, obs = e.reset()
    done = False 
    while not done:
      fmap, obs, rew, done = e.step(actions)

  T = pc() - T

  print("Full map:")
  print(fmap)

  for n in range(n_agents):
    print(f"\tAgent {n}:")
    print(obs[n])
    print("\n\n")

  print(f"{n_episodes} episodes takes {T : .2f} seconds")

