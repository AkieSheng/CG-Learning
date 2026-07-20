#include "rayTree.h"
#include "gl_headers.h"

int RayTree::activated = 0;
Segment RayTree::main_segment;
SegmentVector RayTree::shadow_segments;
SegmentVector RayTree::reflected_segments;
SegmentVector RayTree::transmitted_segments;

auto RayTree::Print() -> void {
  main_segment.Print("main       ");
  for (auto i = 0; i < shadow_segments.getNumSegments(); i++) {
    shadow_segments.getSegment(i).Print("shadow     ");
  }
  for (auto i = 0; i < reflected_segments.getNumSegments(); i++) {
    reflected_segments.getSegment(i).Print("reflected  ");
  }
  for (auto i = 0; i < transmitted_segments.getNumSegments(); i++) {
    transmitted_segments.getSegment(i).Print("transmitted");
  }
}

auto RayTree::paintHelper(Vec4f const& m, Vec4f const& s, Vec4f const& r,
                          Vec4f const& t) -> void {
  ::glBegin(GL_LINES);
  ::glColor4f(m.r(), m.g(), m.b(), m.a());
  main_segment.paint();
  ::glColor4f(s.r(), s.g(), s.b(), s.a());
  for (auto i = 0; i < shadow_segments.getNumSegments(); i++) {
    shadow_segments.getSegment(i).paint();
  }
  ::glColor4f(r.r(), r.g(), r.b(), r.a());
  for (auto i = 0; i < reflected_segments.getNumSegments(); i++) {
    reflected_segments.getSegment(i).paint();
  }
  ::glColor4f(t.r(), t.g(), t.b(), t.a());
  for (auto i = 0; i < transmitted_segments.getNumSegments(); i++) {
    transmitted_segments.getSegment(i).paint();
  }
  ::glEnd();
}

auto RayTree::paint() -> void {
  ::glLineWidth(2);
  ::glDisable(GL_LIGHTING);

  ::glDisable(GL_DEPTH_TEST);
  ::glEnable(GL_BLEND);
  ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_COLOR);
  paintHelper(Vec4f(0.5, 0.5, 0.5, 0.4), Vec4f(0.1, 0.9, 0.1, 0.4),
              Vec4f(0.9, 0.1, 0.1, 0.4), Vec4f(0.1, 0.1, 0.9, 0.4));
  ::glDisable(GL_BLEND);
  ::glEnable(GL_DEPTH_TEST);

  paintHelper(Vec4f(0.5, 0.5, 0.5, 1.0), Vec4f(0.1, 0.9, 0.1, 1.0),
              Vec4f(0.9, 0.1, 0.1, 1.0), Vec4f(0.1, 0.1, 0.9, 1.0));

  ::glEnable(GL_LIGHTING);
}
