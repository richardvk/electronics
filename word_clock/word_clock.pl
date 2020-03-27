#!/usr/bin/perl

use strict;
use warnings;

my $version = 20200326;

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);

# Its just gonealmost ten 23 x 6
# twenty quarterhalf past
# to midday  midnight one
# two three four five six
# seven  eight  nine  ten 
# eleven  twelve   oclock 

# Its just gone almost 20 x 7 
# ten  twenty  quarter
# half past to  midday
# midnight   one   two 
# three four five  six
# seven eight nine ten
# eleven twelve oclock 

# Its   just   gone 17 x 8
# almost ten twenty
# quarter half past
# to midday one two 
# midnight four six
# three  seven nine
# five eight eleven
# ten twelve oclock 


my ($w1, $w2, $w3, $w4, $w5) = ('','','','','');

my %words = (0=>'twelve',1=>'one',2=>'two',3=>'three',4=>'four',5=>'five',6=>'six',7=>'seven',8=>'eight',9=>'nine',10=>'ten',11=>'eleven',12=>'twelve',13=>'one',14=>'two',15=>'three',16=>'four',17=>'five',18=>'six',19=>'seven',20=>'eight',21=>'nine',22=>'ten',23=>'eleven',24=>'twelve');

my $min_str = $min  < 10 ? '0'.$min  : $min;
my $hr_str  = $hour < 10 ? '0'.$hour : $hour;

# The start of all great things...
my $sentence = "It's ";

# Word one
if (($min>=1 && $min<=5) || ($min>=11 && $min<=12) || ($min>=16 && $min<=17) || ($min>=21 && $min<=25) || ($min>=31 && $min<=35) || ($min>=41 && $min<=42) || ($min>=46 && $min<=47) || ($min>=51 && $min<=54)){
    $w1 = "just gone ";
}
elsif (($min>=6 && $min<=9) || ($min>=11 && $min<=14) || ($min>=18 && $min<=19) || ($min>=26 && $min<=29)  || ($min>=36 && $min<=39) || ($min>=43 && $min<=44) || ($min>=48 && $min<=49) || ($min>=55 && $min<=59)){
    $w1 = "almost ";
}

# Word two
if (($min>=13 && $min<=17) || ($min>=43 && $min<=47)){
    $w2 = "quarter ";
}
elsif ($min>=26 && $min<=35){
    $w2 = "half ";
}
elsif (($min>=6 && $min<=12) || ($min>=48 && $min<=54)){
    $w2 = "ten ";
}
elsif (($min>=18 && $min<=25) || ($min>=36 && $min<=42)){
    $w2 = "twenty ";
}


# Word three
if (($min>=6 && $min<=35)) {
    $w3 = "past ";
}
elsif (($min>=36 && $min<=54)) {
    $w3 = "to ";
}

# Word for the hour
my $word_hr = $hour;

if ($min>=36 && $min<=59) {
    $word_hr = $hour+1;
}

$word_hr = $word_hr<=12 ? $word_hr : ($word_hr==24 ? 0 : $word_hr-12);

# Word four
if ($word_hr == 0 && ($min>=55 || $min<=5)) {
    $w4 = "midnight ";
}
elsif ($word_hr == 12 && ($min>=55 || $min<=5)) {
    $w4 = "midday ";
}
else {
    $w4 = $words{$word_hr} . " ";
}

# Word five
if ((($min>=0 && $min<=5) || ($min>=55 && $min<=59)) && $word_hr != 0 && $word_hr != 12) {
    $w5 = "o'clock";
}

$sentence .= $w1 . $w2 . $w3 . $w4 . $w5;
printf "%s:%s: %s\n",$hr_str,$min_str,$sentence;
