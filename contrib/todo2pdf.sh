#!/bin/sh 
C=/usr/share/java/bsf.jar:/usr/share/java/fop.jar:/usr/share/java/xalan2.jar:$HOME/deb/lib-fop-java-0.17.0.orig/lib/w3c.jar 
java -cp "$C" org.apache.fop.apps.XalanCommandLine $1 dev2fo.xsl $2 
