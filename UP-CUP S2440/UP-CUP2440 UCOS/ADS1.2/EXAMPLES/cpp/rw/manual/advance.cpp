/**************************************************************************
 *
 * advance.cpp - Example program for advancing iterators. 
 *               See Class Reference Section
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
#include <iterator>
#include <list>

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
   // Initialize a list using an array.
   //
   int arr[6] = {3,4,5,6,7,8};
   list<int,allocator<int> > l(arr+0, arr+6);
   //
   // Declare a list iterator, s.b. a ForwardIterator.
   //
   list<int,allocator<int> >::iterator itr = l.begin();
   //
   // Output the original list.
   //
   cout << "For the list: ";
   copy(l.begin(),l.end(),ostream_iterator<int,char,char_traits<char> >(cout," "));
   cout << endl << endl;
   cout << "When the iterator is initialized to l.begin()," 
        << endl << "it points to " << *itr << endl << endl;
   //
   // operator+ is not available for a ForwardIterator, so use advance.
   //
   advance(itr, 4);
   cout << "After advance(itr,4), the iterator points to " << *itr << endl;

   return 0;
}
