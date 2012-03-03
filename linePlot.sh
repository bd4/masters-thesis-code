#!/bin/bash

ploticus -prefab lines data=$1 x=1 y=2 y2=3 y3=4 y4=5 header=yes
ploticus -prefab lines data=tableTime.dat x=1 y=2 y2=3 y3=4 y4=5 header=yes ygrid=yes -eps title="Table Build Times" xlbl="message bits" ylbl="seconds" "ylbldet= adjust=-0.1,0"
