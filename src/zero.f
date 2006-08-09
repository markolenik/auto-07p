C=======================================================================
C=======================================================================
C    Utility Program for "zeroing small mumbers" in an AUTO q.xxx file
C=======================================================================
C=======================================================================
C
      INCLUDE 'fcon.h'
      PARAMETER (NRX=NTSTX*NCOLX+1)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DIMENSION U(NRX,NDIMX),TM(NRX),RLDOT(NPARX),PAR(NPARX),ICP(NPARX)
      PARAMETER (EPS = 1d-16)
C
       L=0
       OPEN(28,FILE='fort.28',STATUS='old')
       OPEN(38,FILE='fort.38',STATUS='unknown')
 1     CONTINUE
         READ(28,*,END=99)IBR,NTOT,ITP,LAB,NFPR,ISW,NTPL,
     *               NAR,NROWPR,NTST,NCOL,NPAR
         IF(NTST.EQ.0)THEN
             WRITE(38,101)IBR,NTOT,ITP,LAB,NFPR,ISW,NTPL,
     *                         NAR,NROWPR,NTST,NCOL,NPAR
C            --------------------------------------------
             J=1
             READ(28,*) TM(J),(U(J,I),I=1,NAR-1) 
             DO I=1,NAR-1
		IF(ABS(U(J,I)).LT.EPS) U(J,I)=0.d0
             ENDDO
             WRITE(38,102)TM(J),(U(J,I),I=1,NAR-1)
C            --------------------------------------------
             READ(28,*) (PAR(I),I=1,NPAR)
             DO I=1,NPAR
		IF(ABS(PAR(I)).LT.EPS) PAR(I)=0.d0
             ENDDO
             WRITE(38,102)(PAR(I),I=1,NPAR)
C            --------------------------------------------
         ELSE
C            --------------------------------------------
             WRITE(38,101)IBR,NTOT,ITP,LAB,NFPR,ISW,NTPL,
     *                         NAR,NROWPR,NTST,NCOL,NPAR
             DO J=1,NTPL
               READ(28,*)   TM(J),(U(J,I),I=1,NAR-1) 
               DO I=1,NAR-1
		    IF(ABS(U(J,I)).LT.EPS) U(J,I)=0.d0
               ENDDO
               WRITE(38,102)TM(J),(U(J,I),I=1,NAR-1)
             ENDDO
C            --------------------------------------------
             READ(28,*)   (ICP(I),I=1,NFPR)
             WRITE(38,103)(ICP(I),I=1,NFPR)
C            --------------------------------------------
             READ(28,*)   (RLDOT(I),I=1,NFPR)
             DO I=1,NFPR
		IF(ABS(RLDOT(I)).LT.EPS) RLDOT(I)=0.d0
             ENDDO
             WRITE(38,102)(RLDOT(I),I=1,NFPR)
C            --------------------------------------------
             DO J=1,NTPL
               READ(28,*)   (U(J,I),I=1,NAR-1) 
               DO I=1,NAR-1
		    IF(ABS(U(J,I)).LT.EPS) U(J,I)=0.d0
               ENDDO
               WRITE(38,102)(U(J,I),I=1,NAR-1)
             ENDDO
C            --------------------------------------------
             READ(28,*)   (PAR(I),I=1,NPAR)
             DO I=1,NPAR
		IF(ABS(PAR(I)).LT.EPS) PAR(I)=0.d0
             ENDDO
             WRITE(38,102)(PAR(I),I=1,NPAR)
C            --------------------------------------------
           ENDIF
       GOTO 1
C
 101   FORMAT(6I6,I8,I6,I8,3I5)
 102   FORMAT(4X,1P7E19.10)
 103   FORMAT(20I5)
C
 99   STOP
      END
C=======================================================================
C=======================================================================