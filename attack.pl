#!/usr/bin/perl -w

# Driver script to run mimattack with different parameters and save
# the results to log and data files.

use strict;
use Pod::Usage;
use Getopt::Long;
use Data::Dumper qw(Dumper);
use File::Spec::Functions qw(rel2abs);
use File::Basename;

# Memory should be less than 2**$MEMORY_BITS bytes
# I have 6 GB, so 32 bits => 4GB is max, since 33bits => 8GB
my $MEMORY_BITS = 32; # NOTE: currently not used

my @ATTACKS = qw(mim hashmim diskmim 2table);

my @FIELDS = qw(
    attackName
    cryptosystem
    bits1
    bits2
    message
    seconds
);

my $HASHBITS = 32;

my $MINBITS = 32; my $MAXBITS = 64;
my $BITINC = 2;

# TODO: use separate directory outside of source control for tables and other
# computed ata.
my $CSDIR = dirname(rel2abs($0)) . '/cryptosystems/';

$main::VERSION = '0.1';

my %opts;

my $csFilePath = '';
my $csLabel = '';
my $csDirPath = '';
my $startBits = 0;
my $endBits = 0;
my $tableFilePath = '';
my @messageBitsList = ();
my @attacks = ();

my $man = 0;
my $help = 0;

GetOptions ("attacks|n=s{1,6}" => \@attacks,
            "cryptosystem|c=s" => \$csFilePath,
            "messagebits|b=i{1,2}" => \@messageBitsList,
            'help|?' => \$help, man => \$man) or pod2usage(2);

pod2usage(-verbose => 0) if $help;
pod2usage(-verbose => 2) if $man;

pod2usage(-verbose => 0)
    unless (@messageBitsList && @attacks && $csFilePath);

$startBits = $messageBitsList[0];
if (scalar @messageBitsList == 1) {
    $endBits = $startBits;
} else {
    $endBits = $messageBitsList[1];
}
print "bit range: $startBits - $endBits\n";

if (scalar @attacks == 1 && $attacks[0] eq 'all') {
    @attacks = @ATTACKS;
}

my %requestedAttacks = ();
$requestedAttacks{$_} = 0 for (@ATTACKS);
$requestedAttacks{$_} = 1 for (@attacks);

if (! -f $csFilePath) {
    $csFilePath = $CSDIR . $csFilePath . '.elg';
    if (! -f $csFilePath) {
        pod2usage("No crytposystem found at '$csFilePath'.");
    }
}
($csDirPath = $csFilePath) =~ s/\.elg$//;
($csLabel = $csFilePath) =~ s/.*?([^.\/]*)\.elg$/$1/;
unless (-d $csDirPath) {
    mkdir $csDirPath;
}

my ($primeBits);
# Determine memory usage NOTE: not used
sub tableMemoryBitsUsed ($$) {
    my ($type, $bits1) = @_;
    my $tableMemoryBitsUsed;
    if ($type eq 'mim') {
        my $groupElementBits = log($primeBits)/log(2) - 3;
        # 2^bits1 table entries, each having 2 values, values
        # have size at most 2^groupElementBits
        return $bits1 + $groupElementBits + 1;
    } elsif ($type eq 'hashmim') {
        return $bits1 + log($HASHBITS)/log(2) - 3 + 1;       
    } elsif ($type eq 'diskmim') {
        return 1; # we are not going to every hit the limit here
    } elsif ($type eq '2table') {
        return 0; # TODO: implement
    }
}

print "attacks: ", join (", ", @attacks), "\n";
my $printHeaders = (! -f "$csDirPath/tableTime.dat");

open (CRACKCSV, ">>$csDirPath/crackTime.csv");
print CRACKCSV "cs, attack, bits, tag, time\n"
    if $printHeaders;

my $header = join (" ", 'bits', @attacks);
open (TABLEDAT, ">>$csDirPath/tableTime.dat");
print TABLEDAT $header, "\n" if $printHeaders;

open (CRACKAVGDAT, ">>$csDirPath/crackAvgTime.dat");
print CRACKAVGDAT $header, "\n" if $printHeaders;

$header = join (" ", 'bits', 'messageTag', @attacks);
open (RESULTCOUNTDAT, ">>$csDirPath/resultCount.dat");
print RESULTCOUNTDAT $header, "\n" if $printHeaders;
open (UNIQUERESULTCOUNTDAT, ">>$csDirPath/uniqueResultCount.dat");
print UNIQUERESULTCOUNTDAT $header, "\n" if $printHeaders;

my ($output, $attack, $messageBits, $halfBits, $msgTag, $bitMessageDir, $time, $totalTime, $count);
my (@tableTimes, @crackTimes);
my (%resultCounts);
for $halfBits ($startBits/2 .. $endBits /2) {
    $messageBits = 2 * $halfBits;
    $bitMessageDir = "$csDirPath/${halfBits}_${halfBits}";
    #next unless -d $bitMessageDir;
    @tableTimes = ();
    @crackTimes = ();
    $resultCounts{'unique'} = {};
    $resultCounts{'all'} = {};
    for $attack (@ATTACKS) {
        if (!$requestedAttacks{$attack}) {
            push @tableTimes, "-";
            push @crackTimes, "-";
            print "skipping $messageBits\[$attack\]...\n";
            next;
        }
        print "$messageBits\[$attack\]...\n";
        $tableFilePath = "${bitMessageDir}/$attack.table";
        $output =  `mimattack -n$attack -t'$tableFilePath' -b$messageBits -c '$csFilePath' $bitMessageDir/*.msg 2>&1`;
        
        open (LOG, ">>$bitMessageDir/$attack.log");
        print LOG $output;
        print LOG "=" x 20;
        close (LOG);

        if ($output =~ /^TIME\[table,.*?\]\s*:.*?:\s*(\d+)/m) {
            push @tableTimes, $1;
            #$time = $1;
            #print TABLEDAT "$csLabel, $attack, $halfBits, $time\n";
        }
        $count = 0;
        $totalTime = 0;
        while ($output =~ /^TIME\[crack,file=.*?([^\/.]*)\.msg\]\s*:.*?:\s*(\d+)\s*$/gm) {
            $msgTag = $1;
            $time = $2;
            $totalTime += $time;
            $count++;
            print CRACKCSV "$csLabel, $attack, $messageBits, $msgTag, $time\n";
        }
        if ($count > 0) {
            $time = $totalTime / $count;
        } else {
            $time = '-';
        }
        #print CRACKAVGDAT "$csLabel, $attack, $messageBits, $count, $time\n";
        push @crackTimes, $time;

        while ($output =~ /^(U?)RESULTS\[file=.*?([^\/.]*)\.msg\]\s*:\s*(\d+)\s*$/gm) {
            my $type = $1 ? 'unique' : 'all';
            $msgTag = $2;
            push @{$resultCounts{$type}{$msgTag}}, $3;
        }
    }
    for my $type (qw/all unique/) {
        my $fh = ($type eq 'all') ? *RESULTCOUNTDAT : *UNIQUERESULTCOUNTDAT;
        while (my ($k, $v) = each %{$resultCounts{$type}}) {
            print $fh join (" ", $messageBits, $k, @{$v}), "\n";
        }
    }
    print TABLEDAT join (" ", $messageBits, @tableTimes), "\n";
    print CRACKAVGDAT join (" ", $messageBits, @crackTimes), "\n";
}
close (TABLEDAT);
close (CRACKCSV);
close (CRACKAVGDAT);
close (RESULTCOUNTDAT);
close (UNIQUERESULTCOUNTDAT);

__END__

=head1 NAME

attack.pl - run mimattack on many messages at once and save the results.

=head1 SYNOPSIS

    attack.pl -help|-man
OR  attack.pl -attack ATTACK1 ATTACK2... -cryptosystem FILE \
              -messagebits STARTBITS [ENDBITS]

 Options:
   -help        brief help message
   -man         full documentation
   -attack, -n ATTACK1 ATTACK2 ...
                specify attack names to run, or 'all'
   -cryptosystem, -c CSFILE | CSNAME
                file containing ElGamal cryptosystem or name of cryptosystem
   -messagebits, -b STARTBITS [ENDBITS]
                attack messages in given range, inclusive

=head1 OPTIONS

=over 8

=item B<-help>

Prints a brief help message and exits.

=item B<-man>

Prints the manual page and exits.

=item B<-attack>, B<-n> ATTACK1 ATTACK2 ...

A space separated list of attack names to pass to mimattack. To
run all attacks, use 'all'.

=item B<-cryptosystem>, B<-c> CSFILE | CSNAME

Path to a file containing an ElGamal cryptosystem created using
elgamalmgr. If the file does not exist, then $CSDIR is prepended and
.elg is appended.

=item B<-messagebits>, B<-b> STARTBITS [ENDBITS]

If only STARTBITS is given, run the attack on messages of the given
bit size. If two numbers are given (separated by a space), run on all
messages from STARTBITS to ENDBITS inclusive, in two bit increments.

The messages should be created with createMessages.pl, and the variable
$CSDIR should be set to the base path for cryptosystems and messages.
See 'createMessages.pl -man' for more details of the directory structure.

=back

=head1 DESCRIPTION

B<attack.pl> is a driver script to run mimattack with different parameters
and save the results to log and data files. Cryptosystems and messages
should be created first using elgamalmgr.

=cut
