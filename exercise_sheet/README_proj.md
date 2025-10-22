# The island of two faces

The `render-data` of the client and server can be downloaded here:

- [https://cloud.privacy1st.de/s/Fq3RZjdT4nX9D8d]()

The sky sphere takes some time to load, so please be patient until you see the sky and stars.

Weather changes over time:

- sunny, no clouds
- clouds
- dark clouds, rain and lightning

And don't forget, to turn on your speakers to enjoy the soundtrack.

## Server and Client

A server for 3 players at CIP computer `cip3c0` can be started with:

```shell
./applications/server/a5-server -a cip3c0.cip.cs.fau.de -p 2214 -n 3
```

And each client with:

```shell
./applications/client/a5-client -a cip3c0.cip.cs.fau.de -p 2214 -u SomeUserName
```
