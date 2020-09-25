# stasis
Stasis is a generic service connectivity verifier. This means that it is a
server that accepts clients according to customizable options and then polls
them and logs their connectivity. The stasis service was created for use in the
inaugural Calvert Hall Capture the Flag competition.

# use
Build the project with `make all`, and then launch the daemon with `./stasisd`.
Clients can be connected with the invocation `./stasis [ip_address]
[team_number]`. More modularity will be introduced in a future update.

# licensing
This project is licensed with the GNU GPLv3.
