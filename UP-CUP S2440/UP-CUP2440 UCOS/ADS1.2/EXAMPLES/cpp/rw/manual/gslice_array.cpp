/**************************************************************************
 *
 * gslice_array.cpp -- Generalized array slice examples 
 *                     See Class Reference Section
 *
 ***************************************************************************
 *
 * Copyright (c) 1994-1999 Rogue Wave Software, Inc.  All Rights Reserved.
 *
 * This computer software is owned by Rogue Wave Software, Inc. and is
 * protected by U.S. copyright laws and other laws and by international
 * treaties.  This computer software is furnished by Rogue Wave Software,
 * Inc. pursuant to a written license agreement and may be used, copied,
 * transmitted, and stored only in accordance with the terms of such
 * license and with the inclusion of the above copyright notice.  This
 * computer software or any other copies thereof may not be provided or
 * otherwise made available to any other person.
 *
 * U.S. Government Restricted Rights.  This computer software is provided
 * with Restricted Rights.  Use, duplication, or disclosure by the
 * Government is subject to restrictions as set forth in subparagraph (c)
 * (1) (ii) of The Rights in Technical Data and Computer Software clause
 * at DFARS 252.227-7013 or subparagraphs (c) (1) and (2) of the
 * Commercial Computer Software � Restricted Rights at 48 CFR 52.227-19,
 * as applicable.  Manufacturer is Rogue Wave Software, Inc., 5500
 * Flatiron Parkway, Boulder, Colorado 80301 USA.
 *
 **************************************************************************/

#include "valarray.h"  // Contains a valarray stream inserter

int main(void)
{
#ifndef _RWSTD_NO_NAMESPACE
  using namespace std;
#endif

  int ibuf[27] =   {0,1,2,3,4,5,6,7,8,1,2,3,4,5,6,7,8,9,2,3,4,5,6,7,8,9,10};  
  int buf13[9] = {13,13,13,13,13,13,13,13,13};
  size_t len_buf[3] = {3,3,3};
  size_t stride_buf[3] = {9,3,1};

  // create a valarray of ints
  valarray<int>  vi(ibuf,27);

  // print out the valarray
  cout << vi << endl;

  // Get a two dimensional diagonal slice out of the middle
  valarray<size_t> len2(2);
  len2[0] = 3;
  len2[1] = 3;
  valarray<size_t> stride2(2);
  stride2[0] = 3;
  stride2[1] = 10;
  gslice_array<int> gsl = vi[gslice(0,len2,stride2)];

  // print out the slice
  cout << gsl << endl;

  // Assign 13's to everything in the slice
  gsl = valarray<int>(buf13,9);

  // print out the slice and our original valarray
  cout << gsl << endl << vi <<  endl;

  return 0;
}

