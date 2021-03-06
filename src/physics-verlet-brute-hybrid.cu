#include <cuda.h>

extern "C" {
#include "physics-verlet-brute-hybrid.h"
}

#define BLOCK_SIZE 512

static const value G = GRAVITATIONAL_CONSTANT;
static const value E = SOFTENING*SOFTENING;

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

  physics_cpu_swap();

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
  if (memory_loaded)
    return;

  cudaMemcpy(dpx, px, n*sizeof(value), cudaMemcpyHostToDevice);
  cudaMemcpy(dpy, py, n*sizeof(value), cudaMemcpyHostToDevice);

  cudaMemcpy(dvx, vx, n*sizeof(value), cudaMemcpyHostToDevice);
  cudaMemcpy(dvy, vy, n*sizeof(value), cudaMemcpyHostToDevice);

  cudaMemcpy(dm,  m,  n*sizeof(value), cudaMemcpyHostToDevice);

  memory_loaded = 1;
}

__device__
void physics_advance_positions_inner (value dt, int n,
				      const value * v,
				      const value * a0,
				      value * p) {
  int cpu_n = CPU_N;
  int i = blockIdx.x*blockDim.x + threadIdx.x + cpu_n;

  if (i >= n)
    return;

  p[i] += (v[i] + value_literal(0.5)*a0[i]*dt)*dt;
}

__global__
void physics_advance_positions (value dt, int n,
				const value *  vx, const value *  vy,
				const value * a0x, const value * a0y,
				value * px, value * py) {
  physics_advance_positions_inner(dt, n, vx, a0x, px);
  physics_advance_positions_inner(dt, n, vy, a0y, py);
}

__device__
value2 physics_calculate_forces_inner (int n, value2 pi, value3 pj, value2 ai) {
  value2 r;
  value  s;

  r.x = pj.x - pi.x;
  r.y = pj.y - pi.y;

  s = (r.x*r.x + r.y*r.y) + E;
  s = s*s*s;
  s = rsqrtv(s);

  s = G*s*pj.z;

  ai.x += r.x*s;
  ai.y += r.y*s;

  return ai;
}

__global__
void physics_calculate_forces (int n,
			       const value * px, const value * py,
			       const value * m,
			       value * a1x, value * a1y) {
  extern __shared__ value3 sp[];

  int cpu_n = CPU_N;
  int i = blockIdx.x*blockDim.x + threadIdx.x + cpu_n;
  int tile;

  sp[threadIdx.x].x = value_literal(0.0);
  sp[threadIdx.x].y = value_literal(0.0);
  sp[threadIdx.x].z = value_literal(0.0);

  if (i >= n)
    return;

  value2 pi = {px[i], py[i]};
  value2 ai = {value_literal(0.0), value_literal(0.0)};

  for (tile = 0; tile < (n + blockDim.x-1)/blockDim.x; tile++) {
    int tile_j = tile*blockDim.x + threadIdx.x;
    int j;

    if (tile_j < n) {
      sp[threadIdx.x].x = px[tile_j];
      sp[threadIdx.x].y = py[tile_j];
      sp[threadIdx.x].z =  m[tile_j];
    }
    __syncthreads();

#pragma unroll 64
    for (j = 0; j < blockDim.x; j++)
      ai = physics_calculate_forces_inner(n, pi, sp[j], ai);
    __syncthreads();
  }

  a1x[i] = ai.x;
  a1y[i] = ai.y;
}

__device__
void physics_advance_velocities_inner (value dt, int n,
				       const value * a0,
				       const value * a1,
				       value * v) {
  int cpu_n = CPU_N;
  int i = blockIdx.x*blockDim.x + threadIdx.x + cpu_n;

  if (i >= n)
    return;

  v[i] += value_literal(0.5)*(a0[i]+a1[i])*dt;
}

__global__
void physics_advance_velocities (value dt, int n,
				 const value * a0x, const value * a0y,
				 const value * a1x, const value * a1y,
				 value * vx, value * vy) {
  physics_advance_velocities_inner(dt, n, a0x, a1x, vx);
  physics_advance_velocities_inner(dt, n, a0y, a1y, vy);
}

void physics_advance (value dt, size_t n,
		      value * px, value * py,
		      value * vx, value * vy,
		      value * m) {
  size_t cpu_n = CPU_N;
  size_t gpu_n = n-cpu_n;

  int blockSize  = BLOCK_SIZE;
  int gridSize   = (gpu_n + blockSize-1)/blockSize;
  int sharedSize = blockSize*sizeof(value3);

#pragma omp master
  {
    physics_load_memory(n, px, py, vx, vy, m);  
    physics_advance_positions<<<gridSize, blockSize>>>(dt, n, dvx, dvy, a0x, a0y, dpx, dpy);
  }

  physics_cpu_advance_positions(dt, n, vx, vy, px, py);

#pragma omp master
  {
    cudaMemcpy(   &dpx[0],      &px[0], cpu_n*sizeof(value), cudaMemcpyHostToDevice);
    cudaMemcpy(   &dpy[0],      &py[0], cpu_n*sizeof(value), cudaMemcpyHostToDevice);
    cudaMemcpy(&px[cpu_n], &dpx[cpu_n], gpu_n*sizeof(value), cudaMemcpyDeviceToHost);
    cudaMemcpy(&py[cpu_n], &dpy[cpu_n], gpu_n*sizeof(value), cudaMemcpyDeviceToHost);

    physics_calculate_forces<<<gridSize, blockSize, sharedSize>>>(n, dpx, dpy, dm, a1x, a1y);
    physics_advance_velocities<<<gridSize, blockSize>>>(dt, n, a0x, a0y, a1x, a1y, dvx, dvy);
  }

  physics_cpu_calculate_forces(n, px, py, m);
  physics_cpu_advance_velocities(dt, n, vx, vy);

#pragma omp master
  {
    cudaMemcpy(&vx[cpu_n], &dvx[cpu_n], gpu_n*sizeof(value), cudaMemcpyDeviceToHost);
    cudaMemcpy(&vy[cpu_n], &dvy[cpu_n], gpu_n*sizeof(value), cudaMemcpyDeviceToHost);

    physics_swap();
  }
}

void physics_free (void) {
  physics_cpu_free();

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
  physics_cpu_init(n);

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
  physics_cpu_reset(n);

  cudaMemset(a0x, 0, n*sizeof(value));
  cudaMemset(a0y, 0, n*sizeof(value));
  cudaMemset(a1x, 0, n*sizeof(value));
  cudaMemset(a1y, 0, n*sizeof(value));

  memory_loaded = 0;
}
