#!/usr/bin/perl -w
# udpqotd - UDP message server
use strict;
use IO::Socket;
my($sock, $oldmsg, $newmsg, $hisaddr, $hishost, $MAXLEN, $PORTNO);
$MAXLEN = 1024;
$PORTNO = 6580;

$sock = IO::Socket::INET->new(LocalPort => $PORTNO, Proto => 'udp') or die "socket: $@";
print "Awaiting UDP messages on port $PORTNO\n";
$oldmsg = "This is the starting message.";
while ($sock->recv($newmsg, $MAXLEN)) {
    my($port, $ipaddr) = sockaddr_in($sock->peername);
    $hishost = gethostbyaddr($ipaddr, AF_INET);
    print "Client $hishost said ``$newmsg''\n";
#    $sock->send($oldmsg);
#    $oldmsg = "[$hishost] $newmsg";
} die "recv: $!";
