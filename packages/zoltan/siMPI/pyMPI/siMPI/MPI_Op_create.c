/*****************************************************************************
 * CVS File Information :
 *    $RCSfile$
 *    $Author$
 *    $Date$
 *    $Revision$
 ****************************************************************************/
/****************************************************************************/
/* FILE  ******************     MPI_Op_create.c      ************************/
/****************************************************************************/
/* Author : Lisa Alano July 8 2002                                          */
/* Copyright (c) 2002 University of California Regents                      */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

/*==========================================================================*/
int MPI_Op_create( MPI_User_function* function, int commute, MPI_Op* op)
{
  _MPI_COVERAGE();
  return PMPI_Op_create(function, commute, op);
}
/*==========================================================================*/
