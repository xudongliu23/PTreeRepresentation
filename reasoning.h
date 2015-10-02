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
#include "formula_form.h"

/* given a ptree and an outcome, this function computes the
	 index of the leaf the outcome travels to. */
UInt32 
ptreeRepresentationComputeLeafIndex(
		Ptree *ptree,
		OutcomeVector outcome);

#endif
