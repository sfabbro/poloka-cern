*
* $Id: sfactd.inc,v 1.1 2004/02/20 10:48:16 nrl Exp $
*
* $Log: sfactd.inc,v $
* Revision 1.1  2004/02/20 10:48:16  nrl
* Initial revision
*
* Revision 1.1.1.2  2002/10/03 11:35:46  seb
*
* first release directly imported from lpnhe paris
*
* Revision 1.1.1.1  1999/09/13 14:57:00  astier
*
*
* Revision 1.1.1.1  1996/02/15 17:49:04  mclareni
* Kernlib
*
*
*
* sfactd.inc
*
          IFAIL  =  0
          JFAIL  =  0
          DET    =  ONE
          DO 144    J  =  1, N
             IF(PIVOTF(A(J,J)) .LE. 0.)  GOTO 150
             DET     =  DET * A(J,J)
             A(J,J)  =  ONE / A(J,J)
             T  =  SIZEF(DET)
             IF(T .LT. G1)  THEN
                DET    =  ONE
                IF(JFAIL .EQ. 0)  JFAIL  =  -1
             ELSEIF(T .GT. G2)  THEN
                DET    =  ONE
                IF(JFAIL .EQ. 0)  JFAIL  =  +1
             ENDIF
             IF(J .EQ. N)  GOTO 199
             JP1  =  J+1
             DO 143   L  =  JP1, N
                A(J,L)  =  A(J,J)*A(L,J)
                S1      =  -A(L,J+1)
                DO 141  I  =  1, J
                   S1  =  DOTF(A(L,I),A(I,J+1),S1)
 141               CONTINUE
                A(L,J+1)  =  -S1
 143            CONTINUE
 144         CONTINUE
 150      IFAIL  =  -1
          JFAIL  =  -2
          DET    =  ZERO
          RETURN
 199      IF(JFAIL .NE. 0)   DET  =  ZERO