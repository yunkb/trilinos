/*****************************************************************************
 * CVS File Information :
 *    $RCSfile$
 *    $Author$
 *    $Date$
 *    $Revision$
 ****************************************************************************/
/***********************************************************************************************/
/* FILE  *************************         MPI_Abort.c        ***********************************/
/***********************************************************************************************/
/* Author : Lisa Alano June 24 2002                                                            */
/* Copyright (c) 2002 University of California Regents                                         */
/***********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mpi.h"

int MPI_Abort( MPI_Comm comm, int errorcode )
{
  _MPI_COVERAGE();
  return PMPI_Abort(comm, errorcode);
}
