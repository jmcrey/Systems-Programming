# splot.tcl
# Called whenever we replot the points

proc replot {val ring resign} {
        puts stdout "$val $ring $resign"
        flush stdout
}

# Create canvas widget
canvas .c -width 300 -height 300 -bg black
pack .c -side top

set bell 1
# Create nine wight squares
set rect [.c create rectangle 0 0 100 100 -fill white -tag b1]
set rect [.c create rectangle 0 100 100 200 -fill white -tag b2]
set rect [.c create rectangle 0 200 100 300 -fill white -tag b3]
set rect [.c create rectangle 100 0 200 100 -fill white -tag b4]
set rect [.c create rectangle 100 100 200 200 -fill white -tag b5]
set rect [.c create rectangle 100 200 200 300 -fill white -tag b6]
set rect [.c create rectangle 200 0 300 100 -fill white -tag b7]
set rect [.c create rectangle 200 100 300 200 -fill white -tag b8]
set rect [.c create rectangle 200 200 300 300 -fill white -tag b9]

.c bind b1 <Button-1> {if {$y==1 && $bell==1} {replot %1 %1 %0} elseif {$y==1 && $bell==0} {replot %1 %0 %0} }
.c bind b2 <Button-1> {if {$y==1 && $bell==1} {replot %2 %1 %0} elseif {$y==1 && $bell==0} {replot %2 %0 %0}  }
.c bind b3 <Button-1> {if {$y==1 && $bell==1} {replot %3 %1 %0} elseif {$y==1 && $bell==0} {replot %3 %0 %0} }
.c bind b4 <Button-1> {if {$y==1 && $bell==1} {replot %4 %1 %0} elseif {$y==1 && $bell==0} {replot %4 %0 %0} }
.c bind b5 <Button-1> {if {$y==1 && $bell==1} {replot %5 %1 %0} elseif {$y==1 && $bell==0} {replot %5 %0 %0} }
.c bind b6 <Button-1> {if {$y==1 && $bell==1} {replot %6 %1 %0} elseif {$y==1 && $bell==0} {replot %6 %0 %0}}
.c bind b7 <Button-1> {if {$y==1 && $bell==1} {replot %7 %1 %0} elseif {$y==1 && $bell==0} {replot %7 %0 %0}}
.c bind b8 <Button-1> {if {$y==1 && $bell==1} {replot %8 %1 %0} elseif {$y==1 && $bell==0} {replot %8 %0 %0} }
.c bind b9 <Button-1> {if {$y==1 && $bell==1} {replot %9 %1 %0} elseif {$y==1 && $bell==0} {replot %9 %0 %0}}

# Frame to hold scrollbars
frame .sf
pack  .sf -expand 1 -fill x

# Scrollbars for rotating view.  Call replot whenever
# we move them.

label .sf.status -text "Awaiting Opponent"

label .sf.handle  
label .sf.opponent 

#scale .sf.ryscroll -label "Y Rotate" -length 300 \
 -from 0 -to 360 -command "replot" -orient horiz

#scale .sf.rzscroll -label "Z Rotate" -length 300 \
 -from 0 -to 360 -command "replot" -orient horiz

# Scrollbar for scaling view.
#scale .sf.sscroll -label "Scale" -length 300 \
  -from 1 -to 1000 -command "replot" -orient horiz \
  -showvalue 0
#  .sf.sscroll set 300

# Pack them into the frame
#pack .sf.rxscroll .sf.ryscroll .sf.rzscroll \
     .sf.sscroll -side top

pack .sf.status .sf.handle .sf.opponent -side top

# Frame for holding buttons
frame .bf
pack  .bf -expand 1 -fill x

# Exit button
button .bf.exit -text "Exit"

# Reset button
button .bf.sreset -text "Silent" -command {if {$bell==1} {set bell 0} else {set bell 1}}  
#    {.sf.sscroll set 300; .sf.rxscroll set 0;
#     .sf.ryscroll set 0; .sf.rzscroll set 0; replot 0}

# Dump PostScript
button  .bf.psout -text "Resign" -command {if {$y==1} {replot 0 1 1}}
#    {.c postscript -colormode gray -file "ps.out"}

# Pack buttons into frame
pack .bf.sreset .bf.psout .bf.exit -side left \
    -expand 1 -fill x

#Call replot

