#include <stdio.h>
#include <stdlib.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "math.h"
#include "gmath.h"
#include "symtab.h"
#include "uthash.h"

struct normals {
  char vertex[256];
  double normal[3];
  UT_hash_handle hh;
};

/*======== void draw_scanline() ==========
  Inputs: struct matrix *points
          int i
          screen s
          zbuffer zb
          color c

  Line algorithm specifically for horizontal scanlines
  ====================*/
void draw_scanline(int x0, double z0, int x1, double z1, int y, screen s, zbuffer zb, color c) {
  int tx, tz;
  //swap if needed to assure left->right drawing
  if (x0 > x1) {
    tx = x0;
    tz = z0;
    x0 = x1;
    z0 = z1;
    x1 = tx;
    z1 = tz;
  }

  double delta_z;
  delta_z = (x1 - x0) != 0 ? (z1 - z0) / (x1 - x0 + 1) : 0;
  int x;
  double z = z0;

  for(x=x0; x <= x1; x++) {
    plot(s, zb, c, x, y, z);
    z+= delta_z;
  }
}

/*
void draw_scanline_gouraud(int x0, double z0, int x1, double z1, int y, screen s, zbuffer zb, color c0, color c1) {
  int tmpX, tmpZ;
  color tmpC;
  double cred, cgreen, cblue, dred, dgreen, dblue;
  color cF;
  if (x0 > x1) {
    tmpX = x0;
    tmpZ = z0;
    tmpC = c0;
    x0 = x1;
    z0 = z1;
    c0 = c1;
    x1 = tmpX;
    z1 = tmpZ;
    c1 = tmpC;
  }

  double delta_z;
  delta_z = (x1 - x0) != 0 ? (z1 - z0) / (x1 - x0 + 1) : 0;
  int x;
  double z = z0;

  cred = c0.red;
  cgreen = c0.green;
  cblue = c0.blue;

  dred = (x1 - x0 != 0) ? ((double)c1.red-(double)c0.red)/(x1-x0+1) : 0;
  dgreen = (x1 - x0 != 0) ? ((double)c1.green-(double)c0.green)/(x1-x0+1) : 0;
  dblue = (x1 - x0 != 0) ? ((double)c1.blue-(double)c0.blue)/(x1-x0+1) : 0;

  for(x=x0; x <= x1; x++) {

    cF.red = (unsigned short)cred;
    cF.green = (unsigned short)cgreen;
    cF.blue = (unsigned short)cblue;

    limit_color(&cF);
    plot(s, zb, cF, x, y, z);

    z+= delta_z;
    cred += dred;
    cgreen += dgreen;
    cblue += dblue;

  }
}

void draw_scanline_phong(int x0, double z0, int x1, double z1, int y, screen s, zbuffer zb, double* v0, double* v1,
  double* view, color ambient, struct constants* reflect) {

  int tmpX, tmpZ;
  double tmpv0[3];
  double tmpv1[3];
  double v[3];
  double vn[3];
  double vd[3];
  color c;
  set(tmpv0, v0);
  set(tmpv1, v1);
  if (x0 > x1) {
    tmpX = x0;
    tmpZ = z0;
    x0 = x1;
    z0 = z1;
    x1 = tmpX;
    z1 = tmpZ;
    set(tmpv1, v0);
    set(tmpv0, v1);
  }

  double delta_z;
  int distance = x1 - x0 + 1;
  delta_z = (x1 - x0) != 0 ? (z1 - z0) / distance : 0;
  int x;
  double z = z0;

  set(v,tmpv0);

  vd[0] = distance != 0 ? (tmpv1[0] - tmpv0[0]) / distance : 0;
  vd[1] = distance != 0 ? (tmpv1[1] - tmpv0[1]) / distance : 0;
  vd[2] = distance != 0 ? (tmpv1[2] - tmpv0[2]) / distance : 0;

  for(x=x0; x <= x1; x++) {

    set(vn, v);
    c = get_lighting(v, view, ambient, reflect);
    plot(s, zb, c, x, y, z);
    z+= delta_z;
    v[0] += vd[0];
    v[1] += vd[1];
    v[2] += vd[2];

  }
} */

/*======== void scanline_convert() ==========
  Inputs: struct matrix *points
          int i
          screen s
          zbuffer zb
          color il

  Fills in polygon i by drawing consecutive horizontal (or vertical) lines.
  ====================*/
void scanline_convert( struct matrix *points, int i, screen s, zbuffer zb, color il ) {

  int top, mid, bot, y;
  int distance0, distance1, distance2;
  double x0, x1, y0, y1, y2, dx0, dx1, z0, z1, dz0, dz1;
  int flip = 0;

  z0 = z1 = dz0 = dz1 = 0;

  y0 = points->m[1][i];
  y1 = points->m[1][i+1];
  y2 = points->m[1][i+2];

  //find bot, mid, top
  if ( y0 <= y1 && y0 <= y2) {
    bot = i;
    if (y1 <= y2) {
      mid = i+1;
      top = i+2;
    }
    else {
      mid = i+2;
      top = i+1;
    }
  }//end y0 bottom
  else if (y1 <= y0 && y1 <= y2) {
    bot = i+1;
    if (y0 <= y2) {
      mid = i;
      top = i+2;
    }
    else {
      mid = i+2;
      top = i;
    }
  }//end y1 bottom
  else {
    bot = i+2;
    if (y0 <= y1) {
      mid = i;
      top = i+1;
    }
    else {
      mid = i+1;
      top = i;
    }
  }//end y2 bottom

  x0 = points->m[0][bot];
  x1 = points->m[0][bot];
  z0 = points->m[2][bot];
  z1 = points->m[2][bot];
  y = (int)(points->m[1][bot]);

  distance0 = (int)(points->m[1][top]) - y + 1;
  distance1 = (int)(points->m[1][mid]) - y + 1;
  distance2 = (int)(points->m[1][top]) - (int)(points->m[1][mid]) + 1;

  //printf("distance0: %d distance1: %d distance2: %d\n", distance0, distance1, distance2);
  dx0 = distance0 > 0 ? (points->m[0][top]-points->m[0][bot])/distance0 : 0;
  dx1 = distance1 > 0 ? (points->m[0][mid]-points->m[0][bot])/distance1 : 0;
  dz0 = distance0 > 0 ? (points->m[2][top]-points->m[2][bot])/distance0 : 0;
  dz1 = distance1 > 0 ? (points->m[2][mid]-points->m[2][bot])/distance1 : 0;

  while ( y <= (int)points->m[1][top] ) {
    //printf("\tx0: %0.2f x1: %0.2f y: %d\n", x0, x1, y);

    if ( !flip && y >= (int)(points->m[1][mid]) ) {
      flip = 1;
      dx1 = distance2 > 0 ? (points->m[0][top]-points->m[0][mid])/distance2 : 0;
      dz1 = distance2 > 0 ? (points->m[2][top]-points->m[2][mid])/distance2 : 0;
      x1 = points->m[0][mid];
      z1 = points->m[2][mid];
    }//end flip code
    //draw_line(x0, y, z0, x1, y, z1, s, zb, il);
    draw_scanline(x0, z0, x1, z1, y, s, zb, il);

    x0+= dx0;
    x1+= dx1;
    z0+= dz0;
    z1+= dz1;
    y++;
  }//end scanline loop
}

/*======== void add_polygon() ==========
  Inputs:   struct matrix *polygons
            double x0
            double y0
            double z0
            double x1
            double y1
            double z1
            double x2
            double y2
            double z2
  Returns:
  Adds the vertices (x0, y0, z0), (x1, y1, z1)
  and (x2, y2, z2) to the polygon matrix. They
  define a single triangle surface.
  ====================*/
void add_polygon( struct matrix *polygons,
                  double x0, double y0, double z0,
                  double x1, double y1, double z1,
                  double x2, double y2, double z2 ) {
  add_point(polygons, x0, y0, z0);
  add_point(polygons, x1, y1, z1);
  add_point(polygons, x2, y2, z2);
}

/*======== void draw_polygons() ==========
  Inputs:   struct matrix *polygons
            screen s
            color c
  Returns:
  Goes through polygons 3 points at a time, drawing
  lines connecting each points to create bounding triangles
  ====================*/
void draw_polygons( struct matrix *polygons, screen s, zbuffer zb,
                    double *view, double light[2][3], color ambient,
                    struct constants *reflect, char shading[8]) {

  if ( polygons->lastcol < 3 ) {
    printf("Need at least 3 points to draw a polygon!\n");
    return;
  }

  int point;
  double *normal;
  //struct normals * nv = NULL;
  //char ver[256];

  //color meshC;
  //meshC.red = 255;
  //meshC.green = 100;
  //meshC.blue = 100;

  if(strcmp(shading, "gouraud") == 0) {
    struct normals * vn = NULL;
    for(point=0; point < polygons->lastcol; point++) {
      normal = calculate_normal(polygons, point);
      if (normal[2] > 0) {
        char v[256];
        sprintf(v, "%0.3lf, %0.3lf, %0.3lf",
              polygons->m[0][point],
              polygons->m[1][point],
              polygons->m[2][point]);
        struct normals * out;
        HASH_FIND_STR(vn, v, out);
        if(out == NULL) {
          struct normals * tmp;
          tmp = malloc(sizeof(struct normals));
          strcpy(tmp->vertex, v);
          tmp->normal[0] = normal[0];
          tmp->normal[1] = normal[1];
          tmp->normal[2] = normal[2];
          HASH_ADD_STR(vn, vertex, tmp);
        } else {
          out->normal[0] = (normal[0] + out->normal[0]) / 2;
          out->normal[1] = (normal[1] + out->normal[1]) / 2;
          out->normal[2] = (normal[2] + out->normal[2]) / 2;
        }
      }
    }
    for(point=0; point < polygons->lastcol-2; point+=3) {
      char v[256];
      struct normals * out;
      sprintf(v, "%0.3lf, %0.3lf, %0.3lf",
            polygons->m[0][point],
            polygons->m[1][point],
            polygons->m[2][point]);
      HASH_FIND_STR(vn, v, out);
      if(out != NULL) {
        if(out->normal[2] > 0) {
          double *N = (double *)malloc(3 * sizeof(double));
          N[0] = out->normal[0];
          N[1] = out->normal[1];
          N[2] = out->normal[2];
          color i = get_lighting(N, view, ambient, light, reflect);
        }
        // printf("%s %lf\n", out->vertex, out->normal[2]);
      }
    }
  }
  else if (strcmp(shading, "phong") == 0) {
  // struct vertex_normals * temp;
  // for(temp=vn; temp!=NULL; temp=temp->hh.next) {
  //   printf("%s: %lf %lf %lf\n",
  //     temp->vertex,
  //     temp->normal[0],
  //     temp->normal[1],
  //     temp->normal[2]);
  // }
  } 
  else {
    for (point=0; point < polygons->lastcol-2; point+=3) {
      normal = calculate_normal(polygons, point);
      if ( normal[2] > 0 ) {
        // get color value only if front facing
        color i = get_lighting(normal, view, ambient, light, reflect);
        scanline_convert(polygons, point, s, zb, i);
      }
    }
  }
}

/*======== void add_box() ==========
  Inputs:   struct matrix * edges
            double x
            double y
            double z
            double width
            double height
            double depth

  add the points for a rectagular prism whose
  upper-left-front corner is (x, y, z) with width,
  height and depth dimensions.
  ====================*/
void add_box( struct matrix *polygons,
              double x, double y, double z,
              double width, double height, double depth ) {
  double x0, y0, z0, x1, y1, z1;
  x0 = x;
  x1 = x+width;
  y0 = y;
  y1 = y-height;
  z0 = z;
  z1 = z-depth;


  //front
  add_polygon(polygons, x, y, z, x1, y1, z, x1, y, z);
  add_polygon(polygons, x, y, z, x, y1, z, x1, y1, z);
  //back
  add_polygon(polygons, x1, y, z1, x, y1, z1, x, y, z1);
  add_polygon(polygons, x1, y, z1, x1, y1, z1, x, y1, z1);

  //right side
  add_polygon(polygons, x1, y, z, x1, y1, z1, x1, y, z1);
  add_polygon(polygons, x1, y, z, x1, y1, z, x1, y1, z1);
  //left side
  add_polygon(polygons, x, y, z1, x, y1, z, x, y, z);
  add_polygon(polygons, x, y, z1, x, y1, z1, x, y1, z);

  //top
  add_polygon(polygons, x, y, z1, x1, y, z, x1, y, z1);
  add_polygon(polygons, x, y, z1, x, y, z, x1, y, z);
  //bottom
  add_polygon(polygons, x, y1, z, x1, y1, z1, x1, y1, z);
  add_polygon(polygons, x, y1, z, x, y1, z1, x1, y1, z1);
}

void parse_mesh(struct matrix * polygons, char * filename){
FILE *f;
  f = fopen(filename, "r");
  char line[256];
  char *token;
  struct matrix * v = new_matrix(4, 4); //list of all vertices
  if (f == NULL){ //exception
    printf("Could not open file: %s", filename);
    exit(0);
  }
  while (fgets(line, 256, f) != NULL){
    char *out = malloc(256);
    strcpy(out, line);
    // printf("%s", r);
    token = strsep(&out, " ");
    if (strncmp(token, "v", strlen(token)) == 0) {
      float x = atof(strsep(&out, " "));
      float y = atof(strsep(&out, " "));
      float z = atof(strsep(&out, " "));
      add_point(v, x, y, z);
    }
    else if (strncmp(token, "f", strlen(token)) == 0) {
      //find first token
      int first = atof(strsep(&out, " ")) - 1;

      //add triangles
      int i;
      int p1 = atof(strsep(&out, " ")) - 1;
      int p2 = -1;
      char * token;
      while ((token = strsep(&out, " ")) != NULL){
        p2 = atof(token) - 1;
        add_polygon(polygons, v->m[0][first], v->m[1][first], v->m[2][first],
                              v->m[0][p1], v->m[1][p1], v->m[2][p1],
                              v->m[0][p2], v->m[1][p2], v->m[2][p2]);
        p1 = p2;
      }
    }
  }
  fclose(f);
}


/*======== void add_sphere() ==========
  Inputs:   struct matrix * points
            double cx
            double cy
            double cz
            double r
            int step

  adds all the points for a sphere with center (cx, cy, cz)
  and radius r using step points per circle/semicircle.

  Since edges are drawn using 2 points, add each point twice,
  or add each point and then another point 1 pixel away.

  should call generate_sphere to create the necessary points
  ====================*/
void add_sphere( struct matrix * edges,
                 double cx, double cy, double cz,
                 double r, int step ) {

  struct matrix *points = generate_sphere(cx, cy, cz, r, step);
  int p0, p1, p2, p3, lat, longt;
  int latStop, longStop, latStart, longStart;
  latStart = 0;
  latStop = step;
  longStart = 1;
  longStop = step;

  //step++; needed for my triangles
  for ( lat = latStart; lat < latStop; lat++ ) {
    for ( longt = longStart; longt < longStop; longt++ ) {

      /*Milan's Triangles*/
      p0 = lat * (step+1) + longt;
      p1 = p0 + 1;
      p2 = (p1 + step) % (step * (step+1));
      p3 = (p0 + step) % (step * (step+1));

      add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p1],
                   points->m[1][p1],
                   points->m[2][p1],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2]);
      add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2],
                   points->m[0][p3],
                   points->m[1][p3],
                   points->m[2][p3]);

      /*My Triangles*/
      /* p0 = lat * (step) + longt; */
      /* p1 = p0+1; */
      /* p2 = (p1+step) % (step * (step-1)); */
      /* p3 = (p0+step) % (step * (step-1)); */

      /* //printf("p0: %d\tp1: %d\tp2: %d\tp3: %d\n", p0, p1, p2, p3); */
      /* if (longt < step - 2) */
      /*   add_polygon( edges, points->m[0][p0], */
      /*                points->m[1][p0], */
      /*                points->m[2][p0], */
      /*                points->m[0][p1], */
      /*                points->m[1][p1], */
      /*                points->m[2][p1], */
      /*                points->m[0][p2], */
      /*                points->m[1][p2], */
      /*                points->m[2][p2]); */
      /* if (longt > 0 ) */
      /*   add_polygon( edges, points->m[0][p0], */
      /*                points->m[1][p0], */
      /*                points->m[2][p0], */
      /*                points->m[0][p2], */
      /*                points->m[1][p2], */
      /*                points->m[2][p2], */
      /*                points->m[0][p3], */
      /*                points->m[1][p3], */
      /*                points->m[2][p3]); */
    }
  }
  free_matrix(points);
}

/*======== void generate_sphere() ==========
  Inputs:   struct matrix * points
            double cx
            double cy
            double cz
            double r
            int step
  Returns: Generates all the points along the surface
           of a sphere with center (cx, cy, cz) and
           radius r using step points per circle/semicircle.
           Returns a matrix of those points
  ====================*/
struct matrix * generate_sphere(double cx, double cy, double cz,
                                double r, int step ) {

  struct matrix *points = new_matrix(4, step * step);
  int circle, rotation, rot_start, rot_stop, circ_start, circ_stop;
  double x, y, z, rot, circ;

  rot_start = 0;
  rot_stop = step;
  circ_start = 0;
  circ_stop = step;

  for (rotation = rot_start; rotation < rot_stop; rotation++) {
    rot = (double)rotation / step;

    for(circle = circ_start; circle <= circ_stop; circle++){
      circ = (double)circle / step;

      x = r * cos(M_PI * circ) + cx;
      y = r * sin(M_PI * circ) *
        cos(2*M_PI * rot) + cy;
      z = r * sin(M_PI * circ) *
        sin(2*M_PI * rot) + cz;

      /* printf("rotation: %d\tcircle: %d\n", rotation, circle); */
      /* printf("rot: %lf\tcirc: %lf\n", rot, circ); */
      /* printf("sphere point: (%0.2f, %0.2f, %0.2f)\n\n", x, y, z); */
      add_point(points, x, y, z);
    }
  }

  return points;
}

/*======== void add_torus() ==========
  Inputs:   struct matrix * points
            double cx
            double cy
            double cz
            double r1
            double r2
            double step
  Returns:

  adds all the points required for a torus with center (cx, cy, cz),
  circle radius r1 and torus radius r2 using step points per circle.

  should call generate_torus to create the necessary points
  ====================*/
void add_torus( struct matrix * edges, 
                double cx, double cy, double cz,
                double r1, double r2, int step ) {

  struct matrix *points = generate_torus(cx, cy, cz, r1, r2, step);
  int p0, p1, p2, p3, lat, longt;
  int latStop, longStop, latStart, longStart;
  latStart = 0;
  latStop = step;
  longStart = 0;
  longStop = step;

  for ( lat = latStart; lat < latStop; lat++ ) {
    for ( longt = longStart; longt < longStop; longt++ ) {
      p0 = lat * step + longt;
      if (longt == step - 1)
        p1 = p0 - longt;
      else
        p1 = p0 + 1;
      p2 = (p1 + step) % (step * step);
      p3 = (p0 + step) % (step * step);

      //printf("p0: %d\tp1: %d\tp2: %d\tp3: %d\n", p0, p1, p2, p3);
      add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p3],
                   points->m[1][p3],
                   points->m[2][p3],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2]);
      add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2],
                   points->m[0][p1],
                   points->m[1][p1],
                   points->m[2][p1]);
    }
  }
  free_matrix(points);
}

/*======== void generate_torus() ==========
  Inputs:   struct matrix * points
            double cx
            double cy
            double cz
            double r
            int step
  Returns: Generates all the points along the surface
           of a torus with center (cx, cy, cz),
           circle radius r1 and torus radius r2 using
           step points per circle.
           Returns a matrix of those points
  ====================*/
struct matrix * generate_torus( double cx, double cy, double cz,
                                double r1, double r2, int step ) {

  struct matrix *points = new_matrix(4, step * step);
  int circles, rotation, rot_start, rot_stop, c_start, c_stop;
  double x, y, z, rot, circ;

  rot_start = 0;
  rot_stop = step;
  c_start = 0;
  c_stop = step;

  for (rotation = rot_start; rotation < rot_stop; rotation++) {
    rot = (double)rotation / step;

    for(circles = c_start; circles < c_stop; circles++){

      circ = (double) circles / step;

      x = cos(2 * M_PI * rot) * (r1 * cos(2*M_PI * circ) + r2) + cx;
      y = r1 * sin(2 * M_PI * circ) + cy;
      z = -1*sin(2 * M_PI * rot) * (r1 * cos(2*M_PI * circ) + r2) + cz;

      add_point(points, x, y, z);
    }
  }
  return points;
}

void add_cylinder( struct matrix* edges, double cx, double cy, double cz,
                    double r, double h, int step){

  struct matrix* points = generate_cylinder(cx,cy,cz,r,h,step);

  int longt, longtStart, longtStop;
  int p0, p1, p2, p3;

  double top[3], bot[3];

  top[0] = cx;
  top[1] = cy + (h / 2.0);
  top[2] = cz;

  bot[0] = cx;
  bot[1] = cy - (h / 2.0);
  bot[2] = cz;

  longtStart = 0;
  longtStop = step;

  for(int longt = longtStart; longt < longtStop; longt++){
    p0 = step + (longt % step);
    p1 = step + ((longt + 1) % step);
    p2 = (longt + 1) % step;
    p3 = longt % step;

    add_polygon(edges, points->m[0][p0], points->m[1][p0],points->m[2][p0],
      points->m[0][p1], points->m[1][p1],points->m[2][p1],
      points->m[0][p2], points->m[1][p2],points->m[2][p2]);
    add_polygon(edges, points->m[0][p0], points->m[1][p0],points->m[2][p0],
      points->m[0][p2], points->m[1][p2],points->m[2][p2],
      points->m[0][p3], points->m[1][p3],points->m[2][p3]);

    add_polygon(edges,top[0],top[1],top[2],
      points->m[0][p3], points->m[1][p3],points->m[2][p3],
      points->m[0][p2], points->m[1][p2],points->m[2][p2]);

    add_polygon(edges,bot[0],bot[1],bot[2],
      points->m[0][p1], points->m[1][p1],points->m[2][p1],
      points->m[0][p0], points->m[1][p0],points->m[2][p0]);

  }

  free_matrix(points);
}

/*======== void generate_cylinder() ==========
  Inputs:   struct matrix * points
            double cx
            double cy
            double cz
            double r
            double h
            int step
  Returns: Generates all the points along the surface
           of a cylinder (cx, cy, cz),
           circle radius r and height h
           step points per circle.
           Returns a matrix of those points
  ====================*/
struct matrix* generate_cylinder( double cx, double cy, double cz,
                                  double r, double h, int step){

  struct matrix* points = new_matrix(4, step*2);
  int rotate, rot_start, rot_stop;
  double x, y, z, rot;

  rot_start = 0;
  rot_stop = step;

  for(rotate = rot_start; rotate < rot_stop; rotate++){

    rot = (double)rotate / step;

    x = r * cos(2 * M_PI * rot) + cx;
    y = cy + (h / 2.0);
    z = r * sin(-2 * M_PI * rot) + cz;

    //printf("%lf %lf %lf\n",x,y,z);

    add_point(points,x,y,z);

  }

  for(rotate = rot_start; rotate < rot_stop; rotate++){

    rot = (double)rotate / step;

    x = r * cos(2 * M_PI * rot) + cx;
    y = cy - (h / 2.0);
    z = r * sin(-2 * M_PI * rot) + cz;

    add_point(points,x,y,z);

  }

  return points;

}

void add_cone( struct matrix* edges, double cx, double cy, double cz,
                    double r, double h, int step){
  struct matrix *bottom = generate_cone(cx, cy, cz, r, h, step);
  int longt, longtStart, longtStop;
  longtStart = 0;
  for (longt = longtStart; longt < step; longt++){
    longtStop = (longt + 1) % step;
    //bottom cap
    add_polygon(edges, cx, cy, cz,
                bottom->m[0][longt], bottom->m[1][longt], bottom->m[2][longt],
                bottom->m[0][longtStop], bottom->m[1][longtStop], bottom->m[2][longtStop]);
    //surface triangle
    add_polygon(edges, cx, cy + h, cz,
                bottom->m[0][longtStop], bottom->m[1][longtStop], bottom->m[2][longtStop],
                bottom->m[0][longt], bottom->m[1][longt], bottom->m[2][longt]);
  }

}

struct matrix* generate_cone( double cx, double cy, double cz,
                double r, double h, int step){
  struct matrix * points = new_matrix(4, step * 2);
  int rotate, rot_start, rot_stop;
  double x, z, rot;

  rot_start = 0;
  rot_stop = step;

  for(rotate = rot_start; rotate < rot_stop; rotate++){

    rot = (double) rotate / step;
    x = r * cos(2 * M_PI * rot) + cx;
    z = r * sin(2 * M_PI * rot) + cz;
    add_point(points,x,cy,z);

  }

  return points;
}




/*======== void add_circle() ==========
  Inputs:   struct matrix * edges
            double cx
            double cy
            double r
            double step

  Adds the circle at (cx, cy) with radius r to edges
  ====================*/
void add_circle( struct matrix *edges,
                 double cx, double cy, double cz,
                 double r, int step ) {
  double x0, y0, x1, y1, t;
  int i;

  x0 = r + cx;
  y0 = cy;
  for (i=1; i<=step; i++) {
    t = (double)i/step;
    x1 = r * cos(2 * M_PI * t) + cx;
    y1 = r * sin(2 * M_PI * t) + cy;

    add_edge(edges, x0, y0, cz, x1, y1, cz);
    x0 = x1;
    y0 = y1;
  }
}


/*======== void add_curve() ==========
Inputs:   struct matrix *edges
         double x0
         double y0
         double x1
         double y1
         double x2
         double y2
         double x3
         double y3
         double step
         int type

Adds the curve bounded by the 4 points passsed as parameters
of type specified in type (see matrix.h for curve type constants)
to the matrix edges
====================*/
void add_curve( struct matrix *edges,
                double x0, double y0, 
                double x1, double y1, 
                double x2, double y2, 
                double x3, double y3, 
                int step, int type ) {
  double t, x, y;
  int i;
  struct matrix *xcoefs;
  struct matrix *ycoefs;

  xcoefs = generate_curve_coefs(x0, x1, x2, x3, type);
  ycoefs = generate_curve_coefs(y0, y1, y2, y3, type);
  
  /* print_matrix(xcoefs); */
  /* printf("\n"); */
  /* print_matrix(ycoefs); */
  
  for (i=1; i<=step; i++) {
    t = (double)i/step;

    x = xcoefs->m[0][0] *t*t*t + xcoefs->m[1][0] *t*t+
      xcoefs->m[2][0] *t + xcoefs->m[3][0];
    y = ycoefs->m[0][0] *t*t*t + ycoefs->m[1][0] *t*t+
      ycoefs->m[2][0] *t + ycoefs->m[3][0];

    add_edge(edges, x0, y0, 0, x, y, 0);
    x0 = x;
    y0 = y;
  }

  free_matrix(xcoefs);
  free_matrix(ycoefs);
}



/*======== void add_point() ==========
Inputs:   struct matrix * points
         int x
         int y
         int z 
Returns: 
adds point (x, y, z) to points and increment points.lastcol
if points is full, should call grow on points
====================*/
void add_point( struct matrix * points, double x, double y, double z) {

  if ( points->lastcol == points->cols )
    grow_matrix( points, points->lastcol + 100 );
  
  points->m[0][ points->lastcol ] = x;
  points->m[1][ points->lastcol ] = y;
  points->m[2][ points->lastcol ] = z;
  points->m[3][ points->lastcol ] = 1;
  points->lastcol++;
} //end add_point

/*======== void add_edge() ==========
Inputs:   struct matrix * points
          int x0, int y0, int z0, int x1, int y1, int z1
Returns: 
add the line connecting (x0, y0, z0) to (x1, y1, z1) to points
should use add_point
====================*/
void add_edge( struct matrix * points, 
	       double x0, double y0, double z0, 
	       double x1, double y1, double z1) {
  add_point( points, x0, y0, z0 );
  add_point( points, x1, y1, z1 );
}

/*======== void draw_lines() ==========
Inputs:   struct matrix * points
         screen s
         color c 
Returns: 
Go through points 2 at a time and call draw_line to add that line
to the screen
====================*/
void draw_lines( struct matrix * points, screen s, zbuffer zb, color c) {

 if ( points->lastcol < 2 ) {
   printf("Need at least 2 points to draw a line!\n");
   return;
 }

 int point;
 for (point=0; point < points->lastcol-1; point+=2)
   draw_line( points->m[0][point],
              points->m[1][point],
              points->m[2][point],
              points->m[0][point+1],
              points->m[1][point+1],
              points->m[2][point+1],
              s, zb, c);
}// end draw_lines


void draw_line(int x0, int y0, double z0,
               int x1, int y1, double z1,
               screen s, zbuffer zb, color c) {

  int x, y, d, A, B;
  int dy_east, dy_northeast, dx_east, dx_northeast, d_east, d_northeast;
  int loop_start, loop_end;
  double distance;
  double z, dz;

  //swap points if going right -> left
  int xt, yt;
  if (x0 > x1) {
    xt = x0;
    yt = y0;
    x0 = x1;
    y0 = y1;
    z0 = z1;
    x1 = xt;
    y1 = yt;
  }

  x = x0;
  y = y0;
  A = 2 * (y1 - y0);
  B = -2 * (x1 - x0);
  int wide = 0;
  int tall = 0;
  //octants 1 and 8
  if ( abs(x1 - x0) >= abs(y1 - y0) ) { //octant 1/8
    wide = 1;
    loop_start = x;
    loop_end = x1;
    dx_east = dx_northeast = 1;
    dy_east = 0;
    d_east = A;
    distance = x1 - x + 1;
    if ( A > 0 ) { //octant 1
      d = A + B/2;
      dy_northeast = 1;
      d_northeast = A + B;
    }
    else { //octant 8
      d = A - B/2;
      dy_northeast = -1;
      d_northeast = A - B;
    }
  }//end octant 1/8
  else { //octant 2/7
    tall = 1;
    dx_east = 0;
    dx_northeast = 1;
    distance = abs(y1 - y) + 1;
    if ( A > 0 ) {     //octant 2
      d = A/2 + B;
      dy_east = dy_northeast = 1;
      d_northeast = A + B;
      d_east = B;
      loop_start = y;
      loop_end = y1;
    }
    else {     //octant 7
      d = A/2 - B;
      dy_east = dy_northeast = -1;
      d_northeast = A - B;
      d_east = -1 * B;
      loop_start = y1;
      loop_end = y;
    }
  }

  z = z0;
  dz = (z1 - z0) / distance;
  while ( loop_start < loop_end ) {

    plot( s, zb, c, x, y, z);
    if ( (wide && ((A > 0 && d > 0) ||
                   (A < 0 && d < 0)))
         ||
         (tall && ((A > 0 && d < 0 ) ||
                   (A < 0 && d > 0) ))) {
      y+= dy_northeast;
      d+= d_northeast;
      x+= dx_northeast;
    }
    else {
      x+= dx_east;
      y+= dy_east;
      d+= d_east;
    }
    z+= dz;
    loop_start++;
  } //end drawing loop
  plot( s, zb, c, x1, y1, z );
} //end draw_line
