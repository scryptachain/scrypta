#/bin/bash
sudo add-apt-repository ppa:pivx/berkeley-db4
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys ED72260A3CDEA874
sudo apt-get update
sudo apt-get install libdb4.8-dev libdb4.8++-dev