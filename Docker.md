# SCRYPTA DOCKER IMAGE

## BUILD IMAGE

Commands:
	docker build -t scrypta:latest .

(can be a long running task)

## RUN

Create directory (example):
	mkdir /opt/lyra-data

Create config file:
	/opt/lyra-data/lyra.conf

Config file must contains **rpcuser** and **rpcpassword**:
        rpcuser=lyrarpc
	rpcpassword=<choice a password>	

Command:
	docker run --name=scrypta -dit -p 42222:42222 -p 42223:42223 -v /opt/lyra-data:/.root/.lyra -v /opt/lyra-data/lyra.conf:/root/.lyra/lyra.conf scrypta:latest
## TEST

Command:
	docker exec -it scrypta /opt/scrypta/lyra-cli getinfo

## STOP AND CLEAN

Commands:
	docker stop scrypta
	docker rm scrypta
