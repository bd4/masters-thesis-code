#!/usr/bin/perl -w

# Script to create many messages using elgamalmgr.

use strict;
use Pod::Usage;
use Getopt::Long;
use File::Spec::Functions qw(rel2abs);
use File::Basename;

my $CSDIR = dirname(rel2abs($0)) . '/cryptosystems/';

$main::VERSION = '0.1';

my $csFilePath = '';
my $csDirPath = '';
my $startBits = 0;
my $endBits = 0;
my @messageBitsList = ();

my $man = 0;
my $help = 0;

GetOptions ("cryptosystem|c=s" => \$csFilePath,
            "messagebits|b=i{1,2}" => \@messageBitsList,
            'help|?' => \$help, man => \$man) or pod2usage(2);

pod2usage(-verbose => 0) if $help;
pod2usage(-verbose => 2) if $man;

pod2usage(-verbose => 0)
    unless (@messageBitsList && $csFilePath);

print "csFilePath = $csFilePath\n";

$startBits = $messageBitsList[0];
if (scalar @messageBitsList == 1) {
    $endBits = $startBits;
} else {
    $endBits = $messageBitsList[1];
}

if (! -f $csFilePath) {
    $csFilePath = $CSDIR . $csFilePath . '.elg';
    if (! -f $csFilePath) {
        #die "No crytposystem found at '$csFilePath'.\n";
        pod2usage("No crytposystem found at '$csFilePath'.");
    }
}
($csDirPath = $csFilePath) =~ s/\.elg$//;
unless (-d $csDirPath) {
    mkdir $csDirPath;
}

my ($messageBits, $halfBits, $tag, $bitMessageDir);
for $halfBits ($startBits/2 .. $endBits /2) {
    $messageBits = 2 * $halfBits;
    $bitMessageDir = "$csDirPath/${halfBits}_${halfBits}/";
    unless (-d $bitMessageDir) {
        mkdir $bitMessageDir;
    }
    for $tag ('a' .. 'j') {
        system ("elgamalmgr cm '$bitMessageDir$tag.msg' -m$messageBits -c '$csFilePath'\n");
    }
}

__END__


=head1 NAME

createMessages.pl - create messages for attack.pl

=head1 SYNOPSIS

    createMessages.pl -help|-man
OR  createMessages.pl -cryptosystem CSFILE|CSNAME \
                      -messagebits STARTBITS [ENDBITS]

 Options:
   -help        brief help message
   -man         full documentation
   -cryptosystem, -c CSFILE | CSNAME
                file containing ElGamal cryptosystem or name of cryptosystem
   -messagebits, -b STARTBITS [ENDBITS]
                createMessages messages of given sizes (or inclusive range)

=head1 OPTIONS

=over 8

=item B<-help>

Prints a brief help message and exits.

=item B<-man>

Prints the manual page and exits.

=item B<-cryptosystem>, B<-c> CSFILE | CSNAME

Path to a file containing an ElGamal cryptosystem created using
elgamalmgr. If the file does not exist, then $CSDIR is prepended and
.elg is appended.

=item B<-messagebits>, B<-b> STARTBITS [ENDBITS]

Create messages of sizes starting with STARTBITS, and if given,
increment by two until ENDBITS is reached (inclusive).

=back

=head1 DESCRIPTION

B<createMessages.pl> is a convenience script for creating many splittable
messages of different sizes for a given cryptosystem. elgamalmgr is used to
create the actual messages.

The $CSDIR variable should be set to the desired output directory. If
the cryptosystem specified with -cryptosystem is 'test1024bit.elg', then
createMessages.pl will put messages in "$CSDIR/test1024bit/$halfBits_$halfBits"
with names a.msg, b.msg, c.msg, etc. $halfBits is half the bit size
of the message; the purpose of this naming scheme is to make clear that the
message splits into two part of $halfBits size each. Other splits are currently
not implemented by elgamalmgr, but may be added in the future.

=cut
