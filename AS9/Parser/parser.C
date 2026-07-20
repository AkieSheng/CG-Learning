#include <cstdio>
#include <cstring>

#include "parser.h"
#include "system.h"
#include "generator.h"
#include "integrator.h"
#include "forcefield.h"

Parser::Parser(char const* filename) {
  assert(filename != nullptr);
  file = ::fopen(filename, "r");
  assert(file != nullptr);
  char token[MAX_PARSER_TOKEN_LENGTH];

  getToken(token);
  assert(!::strcmp(token, "num_systems"));
  num_systems = readInt();
  systems = new System*[num_systems];

  for (auto i = 0; i < num_systems; i++) {
    auto* s = ParseSystem();
    assert(s != nullptr);
    systems[i] = s;
  }
  ::fclose(file);
}

Parser::~Parser() {
  for (auto i = 0; i < num_systems; i++) {
    delete systems[i];
  }
  delete[] systems;
}

auto Parser::ParseSystem() -> System* {
  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "system"));
  auto* generator = ParseGenerator();
  auto* integrator = ParseIntegrator();
  auto* forcefield = ParseForceField();
  return new System(generator, integrator, forcefield);
}

auto Parser::ParseGenerator() -> Generator* {
  char type[MAX_PARSER_TOKEN_LENGTH];
  getToken(type);

  auto position = Vec3f(0, 0, 0);
  auto position_randomness = 0.0f;
  auto velocity = Vec3f(0, 0, 0);
  auto velocity_randomness = 0.0f;
  auto color = Vec3f(1, 1, 1);
  auto dead_color = Vec3f(1, 1, 1);
  auto color_randomness = 0.0f;
  auto mass = 1.0f;
  auto mass_randomness = 0.0f;
  auto lifespan = 10.0f;
  auto lifespan_randomness = 0.0f;
  auto desired_num_particles = 1000;

  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  while (1) {
    getToken(token);
    if (!::strcmp(token, "position")) {
      position = readVec3f();
    } else if (!::strcmp(token, "position_randomness")) {
      position_randomness = readFloat();
    } else if (!::strcmp(token, "velocity")) {
      velocity = readVec3f();
    } else if (!::strcmp(token, "velocity_randomness")) {
      velocity_randomness = readFloat();
    } else if (!::strcmp(token, "color")) {
      color = readVec3f();
      dead_color = color;
    } else if (!::strcmp(token, "dead_color")) {
      dead_color = readVec3f();
    } else if (!::strcmp(token, "color_randomness")) {
      color_randomness = readFloat();
    } else if (!::strcmp(token, "mass")) {
      mass = readFloat();
    } else if (!::strcmp(token, "mass_randomness")) {
      mass_randomness = readFloat();
    } else if (!::strcmp(token, "lifespan")) {
      lifespan = readFloat();
    } else if (!::strcmp(token, "lifespan_randomness")) {
      lifespan_randomness = readFloat();
    } else if (!::strcmp(token, "desired_num_particles")) {
      desired_num_particles = readInt();
    } else if (::strcmp(token, "}")) {
      ::printf("ERROR unknown generator token %s\n", token);
      assert(0);
    } else {
      break;
    }
  }

  Generator* answer = nullptr;
  if (!::strcmp(type, "hose_generator")) {
    answer = new HoseGenerator(position, position_randomness, velocity, velocity_randomness);
  } else if (!::strcmp(type, "ring_generator")) {
    answer = new RingGenerator(position_randomness, velocity, velocity_randomness);
  } else {
    ::printf("WARNING:  unknown generator type '%s'\n", type);
    ::printf("WARNING:  unknown generator type '%s'\n", type);
  }

  assert(answer != nullptr);
  answer->SetColors(color, dead_color, color_randomness);
  answer->SetMass(mass, mass_randomness);
  answer->SetLifespan(lifespan, lifespan_randomness, desired_num_particles);

  return answer;
}

auto Parser::ParseIntegrator() -> Integrator* {
  char type[MAX_PARSER_TOKEN_LENGTH];
  getToken(type);

  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  getToken(token);
  assert(!::strcmp(token, "}"));

  Integrator* answer = nullptr;
  if (!::strcmp(type, "euler_integrator")) {
    answer = new EulerIntegrator();
  } else if (!::strcmp(type, "midpoint_integrator")) {
    answer = new MidpointIntegrator();
  } else if (!::strcmp(type, "rungekutta_integrator")) {
    answer = new RungeKuttaIntegrator();
  } else {
    ::printf("WARNING:  unknown integrator type '%s'\n", type);
  }
  assert(answer != nullptr);
  return answer;
}

auto Parser::ParseForceField() -> ForceField* {
  char type[MAX_PARSER_TOKEN_LENGTH];
  getToken(type);

  auto gravity = Vec3f(0, 0, 0);
  auto force = Vec3f(0, 0, 0);
  auto magnitude = 1.0f;

  char token[MAX_PARSER_TOKEN_LENGTH];
  getToken(token);
  assert(!::strcmp(token, "{"));
  while (1) {
    getToken(token);
    if (!::strcmp(token, "gravity")) {
      gravity = readVec3f();
    } else if (!::strcmp(token, "force")) {
      force = readVec3f();
    } else if (!::strcmp(token, "magnitude")) {
      magnitude = readFloat();
    } else if (::strcmp(token, "}")) {
      ::printf("ERROR unknown gravity token %s\n", token);
      assert(0);
    } else {
      break;
    }
  }

  ForceField* answer = nullptr;
  if (!::strcmp(type, "gravity_forcefield")) {
    answer = new GravityForceField(gravity);
  } else if (!::strcmp(type, "constant_forcefield")) {
    answer = new ConstantForceField(force);
  } else if (!::strcmp(type, "radial_forcefield")) {
    answer = new RadialForceField(magnitude);
  } else if (!::strcmp(type, "vertical_forcefield")) {
    answer = new VerticalForceField(magnitude);
  } else if (!::strcmp(type, "wind_forcefield")) {
    answer = new WindForceField(magnitude);
  } else {
    ::printf("WARNING:  unknown forcefield type '%s'\n", type);
  }
  assert(answer != nullptr);
  return answer;
}

auto Parser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) -> int {
  assert(file != nullptr);
  auto success = ::fscanf(file, "%s ", token);
  if (success == EOF) {
    token[0] = '\0';
    return 0;
  }
  return 1;
}

auto Parser::readVec3f() -> Vec3f {
  float x, y, z;
  auto count = ::fscanf(file, "%f %f %f", &x, &y, &z);
  if (count != 3) {
    ::printf("Error trying to read 3 floats to make a Vec3f\n");
    assert(0);
  }
  return Vec3f(x, y, z);
}

auto Parser::readVec2f() -> Vec2f {
  float u, v;
  auto count = ::fscanf(file, "%f %f", &u, &v);
  if (count != 2) {
    ::printf("Error trying to read 2 floats to make a Vec2f\n");
    assert(0);
  }
  return Vec2f(u, v);
}

auto Parser::readFloat() -> float {
  float answer;
  auto count = ::fscanf(file, "%f", &answer);
  if (count != 1) {
    ::printf("Error trying to read 1 float\n");
    assert(0);
  }
  return answer;
}

auto Parser::readInt() -> int {
  int answer;
  auto count = ::fscanf(file, "%d", &answer);
  if (count != 1) {
    ::printf("Error trying to read 1 int\n");
    assert(0);
  }
  return answer;
}
