/*****************************************************************
 *  Copyright (c) 2015, Xudong Liu.                              *
 *  All rights reserved.                                         *
 *****************************************************************/

/*
 * reasoning.c
 */

#include "reasoning.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

UInt32
ptreeRepresentationComputeLeafIndex(
    Ptree *ptree, 
    OutcomeVector outcome) {
	if (ptree == NULL) {
		return 0;
	}

	switch (ptree->type) {
		case: ePreferencePtreeTypeRoot0:
			break;
    case ePreferencePtreeTypeRoot2:                               
      ptree_free(p->lst);                                         
      ptree_free(p->rst);                                         
      myfree(p->lst);
      myfree(p->rst);
      break;                                                      
    case ePreferencePtreeTypeRoot1l:
      ptree_free(p->lst);
      myfree(p->lst);                                             
      break;
    case ePreferencePtreeTypeRoot1m:                              
      ptree_free(p->mst);                                         
      myfree(p->mst);
      break;
    case ePreferencePtreeTypeRoot1r:                              
      ptree_free(p->rst);                                         
      myfree(p->rst);
      break;
    default:                                                      
      /* true, false: nothing to do */
      break;                                                      
    }
	}
}

