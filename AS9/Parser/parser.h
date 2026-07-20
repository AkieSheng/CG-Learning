#pragma once

#include <cassert>
#include <cstdio>

#include "vectors.h"

#define MAX_PARSER_TOKEN_LENGTH 100

struct System;
struct Generator;
struct Integrator;
struct ForceField;

struct Parser final {
  Parser(char const* file);
  ~Parser();

  auto getNumSystems() -> int { return num_systems; }
  auto getSystem(int i) -> System* {
    assert(i >= 0 && i < num_systems);
    return systems[i];
  }

private:
  Parser() { assert(0); }

  auto ParseSystem() -> System*;
  auto ParseGenerator() -> Generator*;
  auto ParseIntegrator() -> Integrator*;
  auto ParseForceField() -> ForceField*;

  auto getToken(char token[MAX_PARSER_TOKEN_LENGTH]) -> int;
  auto readVec3f() -> Vec3f;
  auto readVec2f() -> Vec2f;
  auto readFloat() -> float;
  auto readInt() -> int;

  int num_systems{};
  System** systems{};
  FILE* file{};
};
