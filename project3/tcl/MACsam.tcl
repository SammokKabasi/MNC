#initialize global variable

set ns [new Simulator]

#set tracing on
set tracefile [open simulation.tr w]
$ns trace-all $tracefile


#set variables to be used for topology object
set val(x) 100
set val(y) 100
set val(z) 0

#topography object
set topopl [new Topography]
$topol load_flatgrid $val(x) $val(y)

#define some more variables
set val(nn) 101 #number of mobile nodes

#create the GOD object
create-god $val(nn)

#create the channel used for propagation
set channel [new Channel/wireless channel]

#configure node

$ns_ node-config -adhocRouting DumbAgent \
	-llType LL	\
	-macType MAC/YOURPROTOCOL \
	-ifqType 
	-ifqLen
	-antType Antenna/OmniAntenna
	-propType Phy/WirelessPhy
	-phyType Phy/WirelessPhy \
	-topoInstance $topology \
	-agentTrace ON \
	-macTrace ON \
	-movementTrace OFF \
	-channel $channel \
	-energyModel EnergyModel \
	-idlePower 0.035 \
	-rxPower 0 \
	-txPower 0.660 \
	-sleepPower 0.001 \
	-transitionPower 0.05 \
	-transitionTime 0.005 \
	-initialEnergy 1000 \
	
#setup a random number generator
set rng [new Ring]
$rng seed 1

#and a random variable for node positioning
set randl [new randomVariable/Uniform]
$randl use-rng $rng
$randl set min_ -25.0
$randl set max_ 25.0

#use the RV to assign random positions to each mobile node

for {set i 0} {$i < $val(nn)} {inorr i} {
	set node($i) [$ns node]
	$node($1) random-motion 0
	$node($i) set X_ \
			[expr 25 + [$rand1 value]]
	$node($1) set Y_ \
			[expr 25 + [$rand1 value]]
}

#one mobile node (say node0) is a sink 
set sink [new Agent/LossMonitor]
$ns attach-agent $node(0) $sink

#create traffic sources for each node and attach

for {set i l} {$i < $val(nn)} {incr i} {
	set protocol($i) [new tcp]
	$ns attach-agent $node($i) $protocol($i)
	$ns attach-agent $node($i) $sink
	set traffic($i) [new HAT_TRAFFIC_TYPE?]
	$traffic($i) set packetSize_ 128
	$traffic($i) set rate_ 400
	$traffic($i) attach-agent $protocol($i)
	$ns at 1 "$traffic($1) start"
}


#use the loss monitor to record data loss information

set stats [open statistics.dat w]

#function to collect stats on the losses in traffic flow

proc update_stats {} {
	global sink
	global stats
	
	set ns [Simulator instance]
	
	st bytes [$sink set bytes_]
	set now [$ns now]
	puts $stats "$now $bytes"
	
	#Reset the bytes_ values on the sink
	$sink set bytes_ 0
	
	#reschedule the procedure
	set time 1
	$ns at [expr $now+$time] "update_stats"
}

#define a function to handle the cleanup when simulation ends

proc finish{} {
	global ns tracefile
	$ns flush-trace
	close $tracefile
}

#call the functions, start the simulation

$ns at 1 "update_stats"
$ns at 100 "finish"

$ns halt

$ns run

#======

$val(mac) set interval_ $cbr_interval;

set val(chan)		Channel/WirelessChannel;		#channeltype
set val(prop)		Propagation/TwoRayGround;		#radio-propagation model
set val(ant)		Antenna/OmniAntenna;			#Antenna type
set val(ll)			LL;								#Link layer type
set val(ifq)		Queue/Droptail/PriQueue;		#Interface queue type
set val(ifqlen)		50;								#max packet in ifq
set val(netif)		Phy/WirelessPhy;				#network interface type
set val(mac)		Mac/SamProjC;						#MAC type
set val(rp)			DumbAgent;						#ad-hoc routing protocol
set val(nn)			101;								#number of mobile nodes
	

			
#val(ifqlen) and val(nn) --> what values?










	
