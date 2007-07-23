C======================================================================
C======================================================================
C Utility Module for Listing, Deleting, Relabeling Labeled Solutions
C                        in AUTO-07p Data Files
C======================================================================
C======================================================================
C
      MODULE UTILITY

      PRIVATE
      PUBLIC KEEPMAIN,RDFILE,LISTLB,INLIST,WRFILE7,WRFILE8,RELABEL

      CONTAINS

C     ---------- --------
      SUBROUTINE KEEPMAIN(DELETEFN)
      PARAMETER (MXLB=10000)
      CHARACTER*1 CMD
      DIMENSION LBR(MXLB),LPT(MXLB),LTY(MXLB),LLB(MXLB),LNL(MXLB)
      DIMENSION LFR(MXLB),LTO(MXLB)
      LOGICAL CHCKLB
      LOGICAL, EXTERNAL :: DELETEFN
C
       OPEN(27,FILE='fort.27',STATUS='old',ACCESS='sequential')
       OPEN(28,FILE='fort.28',STATUS='old',ACCESS='sequential')
       OPEN(37,FILE='fort.37',STATUS='unknown',ACCESS='sequential')
       OPEN(38,FILE='fort.38',STATUS='unknown',ACCESS='sequential')
C
       CALL RDFILE(MXLB,NLB,LBR,LPT,LTY,LLB,LNL)
       CALL KEEP(MXLB,NLB,LBR,LPT,LTY,LLB,LNL,0,LFR,LTO,DELETEFN)
       CALL WRFILE7(MXLB,NLB,LBR,LPT,LTY,LLB,LNL)
       CALL WRFILE8(MXLB,NLB,LBR,LPT,LTY,LLB,LNL)
C
      END SUBROUTINE
C
C     ---------- ----
      SUBROUTINE KEEP(MXLB,NLB,LBR,LPT,LTY,LLB,LNL,NL,LFR,LTO,DELETEFN)
C
      DIMENSION LBR(MXLB),LPT(MXLB),LTY(MXLB),LLB(MXLB),LNL(MXLB)
      DIMENSION LFR(MXLB),LTO(MXLB)
      LOGICAL, EXTERNAL :: DELETEFN
C
       IF(NLB.NE.0)THEN
         DO I=1,NLB
          IF(DELETEFN(I,LTY(I)))THEN
              LNL(I)=0
            ENDIF
         ENDDO
       ENDIF
C
      RETURN
      END SUBROUTINE KEEP
C
C     ---------- ------
      SUBROUTINE RDFILE(MXLB,NLB,LBR,LPT,LTY,LLB,LNL)
C
      DIMENSION LBR(MXLB),LPT(MXLB),LTY(MXLB),LLB(MXLB),LNL(MXLB)
      LOGICAL EOF
C
       REWIND 28
       NLB=0
 1     CONTINUE
         READ(28,*,END=2)IBR,NTOT,ITP,LAB,NFPR,ISW,NTPL,NAR,NSKIP
         IF(NLB.GE.MXLB)THEN
           WRITE(6,101)MXLB
           STOP
         ENDIF
         NLB=NLB+1
         LBR(NLB)=IABS(IBR)
         LPT(NLB)=IABS(NTOT)
	 IF(ITP.LT.0)THEN
           LTY(NLB)=-MOD(IABS(ITP),10)
	 ELSE
           LTY(NLB)= MOD(ITP,10)
	 ENDIF
         LLB(NLB)=LAB
         LNL(NLB)=LAB
         CALL SKIP(28,NSKIP,EOF)
         IF(EOF)GOTO 2
       GOTO 1
C
 101  FORMAT(' ERROR : Maximum number of labels (',I6,') exceeded.',
     *     /,' Increase MXLB in auto/07p/src/utility.f and recompile.')
C
 2    RETURN
      END SUBROUTINE RDFILE
C
C     ---------- -------
      SUBROUTINE WRFILE7(MXLB,NLB,LBR,LPT,LTY,LLB,LNL)
C
      DIMENSION LBR(MXLB),LPT(MXLB),LTY(MXLB),LLB(MXLB),LNL(MXLB)
      CHARACTER*132 LINE
      CHARACTER*4 CHR4,CLAB
      CHARACTER*1 CH1
      INTEGER LEN
      LOGICAL EOL
C
       L=0
       LNUM=0
       REWIND 27
       DO
         EOL=.TRUE.
         READ(27,"(A)",ADVANCE='NO',EOR=97,END=99,SIZE=LEN)LINE
         EOL=.FALSE.
 97      CONTINUE
         LNUM=LNUM+1
         CLAB=LINE(16:19)
         IF(LINE(1:4).NE.'   0' .AND. CLAB.NE.'   0')THEN
           L=L+1
           IF(CLAB.NE.CHR4(LLB(L)))THEN
             WRITE(6,"(A/A,A4,A,I5/A,A4/A/A)", ADVANCE="NO")
     *        ' WARNING : The two files have incompatible labels :',
     *        '  b-file label ',CLAB,' at line ',LNUM,
     *        '  s-file label ',CHR4(LLB(L)),
     *        ' New labels may be assigned incorrectly.',
     *        ' Continue ? : '
             READ(5,"(A1)")CH1
             IF(CH1/='y'.AND.CH1/='Y')THEN
               WRITE(6,"(A)")
     *               'Rewrite discontinued. Recover original files'
               RETURN
             ENDIF
           ENDIF
           LINE(16:19)=CHR4(LNL(L))
         ENDIF
         IF(.NOT.EOL)THEN
           DO
             WRITE(37,"(A)",ADVANCE='NO')LINE(1:LEN)
             READ(27,"(A)",ADVANCE='NO',EOR=98,SIZE=LEN)LINE
           ENDDO
 98        CONTINUE
         ENDIF
         WRITE(37,"(A)")LINE(1:LEN)
       ENDDO
 99   RETURN
      END SUBROUTINE WRFILE7
C
C     ---------- -------
      SUBROUTINE WRFILE8(MXLB,NLB,LBR,LPT,LTY,LLB,LNL)
C
      DIMENSION LBR(MXLB),LPT(MXLB),LTY(MXLB),LLB(MXLB),LNL(MXLB)
      DOUBLE PRECISION U,RLDOT,PAR
      INTEGER ICP
      ALLOCATABLE U(:),RLDOT(:),PAR(:),ICP(:)
C
       L=0
       REWIND 28
 1     CONTINUE
         READ(28,*,END=99)IBR,NTOT,ITP,LAB,NFPR,ISW,NTPL,
     *               NAR,NROWPR,NTST,NCOL,NPAR1
         NPAR2=NPAR1
         L=L+1
         IF(LNL(L).GT.0)
     *   WRITE(38,101)IBR,NTOT,ITP,LNL(L),NFPR,ISW,NTPL,
     *             NAR,NROWPR,NTST,NCOL,NPAR2
         ALLOCATE(U(NAR),ICP(NFPR),RLDOT(NFPR),PAR(NPAR2))
         DO 2 J=1,NTPL
           READ(28,*)  (U(I),I=1,NAR) 
           IF(LNL(L).GT.0)
     *     WRITE(38,102)(U(I),I=1,NAR)
 2       CONTINUE
         IF(NTST.NE.0)THEN
           READ(28,*)  (ICP(I),I=1,NFPR)
           IF(LNL(L).GT.0)
     *     WRITE(38,100)(ICP(I),I=1,NFPR)
           READ(28,*)  (RLDOT(I),I=1,NFPR)
           IF(LNL(L).GT.0)
     *     WRITE(38,102)(RLDOT(I),I=1,NFPR)
           DO 3 J=1,NTPL
             READ(28,*)  (U(I),I=1,NAR-1) 
             IF(LNL(L).GT.0)
     *       WRITE(38,102)(U(I),I=1,NAR-1)
 3         CONTINUE
         ENDIF
         READ(28,*) (PAR(I),I=1,NPAR2)
         IF(NPAR1.NE.NPAR2)THEN
           N1= 1+ (NPAR1-1)/7
           N2= 1+ (NPAR2-1)/7
           IF(N1.GT.N2)THEN
             DO I=1,N1-N2
               READ(28,*)
             ENDDO
           ENDIF
         ENDIF
         IF(LNL(L).GT.0)
     *   WRITE(38,102)(PAR(I),I=1,NPAR2)
         DEALLOCATE(U,ICP,RLDOT,PAR)
       GOTO 1
C
 100   FORMAT(20I5)
 101   FORMAT(6I6,I8,I6,I8,3I5)
 102   FORMAT(4X,1P7E19.10)
C
 99   RETURN
      END SUBROUTINE WRFILE8
C
C     ------- -------- ------
      LOGICAL FUNCTION INLIST(MXLB,LAB,NL,LFR,LTO)
C
      DIMENSION LFR(MXLB),LTO(MXLB)      
C
       INLIST=.FALSE.
       DO 1 I=1,NL
         IF(LAB.GE.LFR(I).AND.LAB.LE.LTO(I))THEN
           INLIST=.TRUE.
           RETURN
         ENDIF
 1     CONTINUE
C
      RETURN
      END FUNCTION INLIST
C
C     ---------- ----
      SUBROUTINE SKIP(IUNIT,NSKIP,EOF)
C
C Skips the specified number of lines on fort.IUNIT.
C
      LOGICAL EOF
C
       EOF=.FALSE.
       DO 1 I=1,NSKIP
         READ(IUNIT,*,END=2)
 1     CONTINUE
       RETURN
C
 2     CONTINUE
        EOF=.TRUE.
      RETURN
      END SUBROUTINE SKIP
C
C     ----------- -------- ----
      CHARACTER*4 FUNCTION CHR4(N)
C
      CHARACTER*1 INT2CHR
C
      M=N
      CHR4(1:4)='    '
      DO 1 I=4,1,-1
       CHR4(I:I)=INT2CHR(MOD(M,10))
       M=M/10
       IF(M.EQ.0)RETURN
 1    CONTINUE
C
      RETURN
      END FUNCTION CHR4
C
C     ----------- -------- -------
      CHARACTER*1 FUNCTION INT2CHR(N)
C
       IF(N.EQ.0)THEN
         INT2CHR='0'
       ELSEIF(N.EQ.1)THEN
         INT2CHR='1'
       ELSEIF(N.EQ.2)THEN
         INT2CHR='2'
       ELSEIF(N.EQ.3)THEN
         INT2CHR='3'
       ELSEIF(N.EQ.4)THEN
         INT2CHR='4'
       ELSEIF(N.EQ.5)THEN
         INT2CHR='5'
       ELSEIF(N.EQ.6)THEN
         INT2CHR='6'
       ELSEIF(N.EQ.7)THEN
         INT2CHR='7'
       ELSEIF(N.EQ.8)THEN
         INT2CHR='8'
       ELSEIF(N.EQ.9)THEN
         INT2CHR='9'
       ENDIF
C
      RETURN
      END FUNCTION INT2CHR
C
C LISTLB, TYPE and RELABEL are used by listlabels.f, autlab.f, or relabel.f
C
C     ---------- ------
      SUBROUTINE LISTLB(MXLB,NLB,LBR,LPT,LTY,LLB,LNL,NL,LFR,LTO,NEW)
C
      LOGICAL NEW
      DIMENSION LBR(MXLB),LPT(MXLB),LTY(MXLB),LLB(MXLB),LNL(MXLB)
      DIMENSION LFR(MXLB),LTO(MXLB)
      CHARACTER*2 TYPE
      LOGICAL FIRST
C
       IF(NLB.EQ.0)THEN
          WRITE(6,101)
       ELSE
         FIRST=.TRUE.
         DO 1 I=1,NLB
          IF(NL.EQ.0 .OR. INLIST(MXLB,LLB(I),NL,LFR,LTO))THEN
            IF(FIRST)THEN
              IF(NEW)THEN
                 WRITE(6,105)
              ELSE
                 WRITE(6,102)
              ENDIF
              FIRST=.FALSE.
            ENDIF
            IF(.NOT.NEW)THEN
              WRITE(6,103)LBR(I),LPT(I),TYPE(LTY(I)),LLB(I)
            ELSEIF(LLB(I).EQ.LNL(I))THEN
              WRITE(6,103)LBR(I),LPT(I),TYPE(LTY(I)),LLB(I),LNL(I)
            ELSEIF(LNL(I).EQ.0)THEN
              WRITE(6,104)LBR(I),LPT(I),TYPE(LTY(I)),LLB(I)
            ELSE
              WRITE(6,103)LBR(I),LPT(I),TYPE(LTY(I)),LLB(I),LNL(I)
            ENDIF
          ENDIF
 1       CONTINUE
       ENDIF
C
 101   FORMAT(/,' Empty solutions file')
 102   FORMAT(/,'  BR    PT  TY LAB')
 103   FORMAT(I4,I6,2X,A2,I4,I5)
 104   FORMAT(I4,I6,2X,A2,I4,'        DELETED')
 105   FORMAT(/,'  BR    PT  TY LAB  NEW')
C
      RETURN
      END SUBROUTINE LISTLB
C
C     ----------- -------- ----
      CHARACTER*2 FUNCTION TYPE(ITP)
C
       IF(MOD(ITP,10).EQ.1)THEN
         TYPE='BP'
       ELSE IF(MOD(ITP,10).EQ.2)THEN
         TYPE='LP'
       ELSE IF(MOD(ITP,10).EQ.3)THEN
         TYPE='HB'
       ELSE IF(MOD(ITP,10).EQ.4)THEN
         TYPE='  '
       ELSE IF(MOD(ITP,10).EQ.-4)THEN
         TYPE='UZ'
       ELSE IF(MOD(ITP,10).EQ.5)THEN
         TYPE='LP'
       ELSE IF(MOD(ITP,10).EQ.6)THEN
         TYPE='BP'
       ELSE IF(MOD(ITP,10).EQ.7)THEN
         TYPE='PD'
       ELSE IF(MOD(ITP,10).EQ.8)THEN
         TYPE='TR'
       ELSE IF(MOD(ITP,10).EQ.9)THEN
         TYPE='EP'
       ELSE IF(MOD(ITP,10).EQ.-9)THEN
         TYPE='MX'
       ELSE
         TYPE='  '
       ENDIF
C
      RETURN
      END FUNCTION TYPE
C
C     ---------- -------
      SUBROUTINE RELABEL(MXLB,NLB,LBR,LPT,LTY,LLB,LNL,NL,LFR,LTO)
C
      DIMENSION LBR(MXLB),LPT(MXLB),LTY(MXLB),LLB(MXLB),LNL(MXLB)
      DIMENSION LFR(MXLB),LTO(MXLB)
C
       IF(NL.EQ.0)THEN
         NN=0
         DO I=1,NLB
          IF(LNL(I).GT.0)THEN
            NN=NN+1
            LNL(I)=NN
          ENDIF
         ENDDO
       ELSE
         DO I=1,NLB
          IF(NL.EQ.0 .OR.
     *       (LNL(I).GT.0 .AND. INLIST(MXLB,LLB(I),NL,LFR,LTO)) )THEN
            WRITE(6,101)LLB(I)
            READ(5,*)LNL(I)
          ENDIF
         ENDDO
       ENDIF
C
 101   FORMAT(' Old label ',I4,';  Enter new label : ',$)
C
      RETURN
      END SUBROUTINE RELABEL
C======================================================================
C======================================================================
      END MODULE UTILITY