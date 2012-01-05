#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <qla.h>
#ifdef _OPENMP
# include <omp.h>
# define __USE_GNU
# include <sched.h>
#endif

#if QLA_Precision == 'F'
#define REALBYTES 4
#else
#define REALBYTES 8
#endif

#define myalloc(type, n) (type *) aligned_malloc(n*sizeof(type))
#define clock qtime

#define ALIGN 32
void *
aligned_malloc(size_t n)
{
  size_t m = (size_t) malloc(n+ALIGN);
  size_t r = m % ALIGN;
  if(r) m += (ALIGN - r);
  return (void *)m;
}

double
qtime(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return CLOCKS_PER_SEC*(tv.tv_sec + 1e-6*tv.tv_usec);
}

void
set_V(QLA_ColorVector *v, int i)
{
  int j;
  for(j=0; j<QLA_Nc; j++) {
    QLA_c_eq_r_plus_ir(QLA_elem_V(*v,j), j+1+cos(i), j+1+sin(i));
    //QLA_real(QLA_elem_V(*v,j)) = 1;
    //QLA_imag(QLA_elem_V(*v,j)) = 0;
  }
}

void
set_H(QLA_HalfFermion *h, int i)
{
  int j, k;
  for(j=0; j<QLA_Nc; j++) {
    for(k=0; k<(QLA_Ns/2); k++) {
      QLA_c_eq_r_plus_ir(QLA_elem_H(*h,j,k), (j+4)*(k+1)+cos(i), (j+4)*(k+1)+sin(i));
    }
  }
}

void
set_D(QLA_DiracFermion *d, int i)
{
  int j, k;
  for(j=0; j<QLA_Nc; j++) {
    for(k=0; k<QLA_Ns; k++) {
      QLA_c_eq_r_plus_ir(QLA_elem_D(*d,j,k), (j+4)*(k+1)+cos(i), (j+4)*(k+1)+sin(i));
    }
  }
}

void
set_M(QLA_ColorMatrix *m, int i)
{
  int j, k;
  for(j=0; j<QLA_Nc; j++) {
    for(k=0; k<QLA_Nc; k++) {
      QLA_c_eq_r_plus_ir(QLA_elem_M(*m,j,k), (j+4)*(k+1)+cos(i), (j+4)*(k+1)+sin(i));
      //QLA_real(QLA_elem_M(*m,j,k)) = 1;
      //QLA_imag(QLA_elem_M(*m,j,k)) = 0;
    }
  }
}

QLA_Real
sum_V(QLA_ColorVector *d, int n)
{
  QLA_Real t=0, *r=(QLA_Real *)d;
  int nn = n*sizeof(QLA_ColorVector)/sizeof(QLA_Real);
  int i;
  for(i=0; i<nn; i++) t += r[i];
  return t/nn;
}

QLA_Real
sum_H(QLA_HalfFermion *d, int n)
{
  QLA_Real t=0, *r=(QLA_Real *)d;
  int nn = n*sizeof(QLA_HalfFermion)/sizeof(QLA_Real);
  int i;
  for(i=0; i<nn; i++) t += r[i];
  return t/nn;
}

QLA_Real
sum_D(QLA_DiracFermion *d, int n)
{
  QLA_Real t=0, *r=(QLA_Real *)d;
  int nn = n*sizeof(QLA_DiracFermion)/sizeof(QLA_Real);
  int i;
  for(i=0; i<nn; i++) t += r[i];
  return t/nn;
}

QLA_Real
sum_M(QLA_ColorMatrix *d, int n)
{
  QLA_Real t=0, *r=(QLA_Real *)d;
  int nn = n*sizeof(QLA_ColorMatrix)/sizeof(QLA_Real);
  int i;
  for(i=0; i<nn; i++) t += r[i];
  return t/nn;
}

#define set_fields { \
  r1 = 1.23; \
  _Pragma("omp parallel for") \
  for(i=0; i<n; ++i) { \
    set_V(&v1[i], i); \
    set_V(&v2[i], i); \
    set_V(&v3[i], i); \
    set_V(&v4[i], i); \
    set_V(&v5[i], i); \
    set_H(&h1[i], i); \
    set_H(&h2[i], i); \
    set_D(&d1[i], i); \
    set_D(&d2[i], i); \
    set_M(&m1[i], i); \
    set_M(&m2[i], i); \
    set_M(&m3[i], i); \
    set_M(&m4[i], i); \
    j = ((i|16)+256) % n; \
    vp1[i] = &v2[j]; \
    vp2[i] = &v3[j]; \
    vp3[i] = &v4[j]; \
    vp4[i] = &v5[j]; \
    hp1[i] = &h2[j]; \
    dp1[i] = &d2[j]; \
    mp1[i] = &m3[j]; \
  } \
}

int
main(int argc, char *argv[])
{
  QLA_Complex c1;
  QLA_Real r1, sum;
  QLA_ColorMatrix *m1, *m2, *m3, *m4, **mp1;
  QLA_ColorVector *v1, *v2, *v3, *v4, *v5;
  QLA_ColorVector **vp1, **vp2, **vp3, **vp4;
  QLA_HalfFermion *h1, *h2, **hp1;
  QLA_DiracFermion *d1, *d2, **dp1;
  double time1;
  float cf, flop, mem;
  int i, j, n, c;

  n = 4000;
  if(argc>1) n = atoi(argv[1]);
  cf = 9.e9/n;
  if(argc>2) cf = atof(argv[2]);

  m1 = myalloc(QLA_ColorMatrix, n);
  m2 = myalloc(QLA_ColorMatrix, n);
  m3 = myalloc(QLA_ColorMatrix, n);
  m4 = myalloc(QLA_ColorMatrix, n);
  mp1 = myalloc(QLA_ColorMatrix *, n);
  v1 = myalloc(QLA_ColorVector, n);
  v2 = myalloc(QLA_ColorVector, n);
  v3 = myalloc(QLA_ColorVector, n);
  v4 = myalloc(QLA_ColorVector, n);
  v5 = myalloc(QLA_ColorVector, n);
  vp1 = myalloc(QLA_ColorVector *, n);
  vp2 = myalloc(QLA_ColorVector *, n);
  vp3 = myalloc(QLA_ColorVector *, n);
  vp4 = myalloc(QLA_ColorVector *, n);
  h1 = myalloc(QLA_HalfFermion, n);
  h2 = myalloc(QLA_HalfFermion, n);
  hp1 = myalloc(QLA_HalfFermion *, n);
  d1 = myalloc(QLA_DiracFermion, n);
  d2 = myalloc(QLA_DiracFermion, n);
  dp1 = myalloc(QLA_DiracFermion *, n);
  //QLA_ColorMatrix *ma[4] = { m1, m2, m3, m4 };
  //QLA_ColorVector *va[4] = { v2, v3, v4, v5 };
  //QLA_ColorVector **vpa[4] = { vp1, vp2, vp3, vp4 };

  printf("QLA version %s (%i)\n", QLA_version_str(), QLA_version_int());
  printf("len = %i\n", n);

#ifdef _OPENMP
  printf("OMP THREADS = %i\n", omp_get_max_threads());
#pragma omp parallel
  {
    int tid = omp_get_thread_num();
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(tid, &set);
    sched_setaffinity(0, sizeof(set), &set);
  }
#endif

  set_fields;
  mem = 16*QLA_Nc*REALBYTES;
  flop = 8*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_D_veq_r_times_D(d1, &r1, d2, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = sum_D(d1, n);
  printf("%-32s:", "QLA_D_veq_r_times_D");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

  set_fields;
  mem = 2*(2+QLA_Nc)*QLA_Nc*REALBYTES;
  flop = 8*QLA_Nc*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_V_vpeq_M_times_pV(v1, m1, vp1, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = sum_V(v1, n);
  printf("%-32s:", "QLA_V_vpeq_M_times_pV");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

#if 0
#if QLA_Precision == 'D' && QLA_Colors == 3
  extern void
    QLA_D3_V_vpeq_nM_times_npV(QLA_D3_ColorVector *restrict r,
			       QLA_D3_ColorMatrix *restrict *a,
			       QLA_D3_ColorVector *restrict **b,
			       int n,
			       int nd);
  set_fields;
  mem = 4*2*(2+QLA_Nc)*QLA_Nc*REALBYTES;
  flop = 4*8*QLA_Nc*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_D3_V_vpeq_nM_times_npV(v1, ma, vpa, n, 4);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = sum_V(v1, n);
  printf("%-32s:", "QLA_V_vpeq_nM_times_npV");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));
#endif
#endif

  set_fields;
  mem = 2*(2+QLA_Nc)*QLA_Nc*REALBYTES;
  flop = (8*QLA_Nc-2)*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_V_veq_Ma_times_V(v1, m1, v2, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = sum_V(v1, n);
  printf("%-32s:", "QLA_V_veq_Ma_times_V");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

  set_fields;
  mem = 4*QLA_Nc*REALBYTES;
  flop = 2*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_V_vmeq_pV(v1, vp1, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = sum_V(v1, n);
  printf("%-32s:", "QLA_V_vmeq_pV");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

  set_fields;
  mem = 12*QLA_Nc*REALBYTES;
  flop = 4*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_H_veq_spproj_D(h1, d1, 0, 1, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = sum_H(h1, n);
  printf("%-32s:", "QLA_H_veq_spproj_D");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

  set_fields;
  mem = 2*(6+QLA_Nc)*QLA_Nc*REALBYTES;
  flop = (16*QLA_Nc+4)*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_D_vpeq_sprecon_M_times_pH(d1, m1, hp1, 0, 1, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = sum_D(d1, n);
  printf("%-32s:", "QLA_D_vpeq_sprecon_M_times_pH");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

  set_fields;
  mem = 2*(8+QLA_Nc)*QLA_Nc*REALBYTES;
  flop = (16*QLA_Nc+8)*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_D_vpeq_spproj_M_times_pD(d1, m1, dp1, 0, 1, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = sum_D(d1, n);
  printf("%-32s:", "QLA_D_vpeq_spproj_M_times_pD");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

  set_fields;
  //mem = 16*QLA_Nc*REALBYTES;
  //flop = 12*QLA_Nc;
  mem = 8*QLA_Nc*REALBYTES; // for gamma5
  flop = 4*QLA_Nc; // for gamma5
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_D_vpeq_spproj_D(d1, d2, 4, 1, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = sum_D(d1, n);
  printf("%-32s:", "QLA_D_vpeq_spproj_D");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

  set_fields;
  mem = 6*QLA_Nc*QLA_Nc*REALBYTES;
  flop = (8*QLA_Nc-2)*QLA_Nc*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_M_veq_M_times_pM(m1, m2, mp1, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = sum_M(m1, n);
  printf("%-32s:", "QLA_M_veq_M_times_pM");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

  set_fields;
  mem = 2*QLA_Nc*REALBYTES;
  flop = 4*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_r_veq_norm2_V(&r1, v1, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = r1;
  printf("%-32s:", "QLA_r_veq_norm2_V");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

  set_fields;
  mem = 4*QLA_Nc*REALBYTES;
  flop = 8*QLA_Nc;
  c = cf/(flop+mem);
  time1 = clock();
  for(i=0; i<c; ++i) {
    QLA_c_veq_V_dot_V(&c1, v1, v2, n);
  }
  time1 = clock() - time1;
  time1 /= CLOCKS_PER_SEC;
  sum = QLA_norm2_c(c1);
  printf("%-32s:", "QLA_c_veq_V_dot_V");
  printf("%12g time=%5.2f mem=%5.0f mflops=%5.0f\n", sum, time1, mem*n*c/(1e6*time1), flop*n*c/(1e6*time1));

  return 0;
}
