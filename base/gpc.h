#ifndef GPC_H
#define GPC_H

//#include <stdio.h>
#include "vector2d.h"

//===========================================================================
//                               Constants
//===========================================================================

// Increase GPC_EPSILON to encourage merging of near coincident edges    

#define GPC_EPSILON (DBL_EPSILON)

#define GPC_VERSION "2.31"

//===========================================================================
//                           Public Data Types
//===========================================================================

typedef enum                        /* Set operation type                */
{
  GPC_DIFF,                         /* Difference                        */
  GPC_INT,                          /* Intersection                      */
  GPC_XOR,                          /* Exclusive or                      */
  GPC_UNION                         /* Union                             */
} gpc_op;

typedef struct                      /* Polygon vertex structure          */
{
  double              x;            /* Vertex x component                */
  double              y;            /* vertex y component                */
} gpc_vertex;

typedef struct                      /* Vertex list structure             */
{
  int                 num_vertices; /* Number of vertices in list        */
  gpc_vertex         *vertex;       /* Vertex array pointer              */
} gpc_vertex_list;

typedef struct                      /* Polygon set structure             */
{
  int                 num_contours; /* Number of contours in polygon     */
  int                *hole;         /* Hole / external contour flags     */
  gpc_vertex_list    *contour;      /* Contour array pointer             */
} gpc_polygon;

typedef struct                      /* Tristrip set structure            */
{
  int                 num_strips;   /* Number of tristrips               */
  gpc_vertex_list    *strip;        /* Tristrip array pointer            */
} gpc_tristrip;

//===========================================================================
//                       Public Function Prototypes
//===========================================================================

void gpc_read_polygon        (FILE            *infile_ptr, 
                              int              read_hole_flags,
                              gpc_polygon     *polygon);

void gpc_read_polygon        (vector2dd *point_ptr,
							  int poly_count,//폴리곤 갯수
							  int* per_count,//각각 폴리곤의 점수
							  gpc_polygon *p);//입력될 폴리곤

void gpc_write_polygon       (FILE            *outfile_ptr,
                              int              write_hole_flags,
                              gpc_polygon     *polygon);

void gpc_write_polygon		 (vector2dd *point_ptr,
							  int* hole_flag,//홀폴리곤인지 아닌지 검사
							  int* per_count,//각각 폴리곤의 점수
							  gpc_polygon *p);

int gpc_point_count(gpc_polygon *p);
int gpc_polygon_count(gpc_polygon *p);

void gpc_add_contour         (gpc_polygon     *polygon,
                              gpc_vertex_list *contour,
                              int              hole);

void gpc_polygon_clip        (gpc_op           set_operation,
                              gpc_polygon     *subject_polygon,
                              gpc_polygon     *clip_polygon,
                              gpc_polygon     *result_polygon);

void gpc_tristrip_clip       (gpc_op           set_operation,
                              gpc_polygon     *subject_polygon,
                              gpc_polygon     *clip_polygon,
                              gpc_tristrip    *result_tristrip);

void gpc_polygon_to_tristrip (gpc_polygon     *polygon,
                              gpc_tristrip    *tristrip);

void gpc_free_polygon        (gpc_polygon     *polygon);

void gpc_free_tristrip       (gpc_tristrip    *tristrip);



#endif

