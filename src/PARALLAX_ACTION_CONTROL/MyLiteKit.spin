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
{Object_Title_and_Purpose}


CON
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000
  _ConClkFreq = ((_clkmode - xtal1) >> 6) * _xinfreq
  _Ms_001   = _ConClkFreq / 1_000


  
  ToF_limit = 300
  Ultra_limit = 400


PRI Pause(ms) | t
  t := cnt - 1088                                               ' sync with system counter
  repeat (ms #> 0)                                              ' delay must be > 0
    waitcnt(t += _MS_001)


VAR
  ' Motors
  long Cmd, AllDutyCycle, motOrient, motDCycle

  ' Sensors
  long Tof[2], Ultra[4]

  ' Wireless Communication
  long  commMem, dirval,speedval,checkval, cmc

  'EStop
  Long Emer



OBJ
  'Def   : "Definitions.spin"
  pst   : "Parallax Serial Terminal.spin"

  Mot   : "MecanumControl.spin"
  Sen   : "SensorMUXControl.spin"
  Com   : "Comm2Control.spin"
  'EmerStop : "E-StopControl.spin"

PUB Main | commmand


  pst.Start(115200)

  Mot.Start(_Ms_001, @Cmd, @AllDutyCycle, @motOrient, @motDCycle)

  Sen.Start(@Tof, @Ultra)

  {Emerstop.Start(_Ms_001, @Emer) }

  Com.Start(_Ms_001, @commMem, @dirval, @speedval, @checkval, @cmc)




 repeat

    'WAITCNT((1*(clkfreq)) + cnt)
    pst.Str(String("commMem: "))
    pst.Hex(commMem,2)
    'pst.Hex(commMem,4)
    pst.Chars(pst#NL, 2)

    'WAITCNT((1*(clkfreq)) + cnt)
    pst.Str(String("dirval: "))
    pst.Hex(dirval,2)
    'pst.Hex(dirval,4)
    pst.Chars(pst#NL, 2)

    'WAITCNT((1*(clkfreq)) + cnt)
    pst.Str(String("speedval: "))
    pst.Hex(speedval,2)
    'pst.Hex(speedval,4)
    pst.Chars(pst#NL, 2)

    'WAITCNT((1*(clkfreq)) + cnt)
    pst.Str(String("checkval: "))
    pst.Hex(checkval,2)
    'pst.Hex(checkval,4)
    pst.Chars(pst#NL, 2)

  {repeat
      if Tof[0]>ToF_limit or Tof[1]>ToF_limit {or Ultra[0]<Ultra_limit} or Ultra[1]<Ultra_limit or Ultra[2]<Ultra_limit or Ultra[3]<Ultra_limit
        Cmd := 0
        commMem:=$00
       'AllDutyCycle := 100
      else }



        'commMem:=$01
      repeat
       long[@commMem]:=$01
        case long[@cmc]

          0:
            Cmd := 0
            'AllDutyCycle := 50
          1:
            'if((Ultra[0] > 0) and Ultra[0]< Ultra_limit)
             ' Cmd:=0
              'long[@commMem]:=$00                             'forward
            'if(Ultra[0]>Ultra_limit)
             '   long[@commMem]:=$01
                Cmd:=1
                AllDutyCycle:= long[@speedval]



          2:
            'if(Ultra[1] < Ultra_limit)
             ' Cmd:=0
              'long[@commMem]:=$00
            'if(Ultra[1]>Ultra_limit)                          'reverse
             '   long[@commMem]:=$01
                Cmd:=2
                AllDutyCycle:= long[@speedval]


          3:
            ' if(Ultra[2] < Ultra_limit)
             ' Cmd:=0                                                 '
             ' long[@commMem]:=$00                   'left
           ' if(Ultra[2]>Ultra_limit)
            '    long[@commMem]:=$01
                Cmd:=3
                AllDutyCycle:= long[@speedval]
          4:
             ' if(Ultra[3] < Ultra_limit)
              '  Cmd:=0
              '  long[@commMem]:=$00
            'if(Ultra[3]>Ultra_limit)                     'right
             '   long[@commMem]:=$01
                Cmd:=4
                AllDutyCycle:= long[@speedval]

          6:
             ' if(Ultra[2] < Ultra_limit)  or ((Ultra[0] > 0) and Ultra[0]< Ultra_limit)
              '  Cmd:=0                                    'top left
               ' long[@commMem]:=$00
            'if(Ultra[2]>Ultra_limit) and (Ultra[0]>Ultra_limit)
             '     long[@commMem]:=$01
                 Cmd:=6
                  AllDutyCycle:= long[@speedval]

          7:
            'if(Ultra[3] < Ultra_limit) or ((Ultra[0]>0) and Ultra[0]< Ultra_limit)
            '  Cmd:=0
             ' long[@commMem]:=$00                            'top right
            'if(Ultra[3]>Ultra_limit)and (Ultra[0]>Ultra_limit)           '
             '   long[@commMem]:=$01
                Cmd:=7
                AllDutyCycle:= long[@speedval]

          8:
            'if((Ultra[1] > 0) and Ultra[1]< Ultra_limit) or (Ultra[3]< Ultra_limit)
             ' Cmd:=0
              'long[@commMem]:=$00
            'if(Ultra[3]>Ultra_limit) and (Ultra[1] >Ultra_limit)
             '   long[@commMem]:=$01            'Back left
                Cmd:=9
                AllDutyCycle:= long[@speedval]

          9:
            'if(Ultra[2] < Ultra_limit) or ((Ultra[1] > 0) and Ultra[1]< Ultra_limit)
             ' Cmd:=0
             ' long[@commMem]:=$00                    'Back Right
            'if(Ultra[2]>Ultra_limit) and (Ultra[1]>Ultra_limit)
             '   long[@commMem]:=$01
                Cmd:=8
                AllDutyCycle:= long[@speedval]

          {10:
            if(Ultra[0] < Ultra_limit)
              Cmd:=0
              commMem:=$00
            if(Ultra[0]>Ultra_limit)
                commMem:=$01
                Cmd:=10
                AllDutyCycle:=50

          11:
            if(Ultra[0] < Ultra_limit)
              Cmd:=0
              commMem:=$00
            if(Ultra[0]>Ultra_limit)
                commMem:=$01
                Cmd:=11
                AllDutyCycle:=50   }

          12:
           ' if(Ultra[2] < Ultra_limit)
            '  Cmd:=0
             ' long[@commMem]:=$00
           ' if(Ultra[2]>Ultra_limit)
            '    long[@commMem]:=$01                'sl
                Cmd:=12
                AllDutyCycle:= long[@speedval]

          13:
            'if(Ultra[3] < Ultra_limit)
            '  Cmd:=0
            '  long[@commMem]:=$00
            'if(Ultra[3]>Ultra_limit)
             '   long[@commMem]:=$01              'sr
                Cmd:=13
                AllDutyCycle:= long[@speedval]





















    {case long [@Emer]
        0:
           Cmd:=0

        1:repeat
            case
               0: Cmd := 0 'stop
                 pst.Str(String("Command Stop"))
                 pst.Chars(pst#NL, 2)
               1:
                 if (ToF[0] > ToF_limit) OR (Ultra[1] < Ultra_limit)
                   Comm := 0
                 Cmd := 1 'forward
                 Pulse := 900
                 pst.Str(String("Command Forward"))
                 pst.Chars(pst#NL, 2)
                 Comm := 0
                 'mode := 0 'change the mode back for forward detection
               2:
                 Cmd := 2 'reverse
                 Pulse := 1000
                 pst.Str(String("Command reverse"))
                 pst.Chars(pst#NL, 2)
                 Comm:=0
               3:
                 Cmd := 3 'left
                 Pulse := 1500
                 pst.Str(String("Command left"))
                 pst.Chars(pst#NL, 2)
                 Pause(250)
                 Comm := 0
               4:
                 Cmd := 4 'right
                 Pulse := 1450
                 pst.Str(String("Command right"))
                 pst.Chars(pst#NL, 2)
                 Pause(250)
                 Comm := 0
                                Pause(250)}