#include "rayTree.h"



int RayTree::activated = 0;  
Segment RayTree::main_segment;
SegmentVector RayTree::shadow_segments;
SegmentVector RayTree::reflected_segments;
SegmentVector RayTree::transmitted_segments;
CellFaceVector RayTree::hit_cells;
CellFaceVector RayTree::entered_faces;



auto RayTree::Print() -> void {
  main_segment.Print("main       ");
  int i;
  for (int i = 0; i < shadow_segments.getNumSegments(); i++) {
    shadow_segments.getSegment(i).Print("shadow     "); }
  for (int i = 0; i < reflected_segments.getNumSegments(); i++) {
    reflected_segments.getSegment(i).Print("reflected  "); }
  for (int i = 0; i < transmitted_segments.getNumSegments(); i++) {
    transmitted_segments.getSegment(i).Print("transmitted"); }
}

auto RayTree::paintHelper(Vec4f const&m,Vec4f const&s,Vec4f const&r,Vec4f const&t) -> void {
  glBegin(GL_LINES);
  glColor4f(m.r(),m.g(),m.b(),m.a());
  main_segment.paint();
  int i;
  glColor4f(s.r(),s.g(),s.b(),s.a());
  for (int i = 0; i < shadow_segments.getNumSegments(); i++) {
    shadow_segments.getSegment(i).paint(); }
  glColor4f(r.r(),r.g(),r.b(),r.a());
  for (int i = 0; i < reflected_segments.getNumSegments(); i++) {
    reflected_segments.getSegment(i).paint(); }
  glColor4f(t.r(),t.g(),t.b(),t.a());
  for (int i = 0; i < transmitted_segments.getNumSegments(); i++) {
    transmitted_segments.getSegment(i).paint(); }  
  glEnd();
}



auto RayTree::paint() -> void {
  glLineWidth(2);
  glDisable(GL_LIGHTING);



  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  paintHelper(Vec4f(0.7,0.7,0.7,0.3),
	      Vec4f(0.1,0.9,0.1,0.3),
	      Vec4f(0.9,0.1,0.1,0.3),
	      Vec4f(0.1,0.1,0.9,0.3));
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);


  paintHelper(Vec4f(0.7,0.7,0.7,1.0),
	      Vec4f(0.1,0.9,0.1,1.0),
	      Vec4f(0.9,0.1,0.1,1.0),
	      Vec4f(0.1,0.1,0.9,1.0));
	      
  glEnable(GL_LIGHTING);
}



auto RayTree::paintHitCells() -> void {

  for (int i = 0; i < hit_cells.getNumCellFaces(); i++) {
    hit_cells.getCellFace(i).paint(); }
}



auto RayTree::paintEnteredFaces() -> void {

  for (int i = 0; i < entered_faces.getNumCellFaces(); i++) {
    entered_faces.getCellFace(i).paint(); }
}

