from ctypes import *
import numpy as np
import os
from time import perf_counter as pc
from collections import namedtuple


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
              ("agents", POINTER(HarvestAgent)),
              ("dead_agents", char_p)]


Seed = lib.seed
Seed.argtypes = [c_int]


Create = lib.create_env
Create.restype = HarvestEnv
Create.argtypes = [c_int]


Reset = lib.reset
Reset.argtypes = [HarvestEnv, float_p, float_p, float_p, float_p, float_p]


Step = lib.step
Step.argtypes = [HarvestEnv, int_p, float_p, float_p, float_p, float_p, float_p, float_p]


Observation = namedtuple('Observation', 'full_map local_obs position orientation able_to_shoot')


class Env:
  def __init__(self, n_agents: int, n_steps: int=1000, seed: int=0):
    if seed:
      Seed(seed)

    self.c_env = Create(n_agents)
    self.n_steps = n_steps
    self.n_agents = n_agents
    self.create_new_arrays()


  def create_new_arrays(self):
    self.full_map = np.zeros((self.n_steps + 1, 16, 38), dtype=np.float32)
    self.obs = np.zeros((self.n_steps + 1, self.n_agents, 15, 15), dtype=np.float32)

    self.positions = np.zeros((self.n_steps + 1, self.n_agents, 2), dtype=np.float32)
    self.orientations = np.zeros((self.n_steps + 1, self.n_agents), dtype=np.float32)
    self.able_to_shoot = np.zeros((self.n_steps + 1, self.n_agents), dtype=np.float32)

    self.reward = np.zeros((self.n_steps + 1, self.n_agents), dtype=np.float32)


  def reset(self):
    self.t = 0
    self.create_new_arrays()

    Reset(self.c_env, 
          self.full_map[self.t].ravel().ctypes.data_as(float_p), 
          self.obs[self.t].ravel().ctypes.data_as(float_p),
          self.positions[self.t].ravel().ctypes.data_as(float_p),
          self.orientations[self.t].ctypes.data_as(float_p),
          self.able_to_shoot[self.t].ctypes.data_as(float_p))

    return Observation(self.full_map[self.t], self.obs[self.t],
                       self.positions[self.t], self.orientations[self.t], self.able_to_shoot[self.t])


  def step(self, actions):
    self.t += 1
    Step(self.c_env, actions.ctypes.data_as(int_p),
         self.full_map[self.t].ravel().ctypes.data_as(float_p),
         self.obs[self.t].ravel().ctypes.data_as(float_p),
         self.positions[self.t].ravel().ctypes.data_as(float_p),
         self.orientations[self.t].ctypes.data_as(float_p),
         self.able_to_shoot[self.t].ctypes.data_as(float_p),
         self.reward[self.t].ctypes.data_as(float_p))

    obs = Observation(self.full_map[self.t], self.obs[self.t], 
                      self.positions[self.t], self.orientations[self.t], self.able_to_shoot[self.t])

    return obs, self.reward[self.t], self.t == self.n_steps



if __name__ == "__main__":
  n_agents = 5
  n_episodes = 1000

  e = Env(n_agents)

  actions = np.zeros((n_agents, ), dtype=np.int32)

  T = pc()
  for i in range(n_episodes):
    obs = e.reset()
    done = False 
    while not done:
      obs, rew, done = e.step(actions)

  T = pc() - T

  print("Full map:")
  print(obs.full_map)

  for n in range(n_agents):
    print(f"\tAgent {n}:")
    print(obs.local_obs[n])
    print("\n\n")

  print(f"{n_episodes} episodes takes {T : .2f} seconds")
