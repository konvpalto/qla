#define mult_c_h(rr, ri, a, b)			\
  mt1 = loadups(foff(&a,0));                    \
  mt2 = shufps(mt1, mt1, 0x0);                  \
  rr = mulps(mt2, b);				\
  mt3 = shufps(mt1, mt1, 0x55);                 \
  ri = mulps(mt3, b)
#define multadd_c_h(rr, ri, a, b)               \
  mt1 = loadups(foff(&a,0));                    \
  mt2 = shufps(mt1, mt1, 0x0);                  \
  rr = addps(rr, mulps(mt2, b));		\
  mt3 = shufps(mt1, mt1, 0x55);                 \
  ri = addps(ri, mulps(mt3, b))

#define mult_c0_h(rr, ri, b)			\
  mt2 = shufps(mt1, mt1, 0x0);                  \
  rr = mulps(mt2, b);				\
  mt3 = shufps(mt1, mt1, 0x55);                 \
  ri = mulps(mt3, b)
#define mult_c1_h(rr, ri, b)			 \
  mt2 = shufps(mt1, mt1, 0xaa);                  \
  rr = mulps(mt2, b);				 \
  mt3 = shufps(mt1, mt1, 0xff);			 \
  ri = mulps(mt3, b)
#define multadd_c0_h(rr, ri, b)               \
  mt2 = shufps(mt1, mt1, 0x0);                  \
  rr = addps(rr, mulps(mt2, b));		\
  mt3 = shufps(mt1, mt1, 0x55);                 \
  ri = addps(ri, mulps(mt3, b))
#define multadd_c1_h(rr, ri, b)               \
  mt2 = shufps(mt1, mt1, 0xaa);                  \
  rr = addps(rr, mulps(mt2, b));		\
  mt3 = shufps(mt1, mt1, 0xff);                 \
  ri = addps(ri, mulps(mt3, b))


#define NP 4
{
#pragma omp parallel for
  for(int i=0; i<n; i+=2) {
    v4sf mt1;
    {
      v4sf h1[3], h2[3];
      QLA_F3_DiracFermion *ri = &r[i];
      QLA_F3_ColorMatrix *ai = &a[i];
      QLA_F3_HalfFermion *bi = b[i];

      //prefetch(&r[i+NP]);
      //prefetchnt(&a[i+NP]);
      //prefetchnt(b[i+NP]);
      {
	int i_c;
	for(i_c=0; i_c<3; i_c++) {
	  h1[i_c] = loadaps(foff(bi, 4*i_c));
	}
      }
      {
	v4sf mti, mt2, mt3;
	mt1 = loadaps(&QLA_F3_elem_M(*ai,0,0));
	mult_c0_h(h2[0], mti, h1[0]);
	multadd_c1_h(h2[0], mti, h1[1]);
	mt1 = loadaps(&QLA_F3_elem_M(*ai,0,2));
	multadd_c0_h(h2[0], mti, h1[2]);
	mti = shufps(mti, mti, 0xb1);
	h2[0] = addsubps(h2[0],mti);
	mult_c1_h(h2[1], mti, h1[0]);
	mt1 = loadaps(&QLA_F3_elem_M(*ai,1,1));
	multadd_c0_h(h2[1], mti, h1[1]);
	multadd_c1_h(h2[1], mti, h1[2]);
	mti = shufps(mti, mti, 0xb1);
	h2[1] = addsubps(h2[1],mti);
	mt1 = loadaps(&QLA_F3_elem_M(*ai,2,0));
	mult_c0_h(h2[2], mti, h1[0]);
	multadd_c1_h(h2[2], mti, h1[1]);
	mt1 = loadaps(&QLA_F3_elem_M(*ai,2,2));
	multadd_c0_h(h2[2], mti, h1[2]);
	mti = shufps(mti, mti, 0xb1);
	h2[2] = addsubps(h2[2],mti);
      }
      {
	int i_c;
	for(i_c=0; i_c<3; i_c++) {
#ifdef GAMMA_5_PLUS
	  storeaps(foff(ri,8*i_c), addps(loadaps(foff(ri,8*i_c)), h2[i_c]));
#elif defined GAMMA_5_MINUS
	  storeaps(foff(ri,8*i_c+4), addps(loadaps(foff(ri,8*i_c+4)),h2[i_c]));
#else
	  v4sf x1;
	  storeaps(foff(ri,8*i_c), addps(loadaps(foff(ri,8*i_c)), h2[i_c]));
	  sprecon(SPR)(x1, h2[i_c]);
	  storeaps(foff(ri,8*i_c+4), addps(loadaps(foff(ri,8*i_c+4)), x1));
#endif
	}
      }
    }

    {
      v4sf h1[3], h2[3];
      QLA_F3_DiracFermion *ri = &r[i+1];
      QLA_F3_ColorMatrix *ai = &a[i+1];
      QLA_F3_HalfFermion *bi = b[i+1];

      //prefetch(&r[i+NP+1]);
      //prefetchnt(&a[i+NP+1]);
      //prefetchnt(b[i+NP]);
      {
	int i_c;
	for(i_c=0; i_c<3; i_c++) {
	  h1[i_c] = loadaps(foff(bi, 4*i_c));
	}
      }
      {
	v4sf mti, mt2, mt3;
	mult_c1_h(h2[0], mti, h1[0]);
	mt1 = loadaps(&QLA_F3_elem_M(*ai,0,1));
	multadd_c0_h(h2[0], mti, h1[1]);
	multadd_c1_h(h2[0], mti, h1[2]);
	mti = shufps(mti, mti, 0xb1);
	h2[0] = addsubps(h2[0],mti);
	mt1 = loadaps(&QLA_F3_elem_M(*ai,1,0));
	mult_c0_h(h2[1], mti, h1[0]);
	multadd_c1_h(h2[1], mti, h1[1]);
	mt1 = loadaps(&QLA_F3_elem_M(*ai,1,2));
	multadd_c0_h(h2[1], mti, h1[2]);
	mti = shufps(mti, mti, 0xb1);
	h2[1] = addsubps(h2[1],mti);
	mult_c1_h(h2[2], mti, h1[0]);
	mt1 = loadaps(&QLA_F3_elem_M(*ai,2,1));
	multadd_c0_h(h2[2], mti, h1[1]);
	multadd_c1_h(h2[2], mti, h1[2]);
	mti = shufps(mti, mti, 0xb1);
	h2[2] = addsubps(h2[2],mti);
      }
      {
	int i_c;
	for(i_c=0; i_c<3; i_c++) {
#ifdef GAMMA_5_PLUS
	  storeaps(foff(ri,8*i_c), addps(loadaps(foff(ri,8*i_c)), h2[i_c]));
#elif defined GAMMA_5_MINUS
	  storeaps(foff(ri,8*i_c+4), addps(loadaps(foff(ri,8*i_c+4)),h2[i_c]));
#else
	  v4sf x1;
	  storeaps(foff(ri,8*i_c), addps(loadaps(foff(ri,8*i_c)), h2[i_c]));
	  sprecon(SPR)(x1, h2[i_c]);
	  storeaps(foff(ri,8*i_c+4), addps(loadaps(foff(ri,8*i_c+4)), x1));
#endif
	}
      }
    }

  }
}
#undef mult_c_h
#undef multadd_c_h
#undef mult_c0_h
#undef mult_c1_h
#undef multadd_c0_h
#undef multadd_c1_h
#undef NP
