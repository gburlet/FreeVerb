set mixlevel 95.0
set roomsize 95.0
set damping 32.0
set width 127.0
set freezemode 0.0
set outID "stdout"
set commtype "stdout"

# Configure main window
wm title . "STK FreeVerb"
wm iconname . "FreeVerb"
. config -bg black

# Configure "communications" menu
menu .menu -tearoff 0
menu .menu.communication -tearoff 0 
.menu add cascade -label "Communication" -menu .menu.communication \
				-underline 0
.menu.communication add radio -label "Console" -variable commtype \
				-value "stdout" -command { setComm }
.menu.communication add radio -label "Socket" -variable commtype \
				-value "socket" -command { setComm }
. configure -menu .menu

# Configure title display
label .title -text "Stk FreeVerb" \
				-font {Times 14 bold} -background white \
				-foreground darkred -relief raised

label .title2 -text "by Jezar at Dreampoint\n Ported by Gregory Burlet\n Music Technology, McGill University" \
				-font {Times 12 bold} -background white \
				-foreground darkred -relief raised

pack .title -padx 5 -pady 10
pack .title2 -padx 5 -pady 10

# Configure "note-on" buttons
frame .noteOn -bg black

button .noteOn.on -text NoteOn -bg grey66 -command { noteOn 64.0 64.0 }
button .noteOn.off -text NoteOff -bg grey66 -command { noteOff 64.0 127.0 }
button .noteOn.exit -text "Exit Program" -bg grey66 -command myExit
pack .noteOn.on -side left -padx 5
pack .noteOn.off -side left -padx 5 -pady 10
pack .noteOn.exit -side left -padx 5 -pady 10

pack .noteOn

# Configure sliders
frame .left -bg black

scale .left.effectsmix -from 0 -to 127 -length 400 \
-command {printWhatz "ControlChange    0.0 1 " 44} \
-orient horizontal -label "Effects Mix (0% effect - 100% effect)" \
-tickinterval 32 -showvalue true -bg grey66 \
-variable mixlevel

scale .left.roomsize -from 0 -to 127 -length 400 \
-command {printWhatz "ControlChange    0.0 1 " 22} \
-orient horizontal -label "Room Size" \
-tickinterval 32 -showvalue true -bg grey66 \
-variable roomsize

scale .left.damping -from 0 -to 127 -length 400 \
-command {printWhatz "ControlChange    0.0 1 " 23} \
-orient horizontal -label "Damping" \
-tickinterval 32 -showvalue true -bg grey66 \
-variable damping

scale .left.width -from 0 -to 127 -length 400 \
-command {printWhatz "ControlChange    0.0 1 " 24} \
-orient horizontal -label "Width" \
-tickinterval 32 -showvalue true -bg grey66 \
-variable width

scale .left.freezemode -from 0 -to 127 -length 400 \
-command {printWhatz "ControlChange    0.0 1 " 25} \
-orient horizontal -label "Freeze Mode" \
-tickinterval 32 -showvalue true -bg grey66 \
-variable freezemode

#checkbutton .left.freezemode \
-command {printWhatz "ControlChange    0.0 1 " 27} \
-showvalue true -bg grey66 -label "Disabled" \
-variable freezemode

pack .left.effectsmix -padx 10 -pady 3
pack .left.roomsize -padx 10 -pady 3
pack .left.damping -padx 10 -pady 3
pack .left.width -padx 10 -pady 3
pack .left.freezemode -padx 10 -pady 3

pack .left -side left

proc myExit {} {
    global outID
    puts $outID [format "NoteOff          0.0 1 64 127" ]
    flush $outID
    puts $outID [format "ExitProgram"]
    flush $outID
    close $outID
    exit
}

proc noteOn {pitchVal pressVal} {
    global outID
    puts $outID [format "NoteOn           0.0 1 %f %f" $pitchVal $pressVal]
    flush $outID
}

proc noteOff {pitchVal pressVal} {
    global outID
    puts $outID [format "NoteOff          0.0 1 %f %f" $pitchVal $pressVal]
    flush $outID
}

proc printWhatz {tag value1 value2 } {
    global outID
    puts $outID [format "%s %i %f" $tag $value1 $value2]
    flush $outID
}

# Bind an X windows "close" event with the Exit routine
bind . <Destroy> +myExit

# Socket connection procedure
set d .socketdialog

proc setComm {} {
		global outID
		global commtype
		global d
		if {$commtype == "stdout"} {
				if { [string compare "stdout" $outID] } {
						set i [tk_dialog .dialog "Break Socket Connection?" {You are about to break an existing socket connection ... is this what you want to do?} "" 0 Cancel OK]
						switch $i {
								0 {set commtype "socket"}
								1 {close $outID
								   set outID "stdout"}
						}
				}
		} elseif { ![string compare "stdout" $outID] } {
				set sockport 2001
        set sockhost localhost
				toplevel $d
				wm title $d "STK Client Socket Connection"
				wm resizable $d 0 0
				grab $d
				label $d.message -text "Specify a socket host and port number below (if different than the STK defaults shown) and then click the \"Connect\" button to invoke a socket-client connection attempt to the STK socket server." \
								-background white -font {Helvetica 10 bold} \
								-wraplength 3i -justify left
				frame $d.sockhost
				entry $d.sockhost.entry -width 15
				label $d.sockhost.text -text "Socket Host:" \
								-font {Helvetica 10 bold}
				frame $d.sockport
				entry $d.sockport.entry -width 15
				label $d.sockport.text -text "Socket Port:" \
								-font {Helvetica 10 bold}
				pack $d.message -side top -padx 5 -pady 10
				pack $d.sockhost.text -side left -padx 1 -pady 2
				pack $d.sockhost.entry -side right -padx 5 -pady 2
				pack $d.sockhost -side top -padx 5 -pady 2
				pack $d.sockport.text -side left -padx 1 -pady 2
				pack $d.sockport.entry -side right -padx 5 -pady 2
				pack $d.sockport -side top -padx 5 -pady 2
				$d.sockhost.entry insert 0 $sockhost
				$d.sockport.entry insert 0 $sockport
				frame $d.buttons
				button $d.buttons.cancel -text "Cancel" -bg grey66 \
								-command { set commtype "stdout"
				                   set outID "stdout"
				                   destroy $d }
				button $d.buttons.connect -text "Connect" -bg grey66 \
								-command {
						set sockhost [$d.sockhost.entry get]
						set sockport [$d.sockport.entry get]
					  set err [catch {socket $sockhost $sockport} outID]

						if {$err == 0} {
								destroy $d
						} else {
								tk_dialog $d.error "Socket Error" {Error: Unable to make socket connection.  Make sure the STK socket server is first running and that the port number is correct.} "" 0 OK 
				}   }
				pack $d.buttons.cancel -side left -padx 5 -pady 10
				pack $d.buttons.connect -side right -padx 5 -pady 10
				pack $d.buttons -side bottom -padx 5 -pady 10
		}
}



