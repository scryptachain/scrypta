# SCRYPTA DOCKER IMAGE

## BUILD IMAGE

Commands:

```
git clone https://github.com/scryptachain/scrypta
cd scrypta
docker build -t scrypta:latest .
```

(can be a long running task)

## RUN

First run container:

```
docker run --name=scrypta -dit -p 42222:42222 -p 42223:42223 scrypta:latest
```

Then run the daemon inside the container:

```
docker exec -it scrypta /opt/scrypta/lyrad &
```

Now you're able to launch every command like: 

```
docker exec -it scrypta /opt/scrypta/lyra-cli getinfo
```

## STOP AND CLEAN

Commands:

```
docker stop scrypta
docker rm scrypta
```
