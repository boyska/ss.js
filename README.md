SS.JS - Shamir's Secrets Joined through Sockets
=======

This is a real-life implementation that uses ssss[1] to "collaboratively"
retrieve a secret on a remote box.

Use case
----------

Alice, Bob and Charlie have an encrypted storage, and they like ssss: they
split the keyfile in 3 pieces and set the quorum to 2, so that two of them
together can retrieve it, but one alone cannot. They'll later use the keyfile
to unlock the encrypted storage.

They put the storage on their server "Zeus". When they want to open it, they
run

    ssh zeus socat - UNIX-CONNECT:/var/tmp/combinator < my_secret_part

and magically the encrypted volume get mounted.

How it works
-------------

It is basically an "accumulator" that receives messages from a socket and,
when it's over, launch ssss-combine with the received messages as input.
The "secret" that ssss-combine will reveal, will then be written to stdout, so
that you can use its output for whatever you want (tipically decrypting
something)

Setup on the server
-------------------

Run cryptsetup luksOpen mydevice --keyfile <(ssserver -n 2)

[1]: http://point-at-infinity.org/ssss/
