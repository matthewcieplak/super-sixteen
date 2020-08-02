EESchema Schematic File Version 4
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Mechanical:MountingHole_Pad H4
U 1 1 5F049E58
P 3675 3375
F 0 "H4" H 3775 3424 50  0000 L CNN
F 1 "MountingHole_Pad" H 3775 3333 50  0000 L CNN
F 2 "Mounting_Holes:MountingHole_3.2mm_M3_Pad" H 3675 3375 50  0001 C CNN
F 3 "~" H 3675 3375 50  0001 C CNN
	1    3675 3375
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H3
U 1 1 5F04A1A6
P 3675 3050
F 0 "H3" H 3775 3099 50  0000 L CNN
F 1 "MountingHole_Pad" H 3775 3008 50  0000 L CNN
F 2 "Mounting_Holes:MountingHole_3.2mm_M3_Pad" H 3675 3050 50  0001 C CNN
F 3 "~" H 3675 3050 50  0001 C CNN
	1    3675 3050
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H2
U 1 1 5F04A820
P 3675 2725
F 0 "H2" H 3775 2774 50  0000 L CNN
F 1 "MountingHole_Pad" H 3775 2683 50  0000 L CNN
F 2 "Mounting_Holes:MountingHole_3.2mm_M3_Pad" H 3675 2725 50  0001 C CNN
F 3 "~" H 3675 2725 50  0001 C CNN
	1    3675 2725
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H1
U 1 1 5F04B018
P 3675 2400
F 0 "H1" H 3775 2449 50  0000 L CNN
F 1 "MountingHole_Pad" H 3775 2358 50  0000 L CNN
F 2 "Mounting_Holes:MountingHole_3.2mm_M3_Pad" H 3675 2400 50  0001 C CNN
F 3 "~" H 3675 2400 50  0001 C CNN
	1    3675 2400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5F04B456
P 3250 3750
F 0 "#PWR0101" H 3250 3500 50  0001 C CNN
F 1 "GND" H 3255 3577 50  0000 C CNN
F 2 "" H 3250 3750 50  0001 C CNN
F 3 "" H 3250 3750 50  0001 C CNN
	1    3250 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	3675 2500 3250 2500
Wire Wire Line
	3250 2500 3250 2825
Wire Wire Line
	3675 3475 3250 3475
Connection ~ 3250 3475
Wire Wire Line
	3250 3475 3250 3750
Wire Wire Line
	3675 3150 3250 3150
Connection ~ 3250 3150
Wire Wire Line
	3250 3150 3250 3475
Wire Wire Line
	3675 2825 3250 2825
Connection ~ 3250 2825
Wire Wire Line
	3250 2825 3250 3150
$EndSCHEMATC
