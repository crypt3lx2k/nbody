#include <cuda.h>

extern "C" {
#include "physics.h"
}

#define value1 float1
#define value2 float2
#define value3 float3
#define value4 float4

#define rsqrtv(x) rsqrtf((x))

static int memory_loaded;

static value * a0x;
static value * a0y;

static value * a1x;
static value * a1y;

static value * dpx;
static value * dpy;

static value * dvx;
static value * dvy;

static value * dm;

static inline void physics_swap (void) {
  value * tx;
  value * ty;

  tx = a0x;
  a0x = a1x;
  a1x = tx;

  ty = a0y;
  a0y = a1y;
  a1y = ty;
}

static inline void physics_load_memory (size_t n,
					const value * px,
					const value * py,
					const value * vx,
					const value * vy,
					const value * m) {
  cudaMemcpy(dpx, px, n*sizeof(value), cudaMemcpyHostToDevice);
  cudaMemcpy(dpy, py, n*sizeof(value), cudaMemcpyHostToDevice);

  cudaMemcpy(dvx, vx, n*sizeof(value), cudaMemcpyHostToDevice);
  cudaMemcpy(dvy, vy, n*sizeof(value), cudaMemcpyHostToDevice);

  cudaMemcpy(dm,  m,  n*sizeof(value), cudaMemcpyHostToDevice);

  memory_loaded = 1;
}

static inline void physics_offload_memory (size_t n,
					   value * px, value * py,
					   value * vx, value * vy) {
  cudaMemcpy(px, dpx, n*sizeof(value), cudaMemcpyDeviceToHost);
  cudaMemcpy(py, dpy, n*sizeof(value), cudaMemcpyDeviceToHost);

  cudaMemcpy(vx, dvx, n*sizeof(value), cudaMemcpyDeviceToHost);
  cudaMemcpy(vy, dvy, n*sizeof(value), cudaMemcpyDeviceToHost);
}

__device__
void physics_advance_update_position (value dt, int n,
				      const value * v,
				      const value * a0,
				      value * p) {
  int i = blockIdx.x*blockDim.x + threadIdx.x;

  if (i >= n)
    return;

  p[i] += (v[i] + value_literal(0.5)*a0[i]*dt)*dt;
}

__device__
value2 physics_advance_calculate_inner (value2 pi,
					value3 pj,
					value2 ai) {
  value2 r;
  value s;

  r.x = pj.x - pi.x;
  r.y = pj.y - pi.y;

  s = (r.x*r.x + r.y*r.y) + SOFTENING*SOFTENING;
  s = s*s*s;
  s = rsqrtv(s);

  s *= GRAVITATIONAL_CONSTANT*pj.z;

  ai.x += r.x*s;
  ai.y += r.y*s;

  return ai;
}

__device__
void physics_advance_calculate_forces (int n,
				       const value * px,
				       const value * py,
				       const value * m,
				       value * a1x,
				       value * a1y) {
  extern __shared__ value3 shared_storage[];

  int i = blockIdx.x*blockDim.x + threadIdx.x;
  int tile;

  if (i >= n)
    return;

  value2 ai = {value_literal(0.0), value_literal(0.0)};
  value2 pi;

  pi.x = px[i];
  pi.y = py[i];

  for (tile = 0; tile*blockDim.x < n; tile++) {
    int j;
    int tile_j = tile*blockDim.x + threadIdx.x;

    if (tile_j < n) {
      shared_storage[threadIdx.x].x = px[tile_j];
      shared_storage[threadIdx.x].y = py[tile_j];
      shared_storage[threadIdx.x].z = m[tile_j];
    }
    __syncthreads();

#pragma unroll 64
    for (j = 0; j < blockDim.x; j++) {
      ai = physics_advance_calculate_inner(pi, shared_storage[j], ai);
    }
    __syncthreads();
  }

  a1x[i] = ai.x;
  a1y[i] = ai.y;
}

__device__
void physics_advance_update_velocity (value dt, int n,
				      const value * a0,
				      const value * a1,
				      value * v) {
  int i = blockIdx.x*blockDim.x + threadIdx.x;

  if (i >= n)
    return;

  v[i] += value_literal(0.5)*(a0[i]+a1[i])*dt;
}

__global__
void physics_advance_kernel (value dt, int n,
			     value * px, value * py,
			     value * vx, value * vy,
			     value * m,
			     value * a0x, value * a0y,
			     value * a1x, value * a1y) {
  physics_advance_update_position(dt, n, vx, a0x, px);
  physics_advance_update_position(dt, n, vy, a0y, py);
  __syncthreads();

  physics_advance_calculate_forces(n, px, py, m, a1x, a1y);
  __syncthreads();

  physics_advance_update_velocity(dt, n, a0x, a1x, vx);
  physics_advance_update_velocity(dt, n, a0y, a1y, vy);
}

void physics_advance (value dt, size_t n,
		      value * px, value * py,
		      value * vx, value * vy,
		      value * m) {
  int blockSize  = 512;
  int gridSize   = (n + blockSize-1)/blockSize;
  int sharedSize = blockSize*3*sizeof(value);

  if (!memory_loaded)
    physics_load_memory(n, px, py, vx, vy, m);

  physics_advance_kernel<<<gridSize, blockSize, sharedSize>>>(dt, n,
							      dpx, dpy,
							      dvx, dvy,
							      dm,
							      a0x, a0y,
							      a1x, a1y);
  physics_swap();

  physics_offload_memory(n, px, py, vx, vy);
}

void physics_free (void) {
  cudaFree(a0x);
  cudaFree(a0y);

  cudaFree(a1x);
  cudaFree(a1y);

  cudaFree(dpx);
  cudaFree(dpy);

  cudaFree(dvx);
  cudaFree(dvy);

  cudaFree(dm);

  cudaDeviceReset();
}

void physics_init (size_t n) {
  cudaMalloc(&a0x, n*sizeof(value));
  cudaMalloc(&a0y, n*sizeof(value));

  cudaMalloc(&a1x, n*sizeof(value));
  cudaMalloc(&a1y, n*sizeof(value));

  cudaMalloc(&dpx, n*sizeof(value));
  cudaMalloc(&dpy, n*sizeof(value));

  cudaMalloc(&dvx, n*sizeof(value));
  cudaMalloc(&dvy, n*sizeof(value));

  cudaMalloc(&dm, n*sizeof(value));
}

void physics_reset (size_t n) {
  cudaMemset(a0x, 0, n*sizeof(value));
  cudaMemset(a0y, 0, n*sizeof(value));
  cudaMemset(a1x, 0, n*sizeof(value));
  cudaMemset(a1y, 0, n*sizeof(value));

  memory_loaded = 0;
}
