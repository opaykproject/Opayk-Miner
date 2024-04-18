This document explains how to setup the Opayk miner on a x64 Ubuntu machine.

You must first have a running Opayk full node. You can setup one by following [this guide](https://github.com/opaykproject/opayk/blob/main/doc/setup-node.md).

First, open a terminal and ensure your machine is up to date with the following command:
```
$ sudo apt-get update && sudo apt-get upgrade
```

Install the required dependencies with:
```
$ sudo apt-get install git build-essential cmake libcurl4-openssl-dev libjansson-dev libboost-system-dev libboost-filesystem-dev libboost-thread-dev ocl-icd-opencl-dev librandomx-dev
```

If you want to mine the GPU algorithm using an Intel card, install the Intel OpenCL runtime with:

```
$ sudo apt-get install intel-opencl-icd
```

For NVIDIA cards, install the NVIDIA driver with:

```
$ sudo apt-get install nvidia-headless-550
```

For AMD cards, install the `amdgpu-install` script with:

```
$ wget https://repo.radeon.com/amdgpu-install/6.1/ubuntu/jammy/amdgpu-install_6.1.60100-1_all.deb
$ sudo apt install ./amdgpu-install_6.1.60100-1_all.deb
```

Then, for Vega 10 and newer devices, run:

```
$ sudo amdgpu-install --usecase=opencl --opencl=rocr
```

For older AMD hardware:

```
$ sudo amdgpu-install --usecase=opencl --opencl=legacy --accept-eula
```

Otherwise, you can also use the Mesa drivers:

```
$ sudo apt-get install mesa-opencl-icd
```

You can check if you have a working OpenCL runtime installed with the `clinfo` utility:

```
$ sudo apt-get install clinfo
$ clinfo
```

Next, clone the miner repository with:

```
$ git clone --depth=1 git@github.com:opaykproject/Opayk-Miner.git
$ cd Opayk-Miner
$ git submodule update --init --depth=1
```

Then compile the miner binary with:

```
$ mkdir build && cd build
$ cmake .. -DHUNTER_ENABLED=OFF -DETHASHCUDA=off
$ cmake --build .
```

This will create a `minerd` file in the current directory. You can install it with:

```
$ sudo install minerd /usr/local/bin/opayk-miner
```

Now you can run `opayk-miner --help` to see the list of available options. For example, to start mining against your local Opayk node, you can run:

```
$ opayk-miner --coinbase-addr=<YOUR OPAYK ADDRESS> -o http://127.0.0.1:14911
```
