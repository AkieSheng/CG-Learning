#include "film.h"
#include "image.h"
#include "filter.h"
#include "math.h"

void Film::renderSamples(char *samples_file, int sample_zoom)
{

  Image image = Image(width*sample_zoom,height*sample_zoom);

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {

      for (int zi = i*sample_zoom; zi < (i+1)*sample_zoom; zi++) {
  for (int zj = j*sample_zoom; zj < (j+1)*sample_zoom; zj++) {
    if (((i+j)%2)==0)
      image.SetPixel(zi,zj,Vec3f(0.2,0.2,0.2));
    else
      image.SetPixel(zi,zj,Vec3f(0,0,0));
  }
      }

      for (int n = 0; n < num_samples; n++) {
  Sample s = getSample(i,j,n);
  Vec2f p = s.getPosition();
  assert(p.x() >= 0 && p.x() <= 1);
  assert(p.y() >= 0 && p.y() <= 1);
  int x = static_cast<int>(::floor((i+p.x())*sample_zoom));
  int y = static_cast<int>(::floor((j+p.y())*sample_zoom));
  assert(x >= 0);
  if (x >= width*sample_zoom) x = width*sample_zoom - 1;
  assert(y >= 0);
  if (y >= height*sample_zoom) y = height*sample_zoom - 1;
  image.SetPixel(x,y,s.getColor());
      }
    }
  }

  image.SaveTGA(samples_file);
}

void Film::renderFilter(char *filter_file, int filter_zoom, Filter *filter)
{

  Image image = Image(width*filter_zoom+1,height*filter_zoom+1);
  image.SetAllPixels(Vec3f(0,0,0.5));

  int support_radius = filter->getSupportRadius();

  int center = width/2;
  for (int i = center-support_radius; i <= center+support_radius; i++) {
    for (int j = center-support_radius; j <= center+support_radius; j++) {
      if (i < 0 || i >= width || j < 0 || j >= height) continue;

      for (int i2 = 0; i2 < filter_zoom; i2++) {
  for (int j2 = 0; j2 < filter_zoom; j2++) {
    float x = i + i2/float(filter_zoom) - (center+0.5);
    float y = j + j2/float(filter_zoom) - (center+0.5);
    float weight = filter->getWeight(x,y);
    image.SetPixel(i*filter_zoom+i2,j*filter_zoom+j2,
       Vec3f(weight,weight,weight));
  }
      }
    }
  }

  for (int x = 0; x < width*filter_zoom+1; x++) {
    for (int y = 0; y < height*filter_zoom+1; y++) {
      if (x%filter_zoom == 0 ||
    y%filter_zoom == 0)
  image.SetPixel(x,y,Vec3f(0,0,1));
    }
  }

  image.SaveTGA(filter_file);
}
