C Copyright(C) 2009 Sandia Corporation. Under the terms of Contract
C DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
C certain rights in this software.
C         
C Redistribution and use in source and binary forms, with or without
C modification, are permitted provided that the following conditions are
C met:
C 
C     * Redistributions of source code must retain the above copyright
C       notice, this list of conditions and the following disclaimer.
C 
C     * Redistributions in binary form must reproduce the above
C       copyright notice, this list of conditions and the following
C       disclaimer in the documentation and/or other materials provided
C       with the distribution.
C     * Neither the name of Sandia Corporation nor the names of its
C       contributors may be used to endorse or promote products derived
C       from this software without specific prior written permission.
C 
C THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
C "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
C LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
C A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
C OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
C SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
C LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
C DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
C THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
C (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
C OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

C=======================================================================
      SUBROUTINE PRMAP (OPTION, NOUT, NUMEL, MAPEL, TYPE)
C=======================================================================

C   --*** PRMAP *** (BLOT) Display database node/element number map
C   --
C   --PRMAP displays the node/element order map.
C   --
C   --Parameters:
C   --   OPTION - IN - '*' to print all, else print options:
C   --   NOUT - IN - the output file, <=0 for standard
C   --   NUMEL - IN - the number of node/elements
C   --   MAPEL - IN - the node/element order map

      CHARACTER*(*) OPTION
      INTEGER MAPEL(*)
      CHARACTER*(*) TYPE

      LOGICAL ISABRT
      CHARACTER*4 FMT
      CHARACTER*32 STRA, STRB

      LOGICAL MAPONE

      IF (NOUT .GT. 0) WRITE (NOUT, 10000) type(:lenstr(type))

      IF (NOUT .GT. 0) THEN
         WRITE (NOUT, 10010) type(:lenstr(type))
      ELSE
         WRITE (*, 10010) type(:lenstr(type))
      END IF

      WRITE (STRA, '(I11)', IOSTAT=IDUM) NUMEL
      CALL SQZSTR (STRA, LSTRA)
      WRITE (FMT, '(''(I'', I1, '')'')', IOSTAT=IDUM) LSTRA

C ... Check for 1-1 mapping
      mapone = .TRUE.
      do i = 1, numel
        if (mapel(i) .ne. i) then
          mapone = .FALSE.
          go to 90
        end if
      end do
 90   continue
      if (mapone) then
        IF (NOUT .GT. 0) THEN
          write (nout, 10040) type(:lenstr(type))
        ELSE
          write (*, 10040) type(:lenstr(type))
        END IF
      else
      DO 100 IEL = 1, NUMEL, 8
         IF (ISABRT ()) RETURN
         NE = MIN (IEL+7, NUMEL)
         WRITE (STRA, FMT, IOSTAT=IDUM) IEL
         WRITE (STRB, FMT, IOSTAT=IDUM) NE
         IF (NOUT .GT. 0) THEN
            WRITE (NOUT, 10020, IOSTAT=IDUM)
     &         STRA(:LSTRA), STRB(:LSTRA), (MAPEL(N), N=IEL,NE)
         ELSE
            WRITE (*, 10020, IOSTAT=IDUM)
     &         STRA(:LSTRA), STRB(:LSTRA), (MAPEL(N), N=IEL,NE)
         END IF
  100 CONTINUE
      end if
      RETURN

10000  FORMAT (/, 1X, A, ' NUMBER MAP')
10010  FORMAT (/, 1X, A, ' Number Map:')
10020  FORMAT (1X, 3X, A, '..', A, 3X, 8I11)
10040  format (1x, 3x, 'Map does not modify local ',
     &   A, ' ids (X maps to X)')
      END
