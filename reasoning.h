/*****************************************************************
 *  Copyright (c) 2015, Xudong Liu.                              *
 *  All rights reserved.                                         *
 *****************************************************************/

/*
 * reasoning.h
 */


#ifndef REASONING_
#define REASONING_

/* #include "hpg_types.h" */
#include "types.h"
#include "ptree.h"
#include "formula_label.h"

/* given a ptree and an outcome, this function computes the
	 index of the leaf the outcome travels to. */
void
ptreeRepresentationComputeLeafIndex(
		char         			 *instancename,
		Ptree 		         *ptree,
		OutcomeVector       outcome,
		UInt32						 *ind);

#endif
