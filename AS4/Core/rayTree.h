#pragma once

#include <cassert>
#include "ray.h"
#include "gl_headers.h"

struct Segment final {
  Segment() { Clear(); }
  Segment(Ray const& ray, float tstart, float tstop) {
    if (tstart < -100.0f) {
      tstart = -100.0f;
    }
    if (tstop > 100.0f) {
      tstop = 100.0f;
    }
    a = ray.pointAtParameter(tstart);
    b = ray.pointAtParameter(tstop);
  }
  Segment(Segment const& s) { a = s.a; b = s.b; }
  ~Segment() {}

  auto Clear() -> void { a = Vec3f(0, 0, 0); b = Vec3f(0, 0, 0); }
  auto Print(char const* s) -> void {
    ::printf(" %s (%6.3f %6.3f %6.3f) -> (%6.3f %6.3f %6.3f)\n",
             s, a.x(), a.y(), a.z(), b.x(), b.y(), b.z());
  }
  auto paint() -> void {
    ::glVertex3f(a.x(), a.y(), a.z());
    ::glVertex3f(b.x(), b.y(), b.z());
  }

  Vec3f a{};
  Vec3f b{};
};

struct SegmentVector final {
  SegmentVector() {
    num_segments = 0;
    size = 10;
    segments = new Segment[size];
  }
  ~SegmentVector() { delete[] segments; }
  auto Clear() -> void { num_segments = 0; }

  auto getNumSegments() -> int { return num_segments; }
  auto getSegment(int i) -> Segment {
    assert(i >= 0 && i < num_segments);
    return segments[i];
  }

  auto addSegment(Segment const& s) -> void {
    if (size == num_segments) {
      auto new_size = size * 2;
      auto* new_segments = new Segment[new_size];
      for (auto i = 0; i < size; i++) {
        new_segments[i] = segments[i];
      }
      delete[] segments;
      segments = new_segments;
      size = new_size;
    }
    segments[num_segments] = s;
    num_segments++;
  }

  Segment* segments{};
  int size{};
  int num_segments{};
};

struct RayTree {
  static auto Activate() -> void { Clear(); activated = 1; }
  static auto Deactivate() -> void { activated = 0; }

  static auto SetMainSegment(Ray const& ray, float tstart, float tstop) -> void {
    if (!activated) {
      return;
    }
    main_segment = Segment(ray, tstart, tstop);
  }
  static auto AddShadowSegment(Ray const& ray, float tstart, float tstop) -> void {
    if (!activated) {
      return;
    }
    shadow_segments.addSegment(Segment(ray, tstart, tstop));
  }
  static auto AddReflectedSegment(Ray const& ray, float tstart, float tstop) -> void {
    if (!activated) {
      return;
    }
    reflected_segments.addSegment(Segment(ray, tstart, tstop));
  }
  static auto AddTransmittedSegment(Ray const& ray, float tstart, float tstop) -> void {
    if (!activated) {
      return;
    }
    transmitted_segments.addSegment(Segment(ray, tstart, tstop));
  }

  static auto paint() -> void;
  static auto Print() -> void;

  static auto paintHelper(Vec4f const& m, Vec4f const& s, Vec4f const& r,
                          Vec4f const& t) -> void;
  static auto Clear() -> void {
    main_segment.Clear();
    shadow_segments.Clear();
    reflected_segments.Clear();
    transmitted_segments.Clear();
  }

  static int activated;
  static Segment main_segment;
  static SegmentVector shadow_segments;
  static SegmentVector reflected_segments;
  static SegmentVector transmitted_segments;
};
