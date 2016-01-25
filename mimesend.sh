#!/bin/sh
#
# example script to send a mime message with multiple attachments
# and custom SMTP headers
#
# NOTE: as of mpk version 1.3 you can specify to-addr, bcc-addr 
# and subject directly to mpk which in turn will send them to
# the local MTA - so this script IS NO LONGER NEEDED
#
# this script should be used only if you need to feed custom
# headers or mta flasgs to the outbound email
#

MPK=/usr/local/bin/mpk
SND=/usr/sbin/sendmail
no() { echo "error: no $1"; exit 1; }

# check stuff
[ -x $MPK ] || no mpk
[ -x $SND ] || no sendmail
type date >/dev/null 2>&1 || no date
type whoami >/dev/null 2>&1 || no whoami
type hostname >/dev/null 2>&1 || no hostname

# ask for To and Subject
echo -n "To: "
read to
echo -n "Subject: "
read subject
echo "Type the text part of your message, ^d to finnish." 
echo ""

{
# generate the smtp header
cat <<-EOH
Date: `date`
From: `whoami`@`hostname`
To: $to
Subject: $subject
EOH

# finally read the input and supplied files
# if you want to skip the text input just remove '-m'
# you can also get the text from a file using '<' operator
$MPK -m $*

# redirect to sendmail
} | $SND $to
