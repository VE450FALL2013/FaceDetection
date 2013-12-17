#pragma OPENCL EXTENSION cl_khr_fp64: enable
typedef int sumtype;
typedef float sqsumtype;

typedef struct rect_t {
  int p0[2], p1[2], p2[2], p3[2];
  float weight;
} rect_t;

typedef struct CvHidHaarFeature
{
    int tilted;
    rect_t rect[3];
} CvHidHaarFeature;


typedef struct CvHidHaarTreeNode
{
    CvHidHaarFeature feature;
    float threshold;
    //int left;
    //int right;
} CvHidHaarTreeNode;


typedef struct CvHidHaarClassifier
{
    //int count;
    //CvHaarFeature* orig_feature;
    CvHidHaarTreeNode node;
    float alpha[2];
} CvHidHaarClassifier;


typedef struct CvHidHaarStageClassifier
{
    int  count;
    float threshold;
    CvHidHaarClassifier classifier[213];
    int two_rects;

} CvHidHaarStageClassifier;


typedef struct CvHidHaarClassifierCascade
{
    int  count;
    int  isStumpBased;
    int  has_tilted_features;
    int  is_tree;
    float inv_window_area;
    CvHidHaarStageClassifier stage_classifier[22];
    int p0[2], p1[2], p2[2], p3[2];

} CvHidHaarClassifierCascade;

float calc_sum2(rect_t rect, rect_t rect2, int col, int offset, int* mat) {

  float4 y0 = (float4)(rect.p0[0], rect.p1[0], rect.p2[0], rect.p3[0]);
  float4 x0 = (float4)(rect.p0[1], rect.p1[1], rect.p2[1], rect.p3[1]);
  float4 y2 = (float4)(rect2.p0[0], rect2.p1[0], rect2.p2[0], rect2.p3[0]);
  float4 x2 = (float4)(rect2.p0[1], rect2.p1[1], rect2.p2[1], rect2.p3[1]);
  float4 v0 = y0*(float4)col + x0 + (float4)offset;
  float4 v2 = y2*(float4)col + x2 + (float4)offset;

  float result = dot((float4)(mat[(int)v0.s0],-mat[(int)v0.s1],-mat[(int)v0.s2],mat[(int)v0.s3]), (float4)rect.weight) +
                 dot((float4)(mat[(int)v2.s0],-mat[(int)v2.s1],-mat[(int)v2.s2],mat[(int)v2.s3]), (float4)rect2.weight);
  return result;
}

/*
  rects: rectangles, rects[x][0] -- scale, rects[x][1] -- y, rects[x][2] -- x
  classifiers: classifier list that contains classifiers of all scales.
  sum_list: list of sum matrix that contains sum mat of all scales.
  sqsum_list: list of sum matrix that contains sum mat of all scales.
  tilted_list: list of sum matrix that contains sum mat of all scales.
  result: bool list of whether is face.
*/
__kernel void cascadesum1(__global int *rects, __global float *vnf,
                         __global CvHidHaarClassifierCascade *classifiers,
                         __global uchar *sum_list,
                         int mat_len, __global int *mat_header,
                         __global bool *result, int actual_rects,
                         __global int *actual_ids, int start_stage, int end_stage) {
  const int global_id = get_global_id(0);
  if(global_id >= actual_rects) return;
  const int id = actual_ids[global_id];
  float variance_norm_factor = vnf[id];
  int scale_num = rects[id * 3];
  int y = rects[id * 3 + 1];
  int x = rects[id * 3 + 2];
  __global CvHidHaarClassifierCascade* cascade = &classifiers[scale_num];
  int* sum_list_int = (int*)sum_list;

  float stage_sum;
  int p_offset;
  int i, j;
  int col = mat_header[1 + scale_num * 3];
  int mat_list_offset = mat_header[1 + scale_num * 3 + 2]/4;
  //official start
  p_offset = y * (col * sizeof(int) /sizeof(sumtype)) + x;
  for( i = start_stage; i < end_stage; i++ ) {
     stage_sum = 0;
     for( j = 0; j < cascade->stage_classifier[i].count; j++ ) {
         __global CvHidHaarClassifier* classifier = cascade->stage_classifier[i].classifier + j; //#j classifier in the #i stage
         CvHidHaarTreeNode node = classifier->node;
         float t = node.threshold*variance_norm_factor; //true threshold
         int* mat;
         mat = sum_list_int+mat_list_offset	;
         float sum = (float)calc_sum2(node.feature.rect[0], node.feature.rect[1], 
                              col, p_offset, mat);
         stage_sum += classifier->alpha[sum >= t];
         if( stage_sum > cascade->stage_classifier[i].threshold )
           break;
     }
     if( stage_sum < cascade->stage_classifier[i].threshold ) {
         result[id] = false;
         return;
     }
	else result[id] = true;
  }
  return;
}
