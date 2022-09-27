/**************************************************************************
 *
 * stack.cpp - Example program of stack. See Class Reference Section
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

#include <stack>
#include <vector>
#include <deque>
#include <string>
 
#ifdef _RW_STD_IOSTREAM
#include <iostream>
#else
#include <iostream.h>
#endif     

int main ()
{
#ifndef _RWSTD_NO_NAMESPACE
  using namespace std;
#endif

  //
  // Make a stack using a vector container.
  //
  stack<int,vector<int,allocator<int> > > s;
  //
  // Push a couple of values on the stack.
  //
  s.push(1);
  s.push(2);
  cout << s.top() << endl;
  //
  // Now pop them off.
  //
  s.pop();
  cout << s.top() << endl;
  s.pop();
  //
  // Make a stack of strings using a deque.
  //
  stack<string,deque<string,allocator<string> > > ss;
  //
  // Push a bunch of strings on then pop them off.
  //
  int i;
  for (i = 0; i < 10; i++)
  {
    ss.push(string(i+1,'a'));
    cout << ss.top() << endl;
  }
  for (i = 0; i < 10; i++)
  {
    cout << ss.top() << endl;
    ss.pop();
  }

  return 0;
}
