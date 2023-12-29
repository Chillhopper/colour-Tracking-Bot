{
 ************************************************************************************************************
 *                                                                                                          *
 *  AUTO-RECOVER NOTICE: This file was automatically recovered from an earlier Propeller Tool session.      *
 *                                                                                                          *
 *  ORIGINAL FOLDER:     C:\Users\fizfa\OneDrive\Desktop\WorkingV1\LiteKitWorksV1\                          *
 *  TIME AUTO-SAVED:     18 hours, 2 minutes ago (16/3/2023 5:05:23 pm)                                     *
 *                                                                                                          *
 *  OPTIONS:             1)  RESTORE THIS FILE by deleting these comments and selecting File -> Save.       *
 *                           The existing file in the original folder will be replaced by this one.         *
 *                                                                                                          *
 *                           -- OR --                                                                       *
 *                                                                                                          *
 *                       2)  IGNORE THIS FILE by closing it without saving.                                 *
 *                           This file will be discarded and the original will be left intact.              *
 *                                                                                                          *
 ************************************************************************************************************
.}
{
 ************************************************************************************************************
 *                                                                                                          *
 *  AUTO-RECOVER NOTICE: This file was automatically recovered from an earlier Propeller Tool session.      *
 *                                                                                                          *
 *  ORIGINAL FOLDER:     C:\Users\fizfa\OneDrive\Desktop\WorkingV1\LiteKitWorksV1\                          *
 *  TIME AUTO-SAVED:     over 4 days ago (10/3/2023 5:37:26 pm)                                             *
 *                                                                                                          *
 *  OPTIONS:             1)  RESTORE THIS FILE by deleting these comments and selecting File -> Save.       *
 *                           The existing file in the original folder will be replaced by this one.         *
 *                                                                                                          *
 *                           -- OR --                                                                       *
 *                                                                                                          *
 *                       2)  IGNORE THIS FILE by closing it without saving.                                 *
 *                           This file will be discarded and the original will be left intact.              *
 *                                                                                                          *
 ************************************************************************************************************
.}
{{

  File: RxComm.spin

  Developer: Kenichi Kato
  Copyright (c) 2021, Singapore Institute of Technology
  Platform: Parallax USB Project Board (P1)
  Date: 14 Sep 2021
  Objective:
    - Communication module on the receiver board
    - Module selected is ZigBee

}}
CON


  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000
  _ConClkFreq = ((_clkmode - xtal1) >> 6) * _xinfreq
  _Ms_001   = _ConClkFreq / 1_000

  STM_Rx    = 11 '
  STM_Tx    = 12


  motCmdStart     = $7A
  motCmdStopAll   = $0A
  motCmdForward   = $01
  motCmdReverse   = $02
  motCmdLeft      = $03
  motCmdRight     = $04
  motcmdReset     = $05         'Reset indiviual motors
  motCmdMcTL      = $06         ' Top-Left
  motCmdMcTR      = $07         ' Top-Right
  motCmdMcBL      = $08         ' Bottom-Left
  motCmdMcBR      = $09         ' Bottom-Right
  motCmdMcCW      = $10         ' Turning Clockwise
  motCmdMcCCW     = $11         ' Turning Counter-Clockwise
  motCmdMcSLeft   = $12         ' Move Side Left
  motCmdMcSRight  = $13         ' Move Side Left

  tof_limit = 300
  ultra_limit = 10

  sb = $7A
  pSize = 4
  ok = $01

OBJ
  Comm  : "FullDuplexSerialExt.spin"
  pst   : "Parallax Serial Terminal"
  Def   : "RxBoardDef.spin"
  Mot   : "MecanumControl.spin"
  Sen   : "SensorMUXControl.spin"


VAR
  long mainHubMS, cog, cogStack[64], Tof[2], Ultra[2], Cmd, AllDutyCycle, motOrient, motDCycle
  byte buffer[pSize]

PUB Stop
  if(cog)
    cogstop(~cog - 1)
  return

PUB Start(MS,commMem,dirval,speedval,checkval,cmc)
  mainHubMS := MS

  Stop

  cog := cognew(commCore(commMem,dirval,speedval,checkval,cmc), @cogStack) + 1
  return cog


PUB commCore(commMem,dirval,speedval,checkval,cmc) | B1, B2, B3, B4, checksm, i


  'Mot.Start(_Ms_001, @Cmd, @AllDutyCycle, @motOrient, @motDCycle)
  Comm.Start(14, 18, 0, 115200)
  {pst.Start(115200)}
  'Pause(500)
  'Sen.Start(@ToF, @Ultra)



  repeat
    Comm.tx(long[commMem])
      Pause(50)
       checksm:=0
       repeat
        B1 := Comm.rx
        waitcnt(CLKFREQ/115200 + CNT)          'wait for char to be received before reading
       until B1 == sb

      buffer[0] := sb                          '0 is for start byte
      ' Wait for the rest of the packet
        repeat i from 1 to pSize - 1             '1 to 3 is for x, y, and z values. 4 is for checksum received
          buffer[i] := Comm.rx
          waitcnt(CLKFREQ/115200 + CNT)

        long[commMem] := buffer[0]
        long[dirval] := buffer[1]
        long[speedval] := buffer[2]
        long[checkval] := buffer[3]

        B1 := buffer[0]
        B2 := buffer[1]
        B3 := buffer[2]
        B4 := buffer[3]



       checksm^=B1
       checksm^=B2
       checksm^=B3

      if checksm == B4 and B1 == $7A

       

         case B2

          motCmdStopAll:
            long[cmc] := 0
            'AllDutyCycle:=20

          motCmdForward :
            long[cmc] := 1
            'Comm.rxflush 
            'AllDutyCycle:=20

          motCmdReverse:
            long[cmc] := 2
            'Comm.rxflush
            'AllDutyCycle:=20

          motCmdLeft:
            long[cmc] := 3
            'AllDutyCycle:=20

          motCmdRight:
            long[cmc] := 4
            'AllDutyCycle:=20

          motCmdMcTL:
            long[cmc] := 6
            'AllDutyCycle:=20

          motCmdMcTR:
            long[cmc] := 7
            'AllDutyCycle:=20
                   
          motCmdMcBL:
            long[cmc] := 8
            'AllDutyCycle:=20

          motCmdMcBR:
            long[cmc] := 9
            'AllDutyCycle:=20

          motCmdMcCW:
            long[cmc] := 10
            'AllDutyCycle:=20

          motCmdMcCCW:
            long[cmc] := 11
            'AllDutyCycle:=20

          motCmdMcSRight:
            long[cmc] := 12
            'AllDutyCycle:=20

          motCmdMcSLeft:
            long[cmc] := 13
            'AllDutyCycle:=20




















PRI Pause(ms) | t
  t := cnt - 1088                                               ' sync with system counter
  repeat (ms #> 0)                                              ' delay must be > 0
    waitcnt(t += mainHubMS)
  return