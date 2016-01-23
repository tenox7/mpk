#!/bin/sh
#
# example script to send a mime message with multiple attachments
# one would like to add to/subject options on the command line...
#

MPK=./mpk-osx
SND=/usr/sbin/sendmail
no() { echo "ups... no $1 - exiting"; exit 1; }

# check stuff
[ -x $MPK ] || no mpk
[ -x $SND ] || no sendmail
type date >/dev/null 2>&1 || no date
type whoami >/dev/null 2>&1 || no whoami
type hostname >/dev/null 2>&1 || no hostname

# ask some crucial questions
echo -n "To: "
read to
echo -n "Subject: "
read subject
echo "Type the text part of your message, ^d to finnish." 
echo ""

{
# generate smtp header
cat <<-EOH
Date: $(date)
From: $(whoami)@$(hostname)
To: $to
Subject: $subject
EOH

# finaly read input and supplied files
# if you want to skip the text input just remove cat and -m
# you can also get text from a file using < operator
cat | $MPK -m $*

# fire and forget
} | $SND $to
