Nu 0.1.0 BETA

Copyright (c) 2014 Nu Developers
Distributed under the MIT/X11 software license, see the accompanying
file license.txt or http://www.opensource.org/licenses/mit-license.php.
This product includes software developed by the OpenSSL Project for use in
the OpenSSL Toolkit (http://www.openssl.org/).  This product includes
cryptographic software written by Eric Young (eay@cryptsoft.com).


Intro
-----
// todo: Add intro text


Setup
-----
After completing windows setup then run Nu.
Alternatively you can run windows command line (cmd) in nu program dir.
  cd daemon
  nud
You would need to create a configuration file nu.conf in the default
wallet directory. Grant access to nud/nu in anti-virus and firewall
applications if necessary.

The software automatically finds other nodes to connect to.  You can
enable Universal Plug and Play (UPnP) with your router/firewall
or forward port 9901 (TCP) to your computer so you can receive
incoming connections.  PPCoin works without incoming connections,
but allowing incoming connections helps the PPCoin network.


Upgrade
-------
All your existing coins/transactions should be intact with the upgrade.
To upgrade, first backup wallet:
  nud backupwallet <destination_backup_file>
Then shutdown the daemon with:
  nud stop
Remove files inside wallet directory other than wallet.dat and nu.conf
Start up the new client. It would start re-download of block chain.


See the documentation/wiki at the Nu website:
  http://www.nubits.com
for help and more information.
